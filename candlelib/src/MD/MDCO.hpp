#pragma once

#include <unistd.h>
#include "edsEntry.hpp"
#include "mab_types.hpp"
#include "logger.hpp"
#include "manufacturer_data.hpp"
#include "candle_types.hpp"
#include "candle.hpp"
#include "MDStatus.hpp"

#include <cstring>

#include <memory>
#include <utility>
#include <string>
#include <vector>
#include <iomanip>

namespace mab
{

    /// @brief Software representation of MD device on the can network
    class MDCO
    {
        static constexpr size_t DEFAULT_RESPONSE_SIZE         = 23;
        static constexpr u16    SDO_REQUEST_BASE              = 0x600;
        static constexpr u8     INITIATE_SDO_UPLOAD_REQUEST   = 0x40;
        static constexpr u8     INITIATE_SDO_DOWNLOAD_REQUEST = 0x22;

        Logger m_log;

        manufacturerData_S m_mfData;

      public:
        /// @brief MD can node ID
        const canId_t m_canId;

        std::optional<u32> m_timeout;

        /// @brief Possible errors present in this class
        enum class Error_t
        {
            OK,
            REQUEST_INVALID,
            TRANSFER_FAILED,
            NOT_CONNECTED,
            UNKNOWN_OBJECT
        };

        /// @brief Create MD object instance
        /// @param canId can node id of MD
        /// @param candle pointer to candle instance used for communication
        MDCO(canId_t canId, Candle* candle, std::shared_ptr<EDSObjectDictionary> od)
            : m_canId(canId), m_candle(candle), m_od(od)
        {
            m_log.m_layer = Logger::ProgramLayer_E::TOP;
            std::stringstream tag;
            tag << "MDCO" << std::setfill('0') << std::setw(3) << m_canId;
            m_log.m_tag = tag.str();
        }

        Error_t init();

        Error_t enable();

        Error_t disable();

        Error_t blink();

        Error_t save();

        Error_t zero();

        /// @brief Reset the driver
        /// @return
        Error_t reset();

        /// @brief Clear errors present in the driver
        /// @return
        Error_t clearErrors();

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
        Error_t setOperationMode(mab::ModesOfOperation mode);

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

        /// @brief Set target profile deceleration when performing profile movement
        /// @param profileDeceleration deceleration in s^-2
        /// @return
        Error_t setProfileDeceleration(float profileDeceleration /*s^-2*/);

        /// @brief  Set the symmetrical position window at which position reached flag is raised
        /// @param windowSize size of the window in radians. Spans symmetrically around target
        /// position.
        /// @return
        Error_t setPositionWindow(u32 windowSize /*encode tics*/);

        /// @brief Set target position
        /// @param position target position in radians
        /// @return
        Error_t setTargetPosition(i32 position /*encoder ticks*/);

        /// @brief Set target velocity
        /// @param velocity target velocity in radians per second
        /// @return
        Error_t setTargetVelocity(float velocity /*rad/s*/);

        /// @brief Set target torque
        /// @param torque target torque in Nm
        /// @return
        Error_t setTargetTorque(float torque /*Nm*/);

        /// @brief Request main encoder status
        /// @return Main encoder status map with bit positions as ids
        std::pair<const std::unordered_map<MDStatus::EncoderStatusBits, MDStatus::StatusItem_S>,
                  Error_t>
        getMainEncoderStatus();

        /// @brief Request output encoder status
        /// @return Output encoder status map with bit positions as ids
        std::pair<const std::unordered_map<MDStatus::EncoderStatusBits, MDStatus::StatusItem_S>,
                  Error_t>
        getOutputEncoderStatus();

        /// @brief Request calibration status
        /// @return Calibration status map with bit positions as ids
        std::pair<const std::unordered_map<MDStatus::CalibrationStatusBits, MDStatus::StatusItem_S>,
                  Error_t>
        getCalibrationStatus();

        /// @brief Request bridge status
        /// @return Bridge status map with bit positions as ids
        std::pair<const std::unordered_map<MDStatus::BridgeStatusBits, MDStatus::StatusItem_S>,
                  Error_t>
        getBridgeStatus();

