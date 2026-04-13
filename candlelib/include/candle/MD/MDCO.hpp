#pragma once

#include <unistd.h>
#include "candle/objectDictionary/edsEntry.hpp"
#include "mab_types.hpp"
#include "logger.hpp"
#include "manufacturer_data.hpp"
#include "candle/communication_device/candle_types.hpp"
#include "candle/communication_device/candle.hpp"
#include "candle/MD/MDStatus.hpp"

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
        static constexpr u16 SDO_REQUEST_BASE              = 0x600;
        static constexpr u8  INITIATE_SDO_UPLOAD_REQUEST   = 0x40;
        static constexpr u8  INITIATE_SDO_DOWNLOAD_REQUEST = 0x22;

        Logger m_log;

        manufacturerData_S m_mfData;

      public:
        /// @brief MD can node ID
        const canId_t m_canId;

        std::optional<u32> m_timeout;

        /// @brief Possible errors present in this class
        enum class Error_t
        {
            UNKNOWN_ERROR,
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

        Error_t reset();

        Error_t clearErrors();

        Error_t setCurrentLimit(float currentLimit /*A*/);

        Error_t setTorqueBandwidth(u16 torqueBandwidth /*Hz*/);

        Error_t setOperationMode(mab::ModesOfOperation mode);

        Error_t setPositionPIDparam(float kp, float ki, float kd, float integralMax);

        Error_t setVelocityPIDparam(float kp, float ki, float kd, float integralMax);

        Error_t setImpedanceParams(float kp, float kd);

        Error_t setMaxTorque(float maxTorque /*Nm*/);

        Error_t setProfileVelocity(float profileVelocity /*s^-1*/);

        Error_t setProfileAcceleration(float profileAcceleration /*s^-2*/);

        Error_t setProfileDeceleration(float profileDeceleration /*s^-2*/);

        Error_t setPositionWindow(u32 windowSize /*encode tics*/);

        Error_t setTargetPosition(i32 position /*encoder ticks*/);

        Error_t setTargetVelocity(float velocity /*rad/s*/);

        Error_t setTargetTorque(float torque /*Nm*/);

        std::pair<const std::unordered_map<MDStatus::EncoderStatusBits, MDStatus::StatusItem_S>,
                  Error_t>
        getMainEncoderStatus();

        std::pair<const std::unordered_map<MDStatus::EncoderStatusBits, MDStatus::StatusItem_S>,
                  Error_t>
        getOutputEncoderStatus();

        std::pair<const std::unordered_map<MDStatus::CalibrationStatusBits, MDStatus::StatusItem_S>,
                  Error_t>
        getCalibrationStatus();

        std::pair<const std::unordered_map<MDStatus::BridgeStatusBits, MDStatus::StatusItem_S>,
                  Error_t>
        getBridgeStatus();

        std::pair<const std::unordered_map<MDStatus::HardwareStatusBits, MDStatus::StatusItem_S>,
                  Error_t>
        getHardwareStatus();

        std::pair<
            const std::unordered_map<MDStatus::CommunicationStatusBits, MDStatus::StatusItem_S>,
            Error_t>
        getCommunicationStatus();

        std::pair<const std::unordered_map<MDStatus::MotionStatusBits, MDStatus::StatusItem_S>,
                  Error_t>
        getMotionStatus();

        std::pair<i32, Error_t> getPosition();

        std::pair<float, Error_t> getVelocity();

        std::pair<float, Error_t> getTorque();

        std::pair<float, Error_t> getOutputEncoderPosition();

        std::pair<float, Error_t> getOutputEncoderVelocity();

        std::pair<u8, Error_t> getTemperature();

        std::pair<bool, Error_t> targetReached();

        Error_t readSDO(EDSEntry& edsEntry) const;

        Error_t writeSDO(EDSEntry& edsEntry) const;

        Error_t resetNMT() const;

        static std::vector<canId_t> discoverOpenMDs(Candle*                              candle,
                                                    std::shared_ptr<EDSObjectDictionary> od);

        Error_t enterConfigMode() const;

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
                Id, frameToSend, responseSize, m_timeout.value_or(DEFAULT_CAN_TIMEOUT + 1));

            if (result.second != candleTypes::Error_t::OK)
            {
                m_log.error("Error while transfering CAN frame!");
            }
            return result;
        }
    };

}  // namespace mab
