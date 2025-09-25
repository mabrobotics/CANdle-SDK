#pragma once

#include "mab_types.hpp"
#include "md_types.hpp"
#include "logger.hpp"
#include "manufacturer_data.hpp"
#include "candle_types.hpp"
#include "MDStatus.hpp"
#include "candle.hpp"
#include "OD.hpp"

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
#include <algorithm>
#include <chrono>

namespace mab
{

    /// @brief Parameters for the move operation
    struct moveParameter
    {
        u32 MaxSpeed     = 220;
        u16 MaxCurrent   = 500;
        u32 RatedCurrent = 1000;
        u16 MaxTorque    = 500;
        u32 RatedTorque  = 1000;
        i32 accLimit     = 1000;
        i32 dccLimit     = 1000;
        f32 kp           = 1.0;
        f32 kd           = 0.0;
        f32 ki           = 0.0;
        i16 torqueff     = 0x00;
    };

    /// @brief Modes of operation for the MD device cf. 0x6060 in the CANopen object dictionary
    enum ModesOfOperation : i8
    {
        Impedance          = -3,
        Service            = -2,
        Idle               = 0,
        ProfilePosition    = 1,
        ProfileVelocity    = 3,
        CyclicSyncPosition = 8,
        CyclicSyncVelocity = 9,
    };

    /// @brief Software representation of MD device on the can network
    class MDCO
    {
        static constexpr size_t DEFAULT_RESPONSE_SIZE = 23;
        static constexpr u16    SDO_REQUEST_BASE      = 0x600;

        Logger m_log;

        manufacturerData_S m_mfData;

      public:
        /// @brief MD can node ID
        const canId_t m_canId;

        std::optional<u32> m_timeout;

        /// @brief Possible errors present in this class
        enum Error_t
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
        MDCO(canId_t canId, Candle* candle) : m_canId(canId), m_candle(candle)
        {
            m_log.m_layer = Logger::ProgramLayer_E::TOP;
            std::stringstream tag;
            tag << "MDCO" << std::setfill('0') << std::setw(3) << m_canId;
            m_log.m_tag = tag.str();
        }

        /// @brief use SDO message in order to send all value needed to configure the motor for
        /// moving
        /// @param param struct containing all the parameters needed to configure the motor
        /// @return error_t indicating the result of the operation
        Error_t setProfileParameters(moveParameter& param);

        /// @brief Enable the driver with the specified mode of operation
        /// @param mode Mode of operation to set cf 0x6060 in the CANopen object dictionary
        /// @return Error_t indicating the result of the operation
        Error_t enableDriver(ModesOfOperation mode);

        /// @brief Disable the driver
        /// @return Error_t indicating the result of the operation
        Error_t disableDriver();

        /// @brief Move to desired position
        /// @param DesiredPos desired position
        /// @param timeoutMillis aimed position during timeout time [ms]
        /// @note The motor need to have the profile parameters set & a mode of operation set before
        /// calling this function
        void movePosition(i32 DesiredPos, i16 timeoutMillis = 5000);

        /// @brief Move to desired speed
        /// @param DesiredSpeed desired speed [RPM]
        /// @param timeoutMillis aimed speed during timeout time [ms]
        /// @note The motor need to have the profile parameters set & a mode of operation set before
        /// calling this function
        void moveSpeed(i32 DesiredSpeed, i16 timeoutMillis = 5000);

        /// @brief Move to desired position with impedance control
        /// @param desiredSpeed desired speed [RPM]
        /// @param targetPos desired position
        /// @param param struct containing all the parameters needed to configure the motor
        /// @param timeoutMillis keep Impedance mode during timeout time [ms]
        /// @return Error_t indicating the result of the operation
        /// @details This function sets the motor in impedance control mode and moves it to the
        /// specified position with the given speed, gains, and torque.
        Error_t moveImpedance(i32            desiredSpeed,
                              i32            targetPos,
                              moveParameter& param,
                              i16            timeoutMillis = 5000);

        /// @brief Blink the built-in LEDs with CANopen command
        Error_t blinkOpenTest();

        /// @brief Reset the driver with CANopen command
        /// @return Error_t indicating the result of the operation
        Error_t openReset();

        /// @brief Clear errors present in the driver
        /// @param level 1 => clear error, 2 => clear warning, 3 => clear both
        /// @return Error_t indicating the result of the operation
        Error_t clearOpenErrors(i16 level);