        /// @brief Request hardware status
        /// @return Hardware status map with bit positions as ids
        std::pair<const std::unordered_map<MDStatus::HardwareStatusBits, MDStatus::StatusItem_S>,
                  Error_t>
        getHardwareStatus();

        /// @brief Request communication status
        /// @return Communication status map with bit positions as ids
        std::pair<
            const std::unordered_map<MDStatus::CommunicationStatusBits, MDStatus::StatusItem_S>,
            Error_t>
        getCommunicationStatus();

        /// @brief Request motion status
        /// @return Motion status map with bit positions as ids
        std::pair<const std::unordered_map<MDStatus::MotionStatusBits, MDStatus::StatusItem_S>,
                  Error_t>
        getMotionStatus();

        /// @brief Request position of the MD
        /// @return Position in radians
        std::pair<i32, Error_t> getPosition();

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

        Error_t readSDO(EDSEntry& edsEntry) const;

        Error_t writeSDO(EDSEntry& edsEntry) const;

        Error_t resetNMT() const;

        static std::vector<canId_t> discoverOpenMDs(Candle*                              candle,
                                                    std::shared_ptr<EDSObjectDictionary> od);

        inline Error_t enterConfigMode() const
        {
            Error_t err = MDCO::Error_t::OK;

            (*m_od)[0x6040] = (open_types::UNSIGNED16_t)0x8;
            err             = writeSDO((*m_od)[0x6040]);
            if (err != Error_t::OK)
            {
                m_log.error("Error sending control word cmd!");
                return err;
            }
            (*m_od)[0x6040] = (open_types::UNSIGNED16_t)0x6;
            err             = writeSDO((*m_od)[0x6040]);
            if (err != Error_t::OK)
            {
                m_log.error("Error sending control word cmd!");
                return err;
            }

            (*m_od)[0x6040] = (open_types::UNSIGNED16_t)0xf;
            err             = writeSDO((*m_od)[0x6040]);
            if (err != Error_t::OK)
            {
                m_log.error("Error sending control word cmd!");
                return err;
            }
            err            = readSDO((*m_od)[0x6041]);
            u16 statusWord = (u16)(open_types::UNSIGNED16_t)(*m_od)[0x6041];
            m_log.debug("Statusword: 0x%x", statusWord);
            if (err != Error_t::OK)
            {
                m_log.error("Error setting config mode!");
                return err;
            }

            (*m_od)[0x6060] = (open_types::INTEGER8_t)-2;
            err             = writeSDO((*m_od)[0x6060]);
            if (err != Error_t::OK)
            {
                m_log.error("Error setting config mode!");
                return err;
            }
            usleep(3'000);

            err = readSDO((*m_od)[0x6061]);
            if ((i8)(open_types::INTEGER8_t)(*m_od)[0x6061] != -1)
            {
                m_log.error("Coudl not enter service mode");
                m_log.error("Current mode: %i", (i8)(open_types::INTEGER8_t)((*m_od)[0x6061]));
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(2));
            err = readSDO((*m_od)[0x6060]);
            if ((i8)(open_types::INTEGER8_t)(*m_od)[0x6060] != -2)
            {
                m_log.error("Coudl not enter service mode");
                m_log.error("Current mode: %i", (i8)(open_types::INTEGER8_t)((*m_od)[0x6061]));
            }
            return err;
        }

      private:
        Candle* m_candle;

        /// @brief Generate the Object Dictionary from the EDS file
        /// @return A vector of edsObject representing the Object Dictionary
        std::shared_ptr<EDSObjectDictionary> m_od;

        inline std::pair<std::vector<u8>, mab::candleTypes::Error_t> transferCanOpenFrame(
            i16 Id, std::vector<u8> frameToSend, size_t responseSize) const
        {
            if (m_candle == nullptr)
            {
                m_log.error("Candle empty!");
                return {{}, candleTypes::Error_t::DEVICE_NOT_CONNECTED};
            }
            auto result = m_candle->transferCANFrame(
                Id, frameToSend, responseSize, m_timeout.value_or(DEFAULT_CAN_TIMEOUT));

            if (result.second != candleTypes::Error_t::OK)
            {
                m_log.error("Error while transfering CAN frame!");
            }
            return result;
        }
    };

}  // namespace mab
