#include "candle_frame_adapter.hpp"

namespace mab
{

    std::vector<CANdleFrame> CANdleFrameAdapter::yeldAccumulatedFrames(const size_t maxCount)
    {
        std::unique_lock lock(m_mutex);

        std::vector<CANdleFrame> ret;
        if (maxCount > m_candleFrameAccumulator.size())
        {
            for (size_t i = 0; i < maxCount; i++)
            {
                CANdleFrame cf =
                    m_candleFrameAccumulator.front().build(i + 1).value_or(CANdleFrame());

                // discard bad frames
                if (cf.isValid())
                    ret.push_back(std::move(cf));

                m_candleFrameAccumulator.pop_front();
            }
        }
        else
        {
            // const size_t frameAccumulatorSize = m_candleFrameAccumulator.size();
            // for (size_t i = 0; i < frameAccumulatorSize; i++)
            // {
            //     ret.push_back(std::move(cf));
            //     m_candleFrameAccumulator.pop_front();
            // }
        }
        return ret;
    }

    CANdleFrameAdapter::Error_t CANdleFrameAdapter::accumulateFrame(const canId_t          canId,
                                                                    const std::vector<u8>& data,
                                                                    const u16 timeout100us)
    {
        if (canId == 0)
            return Error_t::INVALID_FRAME;

        std::unique_lock lock(m_mutex);
        // Look for an already created frame to pack the data
        for (auto& cfb : m_candleFrameAccumulator)
        {
            if (cfb.canId == canId && data.size() < CANdleFrame::DATA_MAX_LENGTH - cfb.data->size())
            {
                cfb.data->insert(cfb.data->end(), data.begin(), data.end());
                cfb.timeout100us +=
                    timeout100us;  // accumulate timeout as each command will make handling longer
                return Error_t::OK;
            }
        }

        // If not found create a new frame
        CANdleFrameBuilder cfb;
        cfb.canId        = canId;
        cfb.data         = std::make_unique<std::vector<u8>>(data);
        cfb.timeout100us = timeout100us;
        m_candleFrameAccumulator.push_back(std::move(cfb));
        return Error_t::OK;
    }
}  // namespace mab