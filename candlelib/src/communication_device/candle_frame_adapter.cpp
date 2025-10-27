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
            m_log.error("Frame accumulation timed out! Reader thread might be malfunctioning!");
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
            return std::make_pair<std::vector<u8>, Error_t>({}, Error_t::INVALID_FRAME);
        }
        cf.serialize(buf);
        m_packedFrame.insert(m_packedFrame.end(), buf, buf + cf.DTO_SIZE);

        // Notify host object that the reader must run
        if (auto func = m_requestTransfer.lock())
        {
            (*func)();
        }
        else
        {
            m_log.warn("No thread to notify to start transfer!");
        }

        // Wait for data to be available
        auto timeout = m_cv.wait_for(lock, std::chrono::milliseconds(READER_TIMEOUT));
        if (timeout == std::cv_status::timeout)
        {
            m_log.error("Frame accumulation timed out! Writer thread might be malfunctioning!");
            return std::make_pair<std::vector<u8>, Error_t>({}, Error_t::READER_TIMEOUT);
        }
        return std::make_pair<std::vector<u8>, Error_t>(std::vector(m_responseBuffer[seqIdx]),
                                                        Error_t::OK);
    }

    std::vector<u8> CANdleFrameAdapter::getPackedFrame() noexcept
    {
        std::unique_lock lock(m_mutex);

        std::vector<u8> packedFrame = m_packedFrame;
        u8              count       = m_count;
        m_count                     = 0;
        m_packedFrame.clear();
        m_packedFrame.push_back(CANdleFrame::DTO_PARSE_ID);
        m_packedFrame.push_back(0x1 /*ACK*/);
        m_packedFrame.push_back(0x0 /*Placeholder for count*/);
        m_sem.release(count);

        auto countIter      = packedFrame.begin() + 2 /*PARSE_ID + ACK*/;
        *countIter          = count;
        u32 calculatedCRC32 = Crc::calcCrc((const char*)packedFrame.data(), packedFrame.size());
        packedFrame.push_back(calculatedCRC32);
        packedFrame.push_back(calculatedCRC32 >> 8);
        packedFrame.push_back(calculatedCRC32 >> 16);
        packedFrame.push_back(calculatedCRC32 >> 24);
        return packedFrame;
    }

    CANdleFrameAdapter::Error_t CANdleFrameAdapter::parsePackedFrame(
        const std::vector<u8>& packedFrames) noexcept
    {
        std::unique_lock lock(m_mutex);

        for (auto& buf : m_responseBuffer)
        {
            buf.clear();
        }
        auto pfIterator = packedFrames.begin();
        if (*pfIterator != CANdleFrame::DTO_PARSE_ID)
        {
            m_log.error("Wrong parse ID of CANdle Frames!");
            m_cv.notify_all();
            return Error_t::INVALID_FRAME;
        }
        pfIterator++;
        if (!*pfIterator /*ACK*/)
        {
            m_log.error("Error inside the CANdle Device!");
            m_cv.notify_all();
            return Error_t::INVALID_FRAME;
        }
        pfIterator++;
        u8 count = *pfIterator;
        if (count > FRAME_BUFFER_SIZE ||
            packedFrames.size() >
                PACKED_SIZE - ((FRAME_BUFFER_SIZE - count) * CANdleFrame::DTO_SIZE))
        {
            m_log.error("Invalid message size!");
            m_cv.notify_all();
            return Error_t::INVALID_FRAME;
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
            m_cv.notify_all();
            return Error_t::INVALID_FRAME;
        }
        pfIterator -= count * CANdleFrame::DTO_SIZE;  // Rollback to the data head

        for (; count != 0; count--)
        {
            CANdleFrame cf;
            cf.deserialize((void*)&(*pfIterator));
            pfIterator += CANdleFrame::DTO_SIZE;
            size_t idx = cf.sequenceNo() - 1;
            if (!cf.isValid() || idx > FRAME_BUFFER_SIZE - 1)
            {
                m_log.error("CANdle frame %u is not valid! Index = %u, Can ID = %u, Length = %u",
                            count,
                            idx,
                            cf.canId(),
                            cf.length());
                m_cv.notify_all();
                return Error_t::INVALID_FRAME;
            }
            m_responseBuffer[idx].insert(
                m_responseBuffer[idx].begin(), cf.data(), cf.data() + cf.length());
        }
        m_cv.notify_all();
        return Error_t::OK;
    }

    u8 CANdleFrameAdapter::getCount() const noexcept
    {
        return m_count;
    }

}  // namespace mab