        /// @brief Change CANopen config the command need to be save before shutoff the motors
        /// @param newID new id of the motor
        /// @param newBaud new baudRate
        /// @param watchdog new watchdog
        /// @return Error_t indicating the result of the operation
        Error_t newCanOpenConfig(i32 newID, i32 newBaud, u32 watchdog);

        /// @brief Perform a encoder test
        /// @param Main Main=1 if you want to perform the test on the main encoder
        /// @param output output=1 if you want to perform the test on the output encoder
        /// @return Error_t indicating the result of the operation
        Error_t testEncoder(bool Main, bool output);

        /// @brief Perform a encoder calibration
        /// @param Main Main=1 if you want to perform the calibration on the main encoder
        /// @param output output=1 if you want to perform the calibration on the output encoder
        /// @return Error_t indicating the result of the operation
        Error_t encoderCalibration(bool Main, bool output);

        /// @brief test the heartbeat of the MD device give time between two heartbeat
        /// @return Error_t indicating the result of the operation
        Error_t testHeartbeat();

        /// @brief Save configuration data to the memory
        /// @return Error_t indicating the result of the operation
        Error_t openSave();

        /// @brief Zero out the position of the encoder
        /// @return Error_t indicating the result of the operation
        Error_t openZero();

        /// @brief read a value from a can open register using SDO can frame
        /// @param index index from the object dictionary where the user want to read the value
        /// @param subindex subindex from the object dictionary where the user want to read the
        /// value
        /// @param force if true, skip the check if the object is readable
        /// @return Error on failure
        Error_t readOpenRegisters(i16 index, u8 subindex, bool force = false);

        /// @brief write a value in a can open register using SDO segmented can frame
        /// @param index index from the object dictionary where the user want to write the value
        /// @param subindex subindex from the object dictionary where the user want to write the
        /// value
        /// @param dataString string containing the data to write
        /// @param force if true, skip the check if the object is writable
        /// @return Error on failure
        Error_t writeLongOpenRegisters(i16                index,
                                       short              subindex,
                                       const std::string& dataString,
                                       bool               force = false);

        /// @brief read a value from a can open register using SDO segmented transfer can frame (if
        /// data > 4 bit)
        /// @param index index from the object dictionary where the user want to read the value
        /// @param subindex subindex from the object dictionary where the user want to read the
        /// value
        /// @param outData vector where the data will be stored
        /// @param silent if true, nothings wil be print on the terminal
        /// @return Error on failure
        Error_t readLongOpenRegisters(i16              index,
                                      short            subindex,
                                      std::vector<u8>& outData,
                                      bool             silent = false);

        /// @brief return a value from a can open register using SDO can frame
        /// @param index index from the object dictionary where the user want to read the value
        /// @param subindex subindex from the object dictionary where the user want to read the
        /// value
        /// @return the value contained in the register or -1 if error
        i32 getValueFromOpenRegister(i16 index, u8 subindex);

        /// @brief write a value in a can open register using SDO can frame
        /// @param index index from the object dictionary where the user want to write the value
        /// @param subindex subindex from the object dictionary where the user want to write the
        /// value
        /// @param data value to write
        /// @param size size of the data to write (1,2,4)
        /// @param force if true, skip the check if the object is writable
        /// @return Error on failure
        Error_t writeOpenRegisters(
            i16 index, short subindex, i32 data, short size = 0, bool force = false);

        /// @brief write a value in a can open register using SDO can frame
        /// @param name name of the object to write
        /// @param data value to write
        /// @param size size of the data to write (1,2,4)
        /// @param force if true, skip the check if the object is writable
        /// @return Error on failure
        Error_t writeOpenRegisters(const std::string& name,
                                   u32                data,
                                   u8                 size  = 0,
                                   bool               force = false);

        /// @brief write a value in a can open register using PDO can frame
        /// @param index id of pdo to send (200/300/400/500 + node_id)
        /// @param data value to write
        /// @return Error on failure
        Error_t writeOpenPDORegisters(i16 index, std::vector<u8> data);

        /// @brief write a value in a can open register using PDO can frame
        /// @param index id of pdo to send (200/300/400/500 + node_id)
        /// @param data value to write
        /// @return Error on failure
        Error_t sendCustomData(i16 index, std::vector<u8> data);

