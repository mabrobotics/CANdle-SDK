#pragma once

#include "logger.hpp"
#include "candle_frame_dto.hpp"

#include <atomic>
#include <future>
#include <memory>
#include <deque>
#include <optional>

namespace mab
{
    class CANdleFrameAdapter
    {
      public:
        static constexpr size_t MAX_FRAMES_ACCUMULATED = 256;
        struct CANdleFrameBuilder
        {
            canId_t                          canId        = 0;
            std::unique_ptr<std::vector<u8>> data         = nullptr;
            u16                              timeout100us = 0;

            std::optional<CANdleFrame> build(u8 seqenceNo)
            {
                if (canId != 0 && data != nullptr)
                {
                    CANdleFrame cf;
                    cf.init(canId, seqenceNo, timeout100us);
                    cf.addData(data->data(), data->size());
                    return cf;
                }
                else
                    return {};
            }
        };

        enum class Error_t
        {
            UNKNOWN,
            OK,
            INVALID_FRAME
        };

        std::vector<CANdleFrame> yeldAccumulatedFrames(const size_t maxCount);

      Error_t accumulateFrame(const canId_t          canId,
                              const std::vector<u8>& data,
                              const u16              timeout100us)

          private : Logger m_log = Logger(Logger::ProgramLayer_E::LAYER_2, "CANDLE_FR_ADAPTER");
        std::deque<CANdleFrameBuilder> m_candleFrameAccumulator;

        std::condition_variable m_newData;
        std::mutex              m_mutex;
    };
}  // namespace mab
