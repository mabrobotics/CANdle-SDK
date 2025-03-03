#pragma once

#include "mab_types.hpp"
#include "md_types.hpp"
#include "logger.hpp"
#include "manufacturer_data.hpp"
#include "candle_types.hpp"

#include <cstring>

#include <array>
#include <queue>
#include <utility>
#include <functional>
#include <tuple>
#include <vector>
#include <iomanip>

namespace mab
{
    /// @brief Software representation of MD device on the can network
    class MD
    {
        static constexpr size_t DEFAULT_RESPONSE_SIZE = 23;

        const canId_t m_canId;

        Logger m_log;

        manufacturerData_S m_mfData;

        std::function<canTransmitFrame_t> m_transferCANFrame;

      public:
        MDRegisters_S m_mdRegisters;

        enum Error_t
        {
            OK,
            REQUEST_INVALID,
            TRANSFER_FAILED,
            NOT_CONNECTED
        };

        /// @brief Create MD object instance
        /// @param canId can node id of MD
        /// @param transferCANFrame
        MD(canId_t canId, std::function<canTransmitFrame_t> transferCANFrame)
            : m_canId(canId), m_transferCANFrame(transferCANFrame)
        {
            m_log.m_layer = Logger::ProgramLayer_E::TOP;
            std::stringstream tag;
            tag << "MD" << std::setfill('0') << std::setw(4) << m_canId;
            m_log.m_tag = tag.str();
        }

        /// @brief Initialize communication with MD device
        /// @return
        Error_t init();

        /// @brief Blink the built-in LEDs
        void blink();

        /// @brief Enable PWM output of the drive
        /// @return
        Error_t enable();

        /// @brief Disable PWM output of the drive
        /// @return
        Error_t disable();

        /// @brief Read registers from the memory of the MD
        /// @tparam T Register entry underlying type (should be deducible)
        /// @param reg Register entries references to be read from memory (references are
        /// overwritten by received data)
        /// @return
        template <class T>
        inline std::pair<RegisterEntry_S<T>, Error_t> readRegister(RegisterEntry_S<T>& reg)
        {
            auto regTuple   = std::make_tuple(std::reference_wrapper(reg));
            auto resultPair = readRegisters(regTuple);
            reg.value       = std::get<0>(regTuple).value;
            return std::pair(reg, resultPair.second);
        }

        template <class... T>
        inline std::pair<std::tuple<RegisterEntry_S<T>...>, Error_t> readRegisters(
            RegisterEntry_S<T>&... regs)
        {
            auto regTuple   = std::tuple<RegisterEntry_S<T>&...>(regs...);
            auto resultPair = readRegisters(regTuple);
            return resultPair;
        }

        template <class... T>
        inline std::pair<std::tuple<RegisterEntry_S<T>...>, Error_t> readRegisters(
            std::tuple<RegisterEntry_S<T>&...>& regs)
        {
            m_log.debug("Reading register...");
            std::apply([&](auto&&... reg) { ((reg.clear()), ...); }, regs);
            std::vector<u8> frame;
            frame.push_back((u8)MdFrameId_E::FRAME_READ_REGISTER);
            frame.push_back((u8)0x0);
            auto payload = serializeMDRegisters(regs);
            frame.insert(frame.end(), payload.begin(), payload.end());
            auto readRegResult =
                m_transferCANFrame(m_canId, frame, frame.size(), DEFAULT_CAN_TIMEOUT);
            if (readRegResult.second != candleTypes::Error_t::OK)
            {
                m_log.error("Error while reading registers!");
                return std::pair(regs, Error_t::TRANSFER_FAILED);
            }
            // TODO: for some reason MD sends first byte as 0x0, investigate
            //  if (readRegResult.first.at(0) == 0x41)
            //  {
            //      readRegResult.first.erase(
            //          readRegResult.first.begin(),
            //          readRegResult.first.begin() + 2);  // delete response header
            //  }
            //  else
            //  {
            //      m_log.error("Error while parsing response!");
            //      return std::pair(regs, Error_t::TRANSFER_FAILED);
            //  }
            readRegResult.first.erase(readRegResult.first.begin(),
                                      readRegResult.first.begin() + 2);  // delete response header
            bool deserializeFailed = deserializeMDRegisters(readRegResult.first, regs);
            if (deserializeFailed)
            {
                m_log.error("Error while parsing response!");
                return std::pair(regs, Error_t::TRANSFER_FAILED);
            }

            return std::pair(regs, Error_t::OK);
        }

        template <class... T>
        inline Error_t writeRegisters(RegisterEntry_S<T>&... regs)
        {
            auto tuple = std::tuple<RegisterEntry_S<T>&...>(regs...);
            return writeRegisters(tuple);
        }

        template <class... T>
        inline Error_t writeRegisters(std::tuple<RegisterEntry_S<T>&...>& regs)
        {
            m_log.debug("Writing register...");
            std::vector<u8> frame;
            frame.push_back((u8)MdFrameId_E::FRAME_WRITE_REGISTER);
            frame.push_back((u8)0x0);
            auto payload = serializeMDRegisters(regs);
            frame.insert(frame.end(), payload.begin(), payload.end());
            auto readRegResult =
                m_transferCANFrame(m_canId, frame, DEFAULT_RESPONSE_SIZE, DEFAULT_CAN_TIMEOUT);
            if (readRegResult.second != candleTypes::Error_t::OK)
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

        template <class... T>
        static inline std::vector<u8> serializeMDRegisters(std::tuple<RegisterEntry_S<T>&...>& regs)
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
        static inline bool deserializeMDRegisters(std::vector<u8>&                    output,
                                                  std::tuple<RegisterEntry_S<T>&...>& regs)
        {
            bool failure            = false;
            auto performForEachElem = [&](auto& reg)  // Capture by reference to modify 'failure'
            { failure |= !(reg.setSerializedRegister(output)); };

            std::apply([&](auto&... reg) { (performForEachElem(reg), ...); }, regs);

            return failure;
        }
    };
}  // namespace mab