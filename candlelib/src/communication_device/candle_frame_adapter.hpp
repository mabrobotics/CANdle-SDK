#pragma once

#include "logger.hpp"
#include "candle_frame_dto.hpp"
#include "tsdeque.hpp"

#include <atomic>
#include <future>
#include <memory>
#include <deque>

namespace mab
{
    class CandleFrameAdapter
    {
        enum class Error_t
        {
            UNKNOWN,
            OK
        };

      public:
        CandleFrameAdapter()
        {
        }

        std::vector<CANdleFrame> yeldAccumulatedFrames(const size_t maxCount)
        {
            std::vector<CANdleFrame> ret;
            if (maxCount > m_candleFrameAccumulator.size())
            {
                for (size_t i = 0; i < maxCount; i++)
                {
                    ret.emplace_back(m_candleFrameAccumulator.pop_front());
                }
            }
            else
            {
                const size_t frameAccumulatorSize = m_candleFrameAccumulator.size();
                for (size_t i = 0; i < frameAccumulatorSize; i++)
                {
                    ret.emplace_back(m_candleFrameAccumulator.pop_front());
                }
            }
            return ret;
        }

      private:
        Logger m_log = Logger(Logger::ProgramLayer_E::LAYER_2, "CANDLE_FR_ADAPTER");
        TQueueConcurrent<CANdleFrame> m_candleFrameAccumulator;
    };
}  // namespace mab
