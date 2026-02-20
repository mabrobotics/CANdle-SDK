#pragma once

#include <unistd.h>
#include "edsEntry.hpp"
#include "mab_types.hpp"
#include "logger.hpp"
#include "manufacturer_data.hpp"
#include "candle_types.hpp"
#include "candle.hpp"

#include <cstring>

#include <array>
#include <memory>
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

        /// @brief check communication with the board
        Error_t init();

        /// @brief use SDO message in order to send all value needed to configure the motor for
        /// moving
        /// @param param struct containing all the parameters needed to configure the motor
        /// @return error_t indicating the result of the operation
        // Error_t setProfileParameters(moveParameter& param);

        /// @brief Enable the driver with the specified mode of operation
        /// @param mode Mode of operation to set cf 0x6060 in the CANopen object dictionary
        /// @return Error_t indicating the result of the operation
        Error_t enable();

        /// @brief Disable the driver
        /// @return Error_t indicating the result of the operation
        Error_t disable();

        /// @brief Blink the built-in LEDs with CANopen command
        Error_t blink();

        /// @brief Save configuration data to the memory
        /// @return Error_t indicating the result of the operation
        Error_t save();

        /// @brief Zero out the position of the encoder
        /// @return Error_t indicating the result of the operation
        Error_t zero();

        /// @brief read a value from a can open register using SDO can frame
        /// @param index index from the object dictionary where the user want to read the value
        /// @param subindex subindex from the object dictionary where the user want to read the
        /// value
        /// @param force if true, skip the check if the object is readable
        /// @return Error on failure
        Error_t readSDO(EDSEntry& edsEntry) const;

        /// @brief write a value in a can open register using SDO segmented can frame
        /// @param index index from the object dictionary where the user want to write the value
        /// @param subindex subindex from the object dictionary where the user want to write the
        /// value
        /// @param dataString string containing the data to write
        /// @param force if true, skip the check if the object is writable
        /// @return Error on failure
        Error_t writeSDO(EDSEntry& edsEntry) const;

        /// @brief write a value in a can open register using PDO can frame
        /// @param index id of pdo to send (200/300/400/500 + node_id)
        /// @param data value to write
        /// @return Error on failure
        Error_t writePDO(EDSEntry& edsEntry) const;

        /// @brief try to communicate with canOpen frame (SDO) with all the possible id
        /// @param candle
        /// @return a vector with all id with a MD attach in CANopen communication
        static std::vector<canId_t> discoverOpenMDs(Candle*                              candle,
                                                    std::shared_ptr<EDSObjectDictionary> od);

      private:
        Candle* m_candle;

        /// @brief Generate the Object Dictionary from the EDS file
        /// @return A vector of edsObject representing the Object Dictionary
        std::shared_ptr<EDSObjectDictionary> m_od;

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
            usleep(1'000);

            err            = readSDO((*m_od)[0x6041]);
            u16 statusWord = (u16)(open_types::UNSIGNED16_t)(*m_od)[0x6041];
            m_log.debug("Statusword: 0x%x", statusWord);
            if (err != Error_t::OK || statusWord != 0x27)
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
            err = readSDO((*m_od)[0x6061]);
            if ((i8)(open_types::INTEGER8_t)(*m_od)[0x6061] != -1)
            {
                m_log.error("Coudl not enter service mode");
                m_log.error("Current mode: %i", (i8)(open_types::INTEGER8_t)((*m_od)[0x6061]));
            }

            err = readSDO((*m_od)[0x6060]);
            if ((i8)(open_types::INTEGER8_t)(*m_od)[0x6060] != -2)
            {
                m_log.error("Coudl not enter service mode");
                m_log.error("Current mode: %i", (i8)(open_types::INTEGER8_t)((*m_od)[0x6061]));
            }
            return err;
        }

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
                auto result = m_candle->transferCANFrame(
                    Id, frameToSend, responseSize, 10 * m_timeout.value_or(DEFAULT_CAN_TIMEOUT));
                result.second       = candleTypes::Error_t::OK;
                m_log.g_m_verbosity = levelBeforePDO;

                return result;
            }
            else
            {
                auto result   = m_candle->transferCANFrame(Id, frameToSend, responseSize, timeout);
                result.second = candleTypes::Error_t::OK;
                m_log.g_m_verbosity = levelBeforePDO;

                return result;
            }
        }
    };

}  // namespace mab
