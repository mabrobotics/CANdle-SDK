#include "mab_types.hpp"
#include "md_types.hpp"
#include "candle_frame_adapter.hpp"
#include "candle_types.hpp"
#include "candle.hpp"

#include <atomic>
#include <memory>
#include <chrono>
#include <deque>
#include <unordered_map>
#include <future>
namespace mab
{
    class MDAggregator
    {
      public:
        using CFFrameFuture_t = std::future<std::pair<std::vector<u8>, candleTypes::Error_t>>;
        class MDFrame : candleTypes::CANFrameData_t
        {
          public:
            const MdFrameId_E m_frameType;

            MDFrame(canId_t canId, MdFrameId_E frameType)
                : candleTypes::CANFrameData_t(canId), m_frameType(frameType)
            {
                m_data = getHeader();
            }

            inline std::vector<u8> getHeader() const noexcept
            {
                return {(u8)(u16)m_frameType, (u8)((u16)m_frameType << 8), 0x0};
            }

            template <typename T>
            inline size_t insertRegister(MDRegisterEntry_S<T> reg)
            {
                auto serializedReg = reg.getSerializedRegister();
                if (m_frame.size() + serializedReg.size() > 64)
                    return 0;
                size_t regAt = m_frame.size();
                m_data.insert(m_data.end(), serializedReg->begin(), serializedReg->end());
                return regAt;
            }
        };

        const canId_t m_canId;

        MDAggregator(const canId_t canId,
                     std::shared_ptr<std::function<CFFrameFuture_t(candleTypes::CANFrameData_t)>>
                         sendFrameFunction)
            : m_canId(canId), m_sendFrameFunction(sendFrameFunction)
        {
        }

        template <typename T>
        inline std::future<candleTypes::Error_t> writeRegisterAsync(MDRegisterEntry_S<T> reg)
        {
            size_t serializedRegSize = reg.getSerializedRegister();

            std::unique_lock lock(m_mux);  // Lock to prevent race conditions with sending thread

            auto mapIter = m_frameTypeMap.find(MdFrameId_E::WRITE_REGISTER);
            if (writeQueue == m_frameTypeMap.end())
            {
                mapIter = m_frameTypeMap[MdFrameId_E::WRITE_REGISTER] = std::deque<MDFrame>();
            }
            auto MDFrameQueue = mapIter->second;
            // Small discrepancy over efficiency boost - we only look for a place in the last frame
            // only in order not to look throughout all of the frames (which may have more space
            // left)
            auto frame = MDFrameQueue.back();
            if (CANdleFrame::DATA_MAX_LENGTH - frame.m_data().size() < serializedRegSize)
            {
                frame = MDFrameQueue.emplace_back(m_canId, MdFrameId_E::WRITE_REGISTER);
            }

            size_t idx = frame.insertRegister(reg);
        }

      private:
        const std::shared_ptr<std::function<CFFrameFuture_t(candleTypes::CANFrameData_t)>>
                                                             m_sendFrameFunction;
        std::unordered_map<MdFrameId_E, std::deque<MDFrame>> m_frameTypeMap;

        mutable std::mutex m_mux;
    };
}  // namespace mab