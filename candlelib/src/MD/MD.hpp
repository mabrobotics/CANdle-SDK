#pragma once

#include "mab_types.hpp"
#include "md_types.hpp"
#include "logger.hpp"
#include "manufacturer_data.hpp"
#include "candle_types.hpp"
#include "MDStatus.hpp"
#include "candle.hpp"

#include <cstring>

#include <array>
#include <queue>
#include <type_traits>
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

        std::optional<u32> m_timeout;

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
        MD(canId_t canId, Candle* candle) : m_canId(canId), m_candle(candle)
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

        /// @brief Reset the driver
        /// @return
        Error_t reset();

        /// @brief Clear errors present in the driver
        /// @return
        Error_t clearErrors();

        /// @brief Save configuration data to the memory
        /// @return
        Error_t save();

        /// @brief Zero out the position of the encoder
        /// @return
        Error_t zero();

        /// @brief Set current limit associated with motor that is driven
        /// @param currentLimit Current limit in Amps
        /// @return
        Error_t setCurrentLimit(float currentLimit /*A*/);

        /// @brief Set update rate for the torque control loop
        /// @param torqueBandwidth Update rate in Hz
        /// @return
        Error_t setTorqueBandwidth(u16 torqueBandwidth /*Hz*/);

        /// @brief Set controller mode
        /// @param mode Mode selected
        /// @return
        Error_t setMotionMode(mab::MdMode_E mode);

        /// @brief Set position controller PID parameters
        /// @param kp
        /// @param ki
        /// @param kd
        /// @param integralMax
        /// @return
        Error_t setPositionPIDparam(float kp, float ki, float kd, float integralMax);

        /// @brief Set velocity controller PID parameters
        /// @param kp
        /// @param ki
        /// @param kd
        /// @param integralMax
        /// @return
        Error_t setVelocityPIDparam(float kp, float ki, float kd, float integralMax);

        /// @brief Set impedance controller parameters
        /// @param kp
        /// @param kd
        /// @return
        Error_t setImpedanceParams(float kp, float kd);

        /// @brief Set max torque to be output by the controller
        /// @param maxTorque max torque value in Nm
        /// @return
        Error_t setMaxTorque(float maxTorque /*Nm*/);

        /// @brief Set target velocity of the profile movement
        /// @param profileVelocity
        /// @return
        Error_t setProfileVelocity(float profileVelocity /*s^-1*/);

        /// @brief Set target profile acceleration when performing profile movement
        /// @param profileAcceleration
        /// @return
        Error_t setProfileAcceleration(float profileAcceleration /*s^-2*/);

        /// @brief Set target position
        /// @param position target position in radians
        /// @return
        Error_t setTargetPosition(float position /*rad*/);

        /// @brief Set target velocity
        /// @param velocity target velocity in radians per second
        /// @return
        Error_t setTargetVelocity(float velocity /*rad/s*/);

        /// @brief Set target torque
        /// @param torque target torque in Nm
        /// @return
        Error_t setTargetTorque(float torque /*Nm*/);

        /// @brief Request quick status update
        /// @return Quick Status map with bit positions as ids
        std::pair<const std::unordered_map<const MDStatus::QuickStatusBits, MDStatus::StatusItem_S>,
                  Error_t>
        getQuickStatus();

        /// @brief Request main encoder status
        /// @return Main encoder status map with bit positions as ids
        std::pair<
            const std::unordered_map<const MDStatus::EncoderStatusBits, MDStatus::StatusItem_S>,
            Error_t>
        getMainEncoderStatus();

        /// @brief Request output encoder status
        /// @return Output encoder status map with bit positions as ids
        std::pair<
            const std::unordered_map<const MDStatus::EncoderStatusBits, MDStatus::StatusItem_S>,
            Error_t>
        getOutputEncoderStatus();

        /// @brief Request calibration status
        /// @return Calibration status map with bit positions as ids
        std::pair<
            const std::unordered_map<const MDStatus::CalibrationStatusBits, MDStatus::StatusItem_S>,
            Error_t>
        getCalibrationStatus();

        /// @brief Request bridge status
        /// @return Bridge status map with bit positions as ids
        std::pair<
            const std::unordered_map<const MDStatus::BridgeStatusBits, MDStatus::StatusItem_S>,
            Error_t>
        getBridgeStatus();

        /// @brief Request hardware status
        /// @return Hardware status map with bit positions as ids
        std::pair<
            const std::unordered_map<const MDStatus::HardwareStatusBits, MDStatus::StatusItem_S>,
            Error_t>
        getHardwareStatus();

        /// @brief Request communication status
        /// @return Communication status map with bit positions as ids
        std::pair<const std::unordered_map<const MDStatus::CommunicationStatusBits,
                                           MDStatus::StatusItem_S>,
                  Error_t>
        getCommunicationStatus();

        /// @brief Request motion status
        /// @return Motion status map with bit positions as ids
        std::pair<
            const std::unordered_map<const MDStatus::MotionStatusBits, MDStatus::StatusItem_S>,
            Error_t>
        getMotionStatus();

        /// @brief Request position of the MD
        /// @return Position in radians
        std::pair<float, Error_t> getPosition();

        /// @brief Request current velocity of the MD
        /// @return Velocity in radians per second
        std::pair<float, Error_t> getVelocity();

        /// @brief Request current torque of the MD
        /// @return Torque in Nm
        std::pair<float, Error_t> getTorque();

        /// @brief Request output position if external encoder is configured
        /// @return Position in radians
        std::pair<float, Error_t> getOutputEncoderPosition();

        /// @brief Request output velocity if external encoder is configured
        /// @return Velocity in radians per second
        std::pair<float, Error_t> getOutputEncoderVelocity();

        /// @brief Request motor temperature
        /// @return Temperature in degrees celsius from 0 C to 125 C
        std::pair<u8, Error_t> getTemperature();

        /// @brief Read register from the memory of the MD
        /// @tparam T Register entry underlying type (should be deducible)
        /// @param reg Register entry reference to be read from memory (reference is
        /// overwritten by received data)
        /// @return Error type on failure
        template <class T>
        inline Error_t readRegister(MDRegisterEntry_S<T>& reg)
        {
            auto regTuple = std::make_tuple(std::reference_wrapper(reg));
            auto result   = readRegisters(regTuple);
            reg           = std::get<0>(regTuple);
            return result;
        }

        /// @brief Write register to the memory of the MD
        /// @tparam T Register entry underlying type (should be deducible)
        /// @param reg Register entry reference to be written to memory
        /// @return Error on failure
        template <class T>
        inline Error_t writeRegister(MDRegisterEntry_S<T>& reg)
        {
            // Check if register has read-only access level
            if (reg.m_accessLevel == RegisterAccessLevel_E::RO)
            {
                std::string errMsg =
                    "Attempt to write to read-only register: " + std::string(reg.m_name);
                m_log.error(errMsg.c_str());
                return Error_t::REQUEST_INVALID;
            }

            auto regTuple = std::make_tuple(std::reference_wrapper(reg));
            return writeRegisters(regTuple);
        }

        /// @brief Read registers from the memory of the MD
        /// @tparam ...T Register underlying types
        /// @param ...regs References to the registers to be read from the MD memory (overwritten by
        /// read)
        /// @return Error type on failure
        template <class... T>
        inline Error_t readRegisters(MDRegisterEntry_S<T>&... regs)
        {
            auto regTuple   = std::tuple<MDRegisterEntry_S<T>&...>(regs...);
            auto resultPair = readRegisters(regTuple);
            return resultPair;
        }

        /// @brief Read registers from the memory of the MD
        /// @tparam ...T Register entry underlying type (should be deducible)
        /// @param regs Tuple with register references intended to be read (overwritten by read)
        /// @return Error type on failure
        template <class... T>
        inline Error_t readRegisters(std::tuple<MDRegisterEntry_S<T>&...>& regs)
        {
            m_log.debug("Reading registers...");

            // Check if any registers have write-only access level
            bool        hasWriteOnlyRegister = false;
            std::string writeOnlyRegNames;
            std::apply(
                [&](auto&&... reg)
                {
                    ((hasWriteOnlyRegister |=
                      (reg.m_accessLevel == RegisterAccessLevel_E::WO)
                          ? (writeOnlyRegNames.append(writeOnlyRegNames.empty() ? "" : ", ")
                                 .append(std::string(reg.m_name)),
                             true)
                          : false),
                     ...);
                },
                regs);

            if (hasWriteOnlyRegister)
            {
                std::string errMsg = "Attempt to read write-only registers: " + writeOnlyRegNames;
                m_log.error(errMsg.c_str());
                return Error_t::REQUEST_INVALID;
            }

            // clear all the values for the incoming data from the MD
            std::apply([&](auto&&... reg) { ((reg.clear()), ...); }, regs);

            // Add protocol read header [0x41, 0x00]
            std::vector<u8> frame;
            frame.push_back((u8)MdFrameId_E::FRAME_READ_REGISTER);
            frame.push_back((u8)0x0);
            // Add serialized register data to be read [LSB addr, MSB addr, payload-bytes...]
            auto payload = serializeMDRegisters(regs);
            frame.insert(frame.end(), payload.begin(), payload.end());
            auto readRegResult = transferCanFrame(frame, frame.size());
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
                return Error_t::TRANSFER_FAILED;
            }

            return Error_t::OK;
        }

        /// @brief Write registers to MD memory
        /// @tparam ...T Register entry underlying type (should be deducible)
        /// @param ...regs Registry references to be written to memory
        /// @return Error on failure
        template <class... T>
        inline Error_t writeRegisters(MDRegisterEntry_S<T>&... regs)
        {
            auto tuple = std::tuple<MDRegisterEntry_S<T>&...>(regs...);
            return writeRegisters(tuple);
        }

        /// @brief Write registers to MD memory
        /// @tparam ...T Register entry underlying type (should be deducible)
        /// @param regs Tuple of register reference to be written
        /// @return Error on failure
        template <class... T>
        inline Error_t writeRegisters(std::tuple<MDRegisterEntry_S<T>&...>& regs)
        {
            m_log.debug("Writing register...");
            // Print registers names
            std::apply([&](auto&&... reg)
                       { ((m_log.debug("Register %s", reg.m_name.data())), ...); },
                       regs);

            // Check has already been performed in the variadic template version if coming from
            // there Double-check here for direct tuple calls
            bool        hasReadOnlyRegister = false;
            std::string readOnlyRegNames;
            std::apply(
                [&](auto&&... reg)
                {
                    ((hasReadOnlyRegister |=
                      (reg.m_accessLevel == RegisterAccessLevel_E::RO)
                          ? (readOnlyRegNames.append(readOnlyRegNames.empty() ? "" : ", ")
                                 .append(std::string(reg.m_name)),
                             true)
                          : false),
                     ...);
                },
                regs);

            if (hasReadOnlyRegister)
            {
                std::string errMsg = "Attempt to write to read-only registers: " + readOnlyRegNames;
                m_log.error(errMsg.c_str());
                return Error_t::REQUEST_INVALID;
            }

            std::vector<u8> frame;
            frame.push_back((u8)MdFrameId_E::FRAME_WRITE_REGISTER);
            frame.push_back((u8)0x0);
            auto payload = serializeMDRegisters(regs);
            frame.insert(frame.end(), payload.begin(), payload.end());
            auto readRegResult = transferCanFrame(frame, DEFAULT_RESPONSE_SIZE);

            if (readRegResult.first.at(0) == 0xA0)
            {
                return Error_t::OK;
            }
            else
            {
                m_log.error("Error in the register write response!");
                return Error_t::TRANSFER_FAILED;
            }
        }

        /// @brief Debugging method to test communication efficiency
        void testLatency();

        static std::vector<canId_t> discoverMDs(Candle* candle);

      private:
        const Candle* m_candle;

        inline const Candle* getCandle() const
        {
            if (m_candle != nullptr)
            {
                return m_candle;
            }
            m_log.error("Candle device empty!");
            return nullptr;
        }

        template <class... T>
        static inline std::vector<u8> serializeMDRegisters(
            std::tuple<MDRegisterEntry_S<T>&...>& regs)
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
        static inline bool deserializeMDRegisters(std::vector<u8>&                      output,
                                                  std::tuple<MDRegisterEntry_S<T>&...>& regs)
        {
            bool failure            = false;
            auto performForEachElem = [&](auto& reg)  // Capture by reference to modify 'failure'
            { failure |= !(reg.setSerializedRegister(output)); };

            std::apply([&](auto&... reg) { (performForEachElem(reg), ...); }, regs);

            return failure;
        }

        inline std::pair<std::vector<u8>, mab::candleTypes::Error_t> transferCanFrame(
            std::vector<u8> frameToSend, size_t responseSize) const
        {
            if (m_candle == nullptr)
            {
                m_log.error("Candle empty!");
                return {{}, candleTypes::Error_t::DEVICE_NOT_CONNECTED};
            }
            auto result = getCandle()->transferCANFrame(
                m_canId, frameToSend, responseSize, m_timeout.value_or(DEFAULT_CAN_TIMEOUT));

            if (result.second != candleTypes::Error_t::OK)
            {
                m_log.error("Error while transfering CAN frame!");
            }
            return result;
        }
    };

}  // namespace mab
