#pragma once

#include "MD.fwd.hpp"
#include "mab_types.hpp"
#include "md_types.hpp"
#include "candle_v2.hpp"
#include "logger.hpp"
#include "manufacturer_data.hpp"

#include <cstring>

#include <array>
#include <queue>
#include <utility>
#include <functional>
#include <tuple>
#include <vector>

namespace mab
{
    class MD
    {
        static constexpr size_t DEFAULT_RESPONSE_SIZE = 23;

        const CandleV2::canId_t m_canId;

        const std::shared_ptr<CandleV2> m_CANdle;

        Logger m_log;

        manufacturerData_S m_mfData;

      public:
        MDRegisters_S m_mdRegisters;

        enum Error_t
        {
            OK,
            REQUEST_INVALID,
            TRANSFER_FAILED,
            NOT_CONNECTED
        };

        MD(CandleV2::canId_t canId, std::shared_ptr<CandleV2> Candle)
            : m_canId(canId), m_CANdle(Candle)
        {
            m_log.m_layer = Logger::ProgramLayer_E::TOP;
            std::stringstream tag;
            tag << "MD" << std::setfill('0') << std::setw(4) << m_canId;
            m_log.m_tag = tag.str();
        }

        Error_t init();

        template <class... T>
        inline std::pair<std::tuple<T...>, Error_t> readRegisters(std::tuple<T...>& regs)
        {
            m_log.debug("Reading register...");
            std::vector<u8> frame;
            frame.push_back((u8)MdFrameId_E::FRAME_READ_REGISTER);
            frame.push_back((u8)0x0);
            auto payload = serializeMDRegisters(regs);
            frame.insert(frame.end(), payload.begin(), payload.end());
            auto readRegResult = m_CANdle->transferCANFrame(
                m_canId, frame, frame.size(), CandleV2::DEFAULT_CAN_TIMEOUT);
            if (readRegResult.second != CandleV2::Error_t::OK)
            {
                m_log.error("Error while reading registers!");
                return std::pair(regs, Error_t::TRANSFER_FAILED);
            }

            if (readRegResult.first.at(0) == 0x41)
            {
                readRegResult.first.erase(
                    readRegResult.first.begin(),
                    readRegResult.first.begin() + 2);  // delete response header
            }
            else
            {
                m_log.error("Error while parsing response!");
                return std::pair(regs, Error_t::TRANSFER_FAILED);
            }
            bool deserializeFailed = deserializeMDRegisters(readRegResult.first, regs);
            if (deserializeFailed)
            {
                m_log.error("Error while parsing response!");
                return std::pair(regs, Error_t::TRANSFER_FAILED);
            }

            return std::pair(regs, Error_t::OK);
        }

        template <class... T>
        inline Error_t writeRegisters(std::tuple<T...>& regs)
        {
            m_log.debug("Writing register...");
            std::vector<u8> frame;
            frame.push_back((u8)MdFrameId_E::FRAME_WRITE_REGISTER);
            frame.push_back((u8)0x0);
            auto payload = serializeMDRegisters(regs);
            frame.insert(frame.end(), payload.begin(), payload.end());
            auto readRegResult = m_CANdle->transferCANFrame(
                m_canId, frame, DEFAULT_RESPONSE_SIZE, CandleV2::DEFAULT_CAN_TIMEOUT);
            if (readRegResult.second != CandleV2::Error_t::OK)
            {
                m_log.error("Error while writing registers!");
                return Error_t::TRANSFER_FAILED;
            }

            if (readRegResult.first.at(0) == 0xA0)
            {
                return Error_t::OK;
            }
            else
            {
                m_log.error("Error in the register write response!");
                return Error_t::OK;
            }
        }

        void blink();

        template <class... T>
        static inline std::vector<u8> serializeMDRegisters(std::tuple<T...>& regs)
        {
            std::vector<u8> serialized;

            std::apply(
                [&](auto&&... reg)
                {
                    ((serialized.insert(serialized.end(),
                                        reg.getSerializedRegister()->begin(),
                                        reg.getSerializedRegister()->end())),
                     ...);
                },
                regs);
            return serialized;
        }

        template <class... T>
        static inline bool deserializeMDRegisters(std::vector<u8>& output, std::tuple<T...>& regs)
        {
            bool failure            = false;
            auto performForEachElem = [&](auto& reg)  // Capture by reference to modify `failure`
            { failure |= !(reg.setSerializedRegister(output)); };

            std::apply([&](auto&... reg) { (performForEachElem(reg), ...); }, regs);

            return failure;
        }
    };
}  // namespace mab