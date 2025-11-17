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
#include <thread>
#include <vector>
#include <unordered_map>

namespace mab
{
    /// @brief Adapter class to convert CAN frames to Candle frame DTOs and accumulate them to
    /// fully utilize USB bulk transfers
    class CANdleFrameAdapter
    {
      public:
        static constexpr size_t FRAME_BUFFER_SIZE = 7;
        static constexpr size_t USB_MAX_BULK_TRANSFER =
            512;  // Full-speed USB max bulk transfer size for libusb
        static constexpr std::chrono::duration READER_TIMEOUT = std::chrono::milliseconds(20);

        static constexpr size_t DEPRECATION_FRAME_COUNT =
            500;  // Frames older than this will be deleted

        static constexpr u16 PACKED_SIZE =
            sizeof(CANdleFrame::DTO_PARSE_ID) + sizeof(u8 /*ACK*/) + sizeof(u8 /*COUNT*/) +
            CANdleFrame::DTO_SIZE * FRAME_BUFFER_SIZE + sizeof(u32 /*CRC32*/);

        static_assert(PACKED_SIZE < USB_MAX_BULK_TRANSFER, "USB bulk transfer too long!");

        enum class Error_t
        {
            UNKNOWN,
            OK,
            READER_TIMEOUT,
            INVALID_BUS_FRAME,
            FRAME_LOST,
            INVALID_CANDLE_FRAME
        };

        /// @brief CFAdapter constructor
        /// @param requestTransfer This function will be called every time the CAN frame is
        /// accumulated
        explicit CANdleFrameAdapter(std::shared_ptr<std::function<void(void)>> requestTransfer)
            : m_requestTransfer(requestTransfer)
        {
        }

        /// @brief Accumulate CAN frame into Candle frame(s)
        /// @param canId Target CAN node ID
        /// @param data Data to be transferred via CAN bus
        /// @param timeout100us Time after which candle will stop waiting for node response in
        /// units of 100 microseconds
        /// @return Future containing response can frame (undefined on error being not OK) and error
        /// code
        std::pair<std::vector<u8>, Error_t> accumulateFrame(const canId_t          canId,
                                                            const std::vector<u8>& data,
                                                            const u16              timeout100us);

        /// @brief Get packed frame ready to be sent via bus, clears internal buffer for fresh frame
        /// accumulation
        /// @return packed candle frames (Header,ACK placeholder, length, candle frame(s), CRC32)
        std::pair<std::vector<u8>, std::atomic<u64>> getPackedFrame();

        /// @brief  Parse received packed candle frames for the waiting futures
        /// @param packedFrames Received packed candle frames
        /// @return OK on success, error code otherwise
        Error_t parsePackedFrame(const std::vector<u8>& packedFrames, std::atomic<u64> idx);

        /// @brief Get number of accumulated frames waiting for transfer atomically
        /// @return number of accumulated frames
        inline u8 getCount() const noexcept
        {
            return m_count;
        }

      private:
        Logger m_log = Logger(Logger::ProgramLayer_E::LAYER_2, "CANDLE_FR_ADAPTER");

        std::atomic<u8>  m_count      = 0;
        std::atomic<u64> m_frameIndex = 0;

        std::unordered_map<u64, std::vector<u8>> m_packedFrames = {
            std::make_pair<u64, std::vector<u8>>(0, {CANdleFrame::DTO_PARSE_ID, 0x1, 0x0})};
        std::unordered_map<u64, std::array<std::vector<u8>, FRAME_BUFFER_SIZE>> m_responseBuffer;
        std::unordered_map<u64, std::condition_variable>                        m_notifiers;

        std::mutex m_mutex;
        // std::condition_variable   m_cv;
        std::counting_semaphore<> m_sem = std::counting_semaphore<>((ptrdiff_t)FRAME_BUFFER_SIZE);

        const std::weak_ptr<std::function<void(void)>> m_requestTransfer;
    };
}  // namespace mab
