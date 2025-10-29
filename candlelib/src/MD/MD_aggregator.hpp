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
        using CFFrameFuture_t =
            std::shared_future<std::pair<std::vector<u8>, candleTypes::Error_t>>;
        class MDFrame : public candleTypes::CANFrameData_t
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
                if (m_data.size() + serializedReg.size() > 64)
                    return 0;
                size_t regAt = m_data.size();
                m_data.insert(m_data.end(), serializedReg->begin(), serializedReg->end());
                return regAt;
            }
        };

        template <typename T>
        class RegisterFuture
        {
          public:
            std::future<std::pair<MDRegisterEntry_S<T>, candleTypes::Error_t>> m_regFuture;

            RegisterFuture(MDRegisterEntry_S<T>        reg,
                           CFFrameFuture_t             fullFrameFuture,
                           const std::optional<size_t> registerIdx = {})
                : m_reg(reg), m_fullFrameFuture(fullFrameFuture), m_registerIdx(registerIdx)
            {
                m_regFuture = std::async(
                    std::launch::deferred,
                    [this]() -> std::pair<MDRegisterEntry_S<T>, candleTypes::Error_t>
                    {
                        MDRegisterEntry_S<T> reg;
                        auto                 cfFramePair = m_fullFrameFuture.get();
                        if (cfFramePair.second == candleTypes::Error_t::OK &&
                            m_registerIdx.has_value())
                        {
                            std::vector<u8> tempBuf;
                            tempBuf.insert(tempBuf.end(),
                                           cfFramePair.first.begin() + m_registerIdx.value(),
                                           cfFramePair.first.end());
                            reg.setSerializedRegister(std::move(tempBuf));
                        }
                        return std::make_pair(reg, cfFramePair.second);
                    });
            }

          private:
            MDRegisterEntry_S<T> m_reg;
            CFFrameFuture_t      m_fullFrameFuture;

            const std::optional<size_t> m_registerIdx;
        };

        const canId_t m_canId;

        MDAggregator(const canId_t canId,
                     std::shared_ptr<std::function<CFFrameFuture_t(candleTypes::CANFrameData_t)>>
                                                       sendFrameFunction,
                     std::shared_ptr<std::atomic_bool> senderBusy)
            : m_canId(canId), m_sendFrameFunction(sendFrameFunction), m_senderBusy(senderBusy)
        {
        }

        void forceSendAvailableFrames(size_t maxCount)
        {
            std::unique_lock lock(m_mux);  // Lock to prevent race conditions with user thread

            maxCount = m_frameTypeMap.size() > maxCount ? maxCount : m_frameTypeMap.size();

            for (size_t i = 0; i < maxCount; i++)
            {
                auto it = m_frameTypeMap.begin();
                (*m_sendFrameFunction)(std::move(it->second));
                m_frameTypeMap.erase(it);
            }
        }

        template <typename T>
        inline RegisterFuture<T> writeRegisterAsync(const MDRegisterEntry_S<T> reg)
        {
            size_t serializedRegSize = reg.getSerializedSize();

            std::unique_lock lock(m_mux);  // Lock to prevent race conditions with sending thread

            auto mapIter = m_frameTypeMap.find(MdFrameId_E::WRITE_REGISTER);
            if (mapIter == m_frameTypeMap.end())
            {
                m_frameTypeMap[MdFrameId_E::WRITE_REGISTER] =
                    MDFrame(m_canId, MdFrameId_E::WRITE_REGISTER);
                mapIter = m_frameTypeMap.find(MdFrameId_E::WRITE_REGISTER);
            }
            auto frame = mapIter->second;
            // Small discrepancy over efficiency boost - we only look for a place in the last frame
            // only in order not to look throughout all of the frames (which may have more space
            // left)
            if (CANdleFrame::DATA_MAX_LENGTH - frame.m_data.size() < serializedRegSize)
            {
                (*m_sendFrameFunction)(frame);
                frame = MDFrame(m_canId, MdFrameId_E::WRITE_REGISTER);
            }

            size_t idx = frame.insertRegister(reg);

            return RegisterFuture(std::move(reg), m_sendFrameFunction);
        }

        template <typename T>
        inline RegisterFuture<T> readRegisterAsync(MDRegisterEntry_S<T> reg)
        {
            size_t serializedRegSize = reg.getSerializedSize();

            std::unique_lock lock(m_mux);  // Lock to prevent race conditions with sending thread
            auto             mapIter = m_frameTypeMap.find(MdFrameId_E::READ_REGISTER);
            if (mapIter == m_frameTypeMap.end())
            {
                m_frameTypeMap.emplace(MdFrameId_E::READ_REGISTER,
                                       MDFrame(m_canId, MdFrameId_E::READ_REGISTER));
                mapIter = m_frameTypeMap.find(MdFrameId_E::READ_REGISTER);
            }
            // Small discrepancy over efficiency boost - we only look for a place in the last frame
            // only in order not to look throughout all of the frames (which may have more space
            // left)
            if (CANdleFrame::DATA_MAX_LENGTH - mapIter->second.m_data.size() < serializedRegSize)
            {
                (*m_sendFrameFunction)(std::move(mapIter->second));
                m_frameTypeMap.emplace(MdFrameId_E::READ_REGISTER,
                                       MDFrame(m_canId, MdFrameId_E::READ_REGISTER));
                mapIter = m_frameTypeMap.find(MdFrameId_E::READ_REGISTER);
            }
            const size_t idx = mapIter->second.insertRegister(reg);
            return RegisterFuture<T>(reg, m_sendFrameFunction, idx);
        }

      private:
        const std::shared_ptr<std::function<CFFrameFuture_t(candleTypes::CANFrameData_t)>>
                                                 m_sendFrameFunction;
        std::unordered_map<MdFrameId_E, MDFrame> m_frameTypeMap;
        const std::shared_ptr<std::atomic_bool>  m_senderBusy;

        mutable std::mutex m_mux;
    };
}  // namespace mab