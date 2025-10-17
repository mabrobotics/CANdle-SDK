#include "candle_frame_adapter.hpp"
#include "algorithm"
#include "chrono"

namespace mab
{
    std::pair<std::vector<u8>, CANdleFrameAdapter::Error_t> CANdleFrameAdapter::accumulateFrame(
        const canId_t canId, const std::vector<u8>& data, const u16 timeout100us)
    {
        std::unique_lock lock(m_mutex);

        size_t seqId = m_count;

        if (m_count >= FRAME_BUFFER_SIZE)
        {
            auto timeout = m_cv.wait_for(lock, std::chrono::milliseconds(READER_TIMEOUT));

            if (timeout == std::cv_status::timeout)
            {
                m_log.error("Frame accumulation timed out! Reader thread might be malfunctioning!");
                return std::make_pair<std::vector<u8>, Error_t>({}, Error_t::READER_TIMEOUT);
            }
        }
        CANdleFrame cf;
        cf.init(canId, m_count++, timeout100us);
        if (cf.addData(data.data(), data.size()) != CANdleFrame::Error_t::OK)
        {
            m_log.error("Could not generate CANdle Frame!");
            return std::make_pair<std::vector<u8>, Error_t>({}, Error_t::INVALID_FRAME);
        }
        u8 buf[cf.DTO_SIZE] = {0};
        cf.serialize(buf);
        m_packedFrame.insert(m_packedFrame.end(), buf, buf + cf.DTO_SIZE);

        auto timeout = m_cv.wait_for(lock, std::chrono::milliseconds(READER_TIMEOUT));
        if (timeout == std::cv_status::timeout)
        {
            m_log.error("Frame accumulation timed out! Reader thread might be malfunctioning!");
            return std::make_pair<std::vector<u8>, Error_t>({}, Error_t::READER_TIMEOUT);
        }
        return std::make_pair<std::vector<u8>, Error_t>(std::vector(m_responseBuffer[seqId]),
                                                        Error_t::OK);
    }

}  // namespace mab