#pragma once

#include "logger.hpp"
#include "candle_frame_dto.hpp"
#include "mab_types.hpp"

#include <atomic>
#include <array>
#include <future>
#include <memory>
#include <optional>
#include <mutex>
#include <semaphore>
#include <condition_variable>
#include <chrono>

namespace mab
{
    class CANdleFrameAdapter
    {
      public:
        static constexpr size_t                FRAME_BUFFER_SIZE     = 7;
        static constexpr size_t                USB_MAX_BULK_TRANSFER = 512;
        static constexpr std::chrono::duration READER_TIMEOUT = std::chrono::milliseconds(10);

        static constexpr u16 PACKED_SIZE =
            sizeof(CANdleFrame::DTO_PARSE_ID) + sizeof(u8 /*ACK*/) + sizeof(u8 /*COUNT*/) +
            CANdleFrame::DTO_SIZE * FRAME_BUFFER_SIZE + sizeof(u32 /*CRC32*/);

        static_assert(PACKED_SIZE < USB_MAX_BULK_TRANSFER, "USB bulk transfer too long!");

        enum class Error_t
        {
            UNKNOWN,
            OK,
            READER_TIMEOUT,
            INVALID_FRAME
        };

        std::pair<std::vector<u8>, Error_t> accumulateFrame(const canId_t          canId,
                                                            const std::vector<u8>& data,
                                                            const u16              timeout100us);

        std::vector<u8> getPackedFrame() noexcept;
        Error_t         parsePackedFrame(const std::vector<u8>& packedFrames) noexcept;

      private:
        Logger m_log = Logger(Logger::ProgramLayer_E::LAYER_2, "CANDLE_FR_ADAPTER");
        std::array<std::vector<u8>, FRAME_BUFFER_SIZE> m_responseBuffer;

        std::atomic<u8>           m_count = 0;
        std::vector<u8>           m_packedFrame;
        std::mutex                m_mutex;
        std::condition_variable   m_cv;
        std::counting_semaphore<> m_sem = std::counting_semaphore<>((ptrdiff_t)FRAME_BUFFER_SIZE);
    };
}  // namespace mab
