#include "candle_frame_adapter.hpp"
#include "algorithm"
#include "chrono"
#include "crc.hpp"
namespace mab
{
    std::pair<std::vector<u8>, CANdleFrameAdapter::Error_t> CANdleFrameAdapter::accumulateFrame(
        const canId_t canId, const std::vector<u8>& data, const u16 timeout100us)
    {
        // Wait for the buffer to be available
        auto success = m_sem.try_acquire_for(std::chrono::milliseconds(READER_TIMEOUT));
        if (!success)
        {
            m_log.error("Frame timed out! CANdle is overloaded!");
            return std::make_pair<std::vector<u8>, Error_t>({}, Error_t::READER_TIMEOUT);
        }

        std::unique_lock lock(m_mutex);
        const size_t     seqIdx = m_count++;
        CANdleFrame      cf;
        u8               buf[cf.DTO_SIZE] = {0};

        // Fill and serialize frame object
        cf.init(canId, m_count, timeout100us);
        if (cf.addData(data.data(), data.size()) != CANdleFrame::Error_t::OK)
        {
            m_log.error("Could not generate CANdle Frame!");
            return std::make_pair<std::vector<u8>, Error_t>({}, Error_t::INVALID_BUS_FRAME);
        }
        cf.serialize(buf);
        u64 thisFrameIdx = m_frameIndex;
        m_packedFrames[m_frameIndex].insert(
            m_packedFrames[m_frameIndex].end(), buf, buf + cf.DTO_SIZE);

        // Notify host object that the reader must run
        if (auto func = m_requestTransfer.lock())
        {
            (*func)();
        }
        else
        {
            m_log.warn("No thread to notify to start transfer!");
        }

        // Add notifier if not done for this frame
        if (m_notifiers.find(thisFrameIdx) == m_notifiers.end())
        {
            m_notifiers.try_emplace(thisFrameIdx);
        }

        // Wait for data to be available
        auto timeout =
            m_notifiers[thisFrameIdx].wait_for(lock, std::chrono::milliseconds(READER_TIMEOUT));
        if (timeout == std::cv_status::timeout)
        {
            // TODO: add marked for discard frames
            m_log.error("Frame writer timed out! Frame was lost");
            return std::make_pair<std::vector<u8>, Error_t>({}, Error_t::READER_TIMEOUT);
        }
        auto responseIt = m_responseBuffer.find(thisFrameIdx);
        if (responseIt == m_responseBuffer.end())
        {
            m_log.warn("Frame has timed-out! It will never be received!");
            m_log.debug("Frames in the buffer %u", m_responseBuffer.size());
            return std::make_pair<std::vector<u8>, Error_t>(std::vector<u8>(), Error_t::FRAME_LOST);
        }

        auto buff = m_responseBuffer[thisFrameIdx][seqIdx];
        m_responseBuffer[thisFrameIdx][seqIdx].clear();
        bool shouldDelete = true;
        for (auto responseCANFrames : m_responseBuffer[thisFrameIdx])
        {
            if (!responseCANFrames.empty())
                shouldDelete = false;
        }
        if (shouldDelete)
        {
            m_log.debug("Deleting used buffer!");
            m_responseBuffer.erase(thisFrameIdx);
            m_notifiers.erase(thisFrameIdx);
        }
        return std::make_pair<std::vector<u8>, Error_t>(std::vector(std::move(buff)), Error_t::OK);
    }

