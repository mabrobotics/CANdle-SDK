#pragma once

#include "mab_types.hpp"
#include "md_types.hpp"
#include "logger.hpp"
#include "manufacturer_data.hpp"
#include "candle_types.hpp"
#include "MDStatus.hpp"

#include <cstring>

#include <array>
#include <queue>
#include <utility>
#include <functional>
#include <tuple>
#include <map>
#include <unordered_map>
#include <string>
#include <vector>
#include <iomanip>

namespace mab
{
    struct MDStatus;
    /// @brief Software representation of MD device on the can network
    class MD
    {
        static constexpr size_t DEFAULT_RESPONSE_SIZE = 23;

        Logger m_log;

        manufacturerData_S m_mfData;

      public:
        /// @brief MD can node ID
        const canId_t m_canId;

        /// @brief Helper buffer for interacting with MD registers
        MDRegisters_S m_mdRegisters;

        /// @brief Helper buffer for storing MD status information
        MDStatus m_status;

        /// @brief Possible errors present in this class
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

        /// @brief Check communication with MD device
        /// @return Error if not connected
        Error_t init();

        /// @brief Blink the built-in LEDs
        Error_t blink();

        /// @brief Enable PWM output of the drive
        /// @return
        Error_t enable();

        /// @brief Disable PWM output of the drive
        /// @return
        Error_t disable();

        Error_t reset();

        Error_t clearErrors();

        Error_t save();

        Error_t zero();

        Error_t setCurrentLimit(float currentLimit /*A*/);

        Error_t setTorqueBandwidth(u16 torqueBandwidth /*Hz*/);

        Error_t setMotionMode(mab::Md80Mode_E mode);

        Error_t setPositionPIDparam(float kp, float ki, float kd, float integralMax);

        Error_t setVelocityPIDparam(float kp, float ki, float kd, float integralMax);

        Error_t setImpedanceParams(float kp, float kd);

        Error_t setMaxTorque(float maxTorque /*Nm*/);

        Error_t setProfileVelocity(float profileVelocity /*s^-1*/);

        Error_t setProfileAcceleration(float profileAcceleration /*s^-2*/);

        Error_t setTargetPosition(float position /*rad*/);

        Error_t setTargetVelocity(float velocity /*rad/s*/);

        Error_t setTargetTorque(float torque /*Nm*/);

        std::pair<std::unordered_map<MDStatus::bitPos, MDStatus::StatusItem_S>, Error_t>
        getQuickStatus();

        std::pair<std::unordered_map<MDStatus::bitPos, MDStatus::StatusItem_S>, Error_t>
        getMainEncoderErrors();

        std::pair<std::unordered_map<MDStatus::bitPos, MDStatus::StatusItem_S>, Error_t>
        getOutputEncoderErrors();

        std::pair<std::unordered_map<MDStatus::bitPos, MDStatus::StatusItem_S>, Error_t>
        getCalibrationErrors();

        std::pair<std::unordered_map<MDStatus::bitPos, MDStatus::StatusItem_S>, Error_t>
        getBridgeErrors();

        std::pair<std::unordered_map<MDStatus::bitPos, MDStatus::StatusItem_S>, Error_t>
        getHardwareErrors();

        std::pair<std::unordered_map<MDStatus::bitPos, MDStatus::StatusItem_S>, Error_t>
        getCommunicationErrors();

        std::pair<std::unordered_map<MDStatus::bitPos, MDStatus::StatusItem_S>, Error_t>
        getMotionErrors();

        std::pair<float, Error_t> getPosition();

        std::pair<float, Error_t> getVelocity();

        std::pair<float, Error_t> getTorque();

        std::pair<float, Error_t> getOutputEncoderPosition();

        std::pair<float, Error_t> getOutputEncoderVelocity();

        std::pair<u8, Error_t> getTemperature();

        // TODO: this method is useless, remove it after unit test refactor
        /// @brief Read register from the memory of the MD
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

        /// @brief Read registers from the memory of the MD
        /// @tparam ...T Register underlying types
        /// @param ...regs References to the registers to be read from the MD memory (overwritten by
        /// read)
        /// @return Register values read and error type on failure
        template <class... T>
        inline std::pair<std::tuple<RegisterEntry_S<T>...>, Error_t> readRegisters(
            RegisterEntry_S<T>&... regs)
        {
            auto regTuple   = std::tuple<RegisterEntry_S<T>&...>(regs...);
            auto resultPair = readRegisters(regTuple);
            return resultPair;
        }

        /// @brief Read registers from the memory of the MD
        /// @tparam ...T Register entry underlying type (should be deducible)
        /// @param regs Tuple with register references intended to be read (overwritten by read)
        /// @return Register values read and error type on failure
        template <class... T>
        inline std::pair<std::tuple<RegisterEntry_S<T>...>, Error_t> readRegisters(
            std::tuple<RegisterEntry_S<T>&...>& regs)
        {
            m_log.debug("Reading register...");

            // clear all the values for the incoming data from the MD
            std::apply([&](auto&&... reg) { ((reg.clear()), ...); }, regs);

            // Add protocol read header [0x41, 0x00]
            std::vector<u8> frame;
            frame.push_back((u8)MdFrameId_E::FRAME_READ_REGISTER);
            frame.push_back((u8)0x0);
            // Add serialized register data to be read [LSB addr, MSB addr, payload-bytes...]
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
            // delete response header
            readRegResult.first.erase(readRegResult.first.begin(), readRegResult.first.begin() + 2);
            bool deserializeFailed = deserializeMDRegisters(readRegResult.first, regs);
            if (deserializeFailed)
            {
                m_log.error("Error while parsing response!");
                return std::pair(regs, Error_t::TRANSFER_FAILED);
            }

            return std::pair(regs, Error_t::OK);
        }

        /// @brief Write registers to MD memory
        /// @tparam ...T Register entry underlying type (should be deducible)
        /// @param ...regs Registry references to be written to memory
        /// @return Error on failure
        template <class... T>
        inline Error_t writeRegisters(RegisterEntry_S<T>&... regs)
        {
            auto tuple = std::tuple<RegisterEntry_S<T>&...>(regs...);
            return writeRegisters(tuple);
        }

        /// @briefWrite registers to MD memory
        /// @tparam ...T Register entry underlying type (should be deducible)
        /// @param regs Tuple of register reference to be written
        /// @return Error on failure
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

      private:
        std::function<canTransmitFrame_t> m_transferCANFrame;

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