#include "candle_frame_adapter.hpp"
#include "algorithm"

namespace mab
{

    std::vector<CANdleFrame> CANdleFrameAdapter::yeldAccumulatedFrames(const size_t maxCount)
    {
        std::unique_lock lock(m_frameAccumulatorMux);

        std::vector<CANdleFrame> ret;
        const size_t             count =
            maxCount < m_candleFrameAccumulator.size() ? maxCount : m_candleFrameAccumulator.size();

        for (size_t i = 0; i < count; i++)
        {
            CANdleFrame cf = m_candleFrameAccumulator.front().build(i + 1).value_or(CANdleFrame());

            // discard bad frames
            if (cf.isValid())
            {
                ret.push_back(cf);

                // Register return frame
                m_returnRegister.push_back(std::move(cf));
                m_returnRegister.back().clearData();
                m_returnRegistered.notify_one();
            }
            else
            {
                m_log.error("Invalid frame in the accumulator!");
            }

            m_candleFrameAccumulator.pop_front();
        }
        return ret;
    }

    CANdleFrameAdapter::Error_t CANdleFrameAdapter::accumulateFrame(const canId_t          canId,
                                                                    const std::vector<u8>& data,
                                                                    const u16 timeout100us)
    {
        if (canId == 0)
            return Error_t::INVALID_FRAME;

        std::unique_lock lock(m_frameAccumulatorMux);
        // If not found create a new frame
        CANdleFrameBuilder cfb;
        cfb.canId        = canId;
        cfb.data         = std::make_unique<std::vector<u8>>(data);
        cfb.timeout100us = timeout100us;
        m_candleFrameAccumulator.push_back(std::move(cfb));
        return Error_t::OK;
    }

    CANdleFrameAdapter::Error_t CANdleFrameAdapter::parseResponse(const std::vector<CANdleFrame>&)
    {
        return Error_t::OK;
    }
}  // namespace mab