    std::pair<std::vector<u8>, std::atomic<u64>> CANdleFrameAdapter::getPackedFrame()
    {
        std::unique_lock lock(m_mutex);

        auto m_packedFrameIt = m_packedFrames.find(m_frameIndex);

        if (m_packedFrameIt == m_packedFrames.end())
        {
            m_log.warn("Expected frame is empty!");
        }

        m_log.debug("Sending frame with idx: %u", m_frameIndex.load());
        std::vector<u8> packedFrame = m_packedFrames[m_frameIndex];
        m_packedFrames.erase(m_frameIndex++);
        u8 count                     = m_count;
        m_count                      = 0;
        m_packedFrames[m_frameIndex] = std::vector<u8>({CANdleFrame::DTO_PARSE_ID, 0x1, 0x0});
        m_sem.release(count);

        lock.unlock();

        auto countIter      = packedFrame.begin() + 2 /*PARSE_ID + ACK*/;
        *countIter          = count;
        u32 calculatedCRC32 = Crc::calcCrc((const char*)packedFrame.data(), packedFrame.size());
        packedFrame.push_back(calculatedCRC32);
        packedFrame.push_back(calculatedCRC32 >> 8);
        packedFrame.push_back(calculatedCRC32 >> 16);
        packedFrame.push_back(calculatedCRC32 >> 24);
        return std::make_pair(packedFrame, m_frameIndex - 1);
    }

    CANdleFrameAdapter::Error_t CANdleFrameAdapter::parsePackedFrame(
        const std::vector<u8>& packedFrames, std::atomic<u64> idx)
    {
        std::unique_lock lock(m_mutex);

        m_responseBuffer[idx] = std::array<std::vector<u8>, FRAME_BUFFER_SIZE>();
        auto pfIterator       = packedFrames.begin();
        if (*pfIterator != CANdleFrame::DTO_PARSE_ID)
        {
            m_log.error("Wrong parse ID of CANdle Frames!");
            m_notifiers[idx].notify_all();
            return Error_t::INVALID_BUS_FRAME;
        }
        pfIterator++;
        if (!*pfIterator /*ACK*/)
        {
            m_log.error("Error inside the CANdle Device!");
            m_notifiers[idx].notify_all();
            return Error_t::INVALID_BUS_FRAME;
        }
        pfIterator++;
        u8 count = *pfIterator;
        if (count > FRAME_BUFFER_SIZE ||
            packedFrames.size() >
                PACKED_SIZE - ((FRAME_BUFFER_SIZE - count) * CANdleFrame::DTO_SIZE))
        {
            m_log.error("Invalid message size!");
            m_notifiers[idx].notify_all();
            return Error_t::INVALID_BUS_FRAME;
        }

        pfIterator += count * CANdleFrame::DTO_SIZE + 1;  // Skip to CRC32
        u32 readCRC32 = static_cast<u32>(*pfIterator) | (static_cast<u32>(*(pfIterator + 1)) << 8) |
                        (static_cast<u32>(*(pfIterator + 2)) << 16) |
                        (static_cast<u32>(*(pfIterator + 3)) << 24);

        u32 calculatedCRC32 =
            Crc::calcCrc((const char*)packedFrames.data(), packedFrames.size() - 4 /*CRC32*/);

        if (readCRC32 != calculatedCRC32)
        {
            m_log.error("Invalid message checksum! 0x%08x != 0x%08x", readCRC32, calculatedCRC32);
            m_notifiers[idx].notify_all();
            return Error_t::INVALID_BUS_FRAME;
        }
        pfIterator -= count * CANdleFrame::DTO_SIZE;  // Rollback to the data head

        Error_t err = Error_t::OK;
        for (; count != 0; count--)
        {
            CANdleFrame cf;
            cf.deserialize((void*)&(*pfIterator));
            pfIterator += CANdleFrame::DTO_SIZE;
            if (!cf.isValid() || cf.sequenceNo() > FRAME_BUFFER_SIZE)
            {
                m_log.error("CANdle frame %u is not valid! Index = %u, Can ID = %u, Length = %u",
                            count,
                            cf.sequenceNo(),
                            cf.canId(),
                            cf.length());
                err = Error_t::INVALID_CANDLE_FRAME;
                continue;
            }
            size_t subidx = cf.sequenceNo() - 1;
            m_log.debug("Parsing bus frame %u with CAN frame %u", idx.load(), subidx + 1);
            m_responseBuffer[idx][subidx].insert(
                m_responseBuffer[idx][subidx].begin(), cf.data(), cf.data() + cf.length());
        }
        m_notifiers[idx].notify_all();
        return err;
    }

    u8 CANdleFrameAdapter::getCount() const noexcept
    {
        return m_count;
    }

}  // namespace mab