        /// @brief try to communicate with canOpen frame (SDO) with all the possible id
        /// @param candle
        /// @return a vector with all id with a MD attach in CANopen communication
        static std::vector<canId_t> discoverOpenMDs(Candle* candle);

        /// @brief Return the size of the data of an EDS object
        /// @param index Index of the object in the Object Dictionary
        /// @param subIndex Subindex of the object in the Object Dictionary
        /// @return Size of the data in bytes, or 0 if the object is a string or -1 if the object is
        /// not found
        i8 dataSizeOfEdsObject(const u32 index, const u8 subIndex);

        /// @brief Display all information about the MD device
        /// @details This function prints the all the actual register value, device type, and all
        /// objects in the Object Dictionary.
        void printAllInfo();

      private:
        const Candle* m_candle;

        /// @brief Generate the Object Dictionary from the EDS file
        /// @return A vector of edsObject representing the Object Dictionary
        const std::vector<edsObject> ObjectDictionary =
            generateObjectDictionary();  // Object dictionary generated from the EDS file

        /// @brief Find an object in the Object Dictionary by its name
        /// @param searchTerm name of the object to find
        /// @param index Output parameter to store the found index
        /// @param subIndex Output parameter to store the found subindex
        /// @return Error_t indicating the result of the operation
        /// @details If multiple objects with the same name are found, only the first one is
        /// returned, and a warning is logged.
        Error_t findObjectByName(const std::string& searchTerm, u32& index, u8& subIndex);

        /// @brief Check if an object is writable
        /// @param index Index of the object in the Object Dictionary
        /// @param subIndex Subindex of the object in the Object Dictionary
        /// @return Error_t indicating whether the object is writable or not
        /// @details This function checks the Object Dictionary to determine if the specified object
        /// is writable. If the object is not found or not writable, it returns an error.
        /// If the object is writable, it returns OK.
        Error_t isWritable(const u32 index, const u8 subIndex);

        /// @brief Check if an object is readable
        /// @param index Index of the object in the Object Dictionary
        /// @param subIndex Subindex of the object in the Object Dictionary
        /// @return Error_t indicating whether the object is readable or not
        /// @details This function checks the Object Dictionary to determine if the specified object
        /// is readable. If the object is not found or not readable, it returns an error.
        /// If the object is readable, it returns OK.
        Error_t isReadable(const u32 index, const u8 subIndex);

        inline const Candle* getCandle() const
        {
            if (m_candle != nullptr)
            {
                return m_candle;
            }
            m_log.error("Candle device empty!");
            return nullptr;
        }

        inline std::pair<std::vector<u8>, mab::candleTypes::Error_t> transferCanOpenFrame(
            i16 Id, std::vector<u8> frameToSend, size_t responseSize) const
        {
            if (m_candle == nullptr)
            {
                m_log.error("Candle empty!");
                return {{}, candleTypes::Error_t::DEVICE_NOT_CONNECTED};
            }
            auto result = getCandle()->transferCANFrame(
                Id, frameToSend, responseSize, m_timeout.value_or(DEFAULT_CAN_TIMEOUT));

            if (result.second != candleTypes::Error_t::OK)
            {
                m_log.error("Error while transfering CAN frame!");
            }
            return result;
        }

        inline std::pair<std::vector<u8>, mab::candleTypes::Error_t>
        transferCanOpenFrameNoRespondExpected(i16             Id,
                                              std::vector<u8> frameToSend,
                                              size_t          responseSize,
                                              u32             timeout = 0) const
        {
            if (m_candle == nullptr)
            {
                m_log.error("Candle empty!");
                return {{}, candleTypes::Error_t::DEVICE_NOT_CONNECTED};
            }

            std::optional<Logger::Verbosity_E> levelBeforePDO = m_log.g_m_verbosity;

            m_log.g_m_verbosity = Logger::Verbosity_E::SILENT;

            if (timeout == 0)
            {
                auto result = getCandle()->transferCANFrame(
                    Id, frameToSend, responseSize, 10 * m_timeout.value_or(DEFAULT_CAN_TIMEOUT));
                result.second       = candleTypes::Error_t::OK;
                m_log.g_m_verbosity = levelBeforePDO;

                return result;
            }
            else
            {
                auto result = getCandle()->transferCANFrame(Id, frameToSend, responseSize, timeout);
                result.second       = candleTypes::Error_t::OK;
                m_log.g_m_verbosity = levelBeforePDO;

                return result;
            }
        }
    };

}  // namespace mab
