#pragma once

#include "logger.hpp"
#include "candle_frame_dto.hpp"

#include <future>
#include <memory>

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

      private:
        Logger m_log = Logger(Logger::ProgramLayer_E::LAYER_2, "CANDLE_FR_ADAPTER");
        std::vector<CANdleFrame> m_candleFrameAccumulator;
    };
}  // namespace mab
