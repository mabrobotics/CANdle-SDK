#pragma once

#include "mab_types.hpp"
#include "md_types.hpp"
#include "logger.hpp"
#include "manufacturer_data.hpp"
#include "candle_types.hpp"
#include "MDStatus.hpp"
#include "candle.hpp"
#include "../../../candletool/objectDictionary/OD.hpp"

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

namespace mab
{
    /// @brief Software representation of MD device on the can network
    class MDCO
    {
        static constexpr size_t DEFAULT_RESPONSE_SIZE = 23;

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
        /// @param transferCANFrame
        MDCO(canId_t canId, Candle* candle) : m_canId(canId), m_candle(candle)
        {
            m_log.m_layer = Logger::ProgramLayer_E::TOP;
            std::stringstream tag;
            tag << "MD" << std::setfill('0') << std::setw(4) << m_canId;
            m_log.m_tag = tag.str();
        }

        /// @brief Move to desired position within the acceleration,deceleration constraint
        /// @param targetPos desired position
        /// @param accLimit acceleration limit [RPM/s]
        /// @param dccLimit decelaration limit [RPM/s]
        /// @param MaxSpeed maximum motor speed [RPM]
        /// @param MaxCurrent Maximum current accepted by the motor [mA*RatedCurrent]
        /// @param RatedCurrent Rated Current [mA]
        /// @param MaxTorque Maximum torque accepted by the motor [mNM*RatedCurrent]
        /// @param RatedTorque Rated torque [mNM]
        void movePositionAcc(i32 targetPos,
                             i32 accLimit,
                             i32 dccLimit,
                             u32 MaxSpeed,
                             u16 MaxCurrent,
                             u32 RatedCurrent,
                             u16 MaxTorque,
                             u32 RatedTorque);

        /// @brief Move to desired position
        /// @param targetPos desired position
        /// @param MaxSpeed maximum motor speed [RPM]
        /// @param MaxCurrent Maximum current accepted by the motor [mA*RatedCurrent]
        /// @param RatedCurrent Rated Current [mA]
        /// @param MaxTorque Maximum torque accepted by the motor [mNM*RatedCurrent]
        /// @param RatedTorque Rated torque [mNM]
        void movePosition(u16 MaxCurrent,
                          u32 RatedCurrent,
                          u16 MaxTorque,
                          u32 RatedTorque,
                          u32 MaxSpeed,
                          i32 DesiredPos);

        /// @brief Move to desired speed
        /// @param DesiredSpeed desired speed [RPM]
        /// @param MaxSpeed maximum motor speed [RPM] (must be superior than DesiredSpeed)
        /// @param MaxCurrent Maximum current accepted by the motor [mA*RatedCurrent]
        /// @param RatedCurrent Rated Current [mA]
        /// @param MaxTorque Maximum torque accepted by the motor [mNM*RatedCurrent]
        /// @param RatedTorque Rated torque [mNM]
        void moveSpeed(u16 MaxCurrent,
                       u32 RatedCurrent,
                       u16 MaxTorque,
                       u32 RatedTorque,
                       u32 MaxSpeed,
                       i32 DesiredSpeed);

        /// @brief Move to desired position with impedance control
        /// @param desiredSpeed desired speed [RPM]
        /// @param targetPos desired position
        /// @param kp proportional gain
        /// @param kd derivative gain
        /// @param torque torque to apply [mNM]
        /// @param MaxSpeed maximum motor speed [RPM]
        /// @param MaxCurrent Maximum current accepted by the motor [mA*RatedCurrent]
        /// @param RatedCurrent Rated Current [mA]
        /// @param MaxTorque Maximum torque accepted by the motor [mNM*RatedCurrent]
        /// @param RatedTorque Rated torque [mNM]
        /// @return Error_t indicating the result of the operation
        /// @details This function sets the motor in impedance control mode and moves it to the
        /// specified position with the given speed, gains, and torque.
        Error_t moveImpedance(i32 desiredSpeed,
                              i32 targetPos,
                              f32 kp,
                              f32 kd,
                              i16 torque,
                              u32 MaxSpeed,
                              u16 MaxCurrent,
                              u32 RatedCurrent,
                              u16 MaxTorque,
                              u32 RatedTorque);

        /// @brief Check communication with MD device
        /// @return Error if not connected
        Error_t init();

        /// @brief Blink the built-in LEDs with CANopen command
        Error_t blinkOpenTest();

        /// @brief Reset the driver with CANopen command
        /// @return
        Error_t OpenReset();

        /// @brief Clear errors present in the driver
        /// @param level 1 => clear error, 2 => clear warning, 3 => clear both
        /// @return
        Error_t clearOpenErrors(int level);

        /// @brief Change CANopen config the command need to be save before shutoff the motors
        /// @param newID new id of the motor
        /// @param newBaud new baudRate
        /// @param newwatchdog new watchdog
        Error_t newCanOpenConfig(long newID, long newBaud, int newwatchdog);

        /// @brief Perform a encoder test
        /// @param Main Main=1 if you want to perform the test on the main encoder
        /// @param output output=1 if you want to perform the test on the output encoder
        Error_t testencoder(bool Main, bool output);

        /// @brief Perform a encoder calibration
        /// @param Main Main=1 if you want to perform the calibration on the main encoder
        /// @param output output=1 if you want to perform the calibration on the output encoder
        Error_t encoderCalibration(bool Main, bool output);

        /// @brief change the torque bandwidth with CANopen comand, modification must be saved
        /// before shuting off the motors
        /// @param newBandwidth new value for the bandwidth {50-2500}[Hz]
        /// @return
        Error_t CanOpenBandwidth(int newBandwidth);

        /// @brief Save configuration data to the memory
        /// @return
        Error_t openSave();

        /// @brief Zero out the position of the encoder
        /// @return
        Error_t openZero();

        /// @brief read a value from a can open register using SDO can frame
        /// @param index index from the object dictionary where the user want to read the value
        /// @param subindex subindex from the object dictionary where the user want to read the
        /// value
        /// @return Error on failure
        inline Error_t ReadOpenRegisters(int index, short subindex, bool force = false)
        {
            if (!force)
            {
                if (isReadable(index, subindex) != OK)
                {
                    m_log.error("Object 0x%04x:0x%02x is not Readable!", index, subindex);
                    return Error_t::REQUEST_INVALID;
                }
            }

            m_log.debug("Read Open register...");
            std::vector<u8> frame;
            frame.push_back(0x40);                // Command: initiate upload
            frame.push_back(((u8)index));         // Index LSB
            frame.push_back(((u8)(index >> 8)));  // Index MSB
            frame.push_back(subindex);            // Subindex
            frame.push_back(0x00);                // Padding
            frame.push_back(0x00);
            frame.push_back(0x00);
            frame.push_back(0x00);

            // std::cout << "size de frame: " << frame.size() << std::endl;
            //  message sending via transferCanFrame
            auto [response, error] = transferCanOpenFrame(0x600 + m_canId, frame, frame.size());

            // data display

            std::cout << " ---- Received CAN Frame Info ----" << endl;
            uint8_t cmd = response[0];

            if ((cmd & 0xF0) != 0x40)
            {
                std::cout << "Frame not recognized as an SDO Upload Expedited response."
                          << std::endl;
            }
            else
            {
                // cout << (int)cmd << endl;
                // FLAGS Extraction
                // bool    expedited     = cmd & 0x02;
                // bool    sizeIndicated = cmd & 0x01;
                uint8_t n       = (cmd & 0x0C) >> 2;  // bits 1-0
                uint8_t dataLen = 4 - n;
                // cout << (int)n << endl;

                // Index et Subindex
                uint16_t index    = response[2] << 8 | response[1];
                uint8_t  subindex = response[3];

                // data display
                std::cout << "Index      : 0x" << std::hex << std::setw(4) << std::setfill('0')
                          << index << std::endl;

                std::cout << "Subindex   : 0x" << std::hex << std::setw(2) << std::setfill('0')
                          << (int)subindex << std::endl;

                std::cout << "Data (" << std::dec << (int)dataLen << " byte(s)): 0x";

                for (int i = dataLen - 1; i >= 0; --i)
                {
                    std::cout << std::hex << std::setw(2) << std::setfill('0')
                              << (int)response[4 + i];
                }
                cout << endl << "------------------------" << endl;
            }

            std::cout << std::endl;
            if (error == mab::candleTypes::Error_t::OK)
            {
                return Error_t::OK;
            }
            else
            {
                m_log.error("Error in the register write response!");
                return Error_t::TRANSFER_FAILED;
            }
        }

        /// @brief write a value in a can open register using SDO segmented can frame
        /// @param name name of the object to write
        /// @param data value to write
        /// @param size size of the data to write (1,2,4)
        /// @return Error on failure
        Error_t WriteLongOpenRegisters(int                index,
                                       short              subindex,
                                       const std::string& dataString,
                                       bool               force = false)
        {
            if (!force)
            {
                if (isWritable(index, subindex) != OK)
                {
                    m_log.error("Object 0x%04x:0x%02x is not writable!", index, subindex);
                    return Error_t::REQUEST_INVALID;
                }
            }
            std::string motorName = dataString;

            m_log.debug("Writing Motor Name to 0x2000:0x06 via segmented SDO...");

            // 1. prepare data to send clip data to 20 bytes (Motor Name)
            // std::vector<u8> data(20, 0x00);
            // std::memcpy(data.data(), motorName.data(), std::min<size_t>(motorName.size(), 20));
            std::vector<u8> data = std::vector<u8>(motorName.begin(), motorName.end());
            // 2. sending init message of segmented transfer
            std::vector<u8> initFrame = {0x21,  // CCS=1, E=1, S=1
                                         u8(index & 0xFF),
                                         u8(index >> 8),
                                         u8(subindex),
                                         u8(data.size() >> 0),
                                         u8(data.size() >> 8),
                                         u8(data.size() >> 16),
                                         u8(data.size() >> 24)};

            auto [initResponse, initError] =
                transferCanOpenFrame(0x600 + m_canId, initFrame, initFrame.size());

            if (initError != mab::candleTypes::Error_t::OK)
            {
                m_log.error("Failed to initiate segmented SDO download.");
                return Error_t::TRANSFER_FAILED;
            }

            constexpr size_t OFF = 0;  // Offset CAN response header
            if (initResponse.size() < OFF + 1 || (initResponse[OFF] & 0xE0) != 0x60)
            {
                m_log.error("Unexpected response to initiate download (expected 0x60).");
                return Error_t::TRANSFER_FAILED;
            }

            // 3. sending data segments
            size_t offset = 0;
            u8     toggle = 0;

            while (offset < data.size())
            {
                std::vector<u8> segmentFrame;

                size_t remaining     = data.size() - offset;
                size_t segmentLength = std::min<size_t>(7, remaining);
                u8     emptyBytes    = 7 - segmentLength;
                bool   lastSegment   = (segmentLength == remaining);

                u8 cmdByte = 0x00;
                cmdByte |= (toggle & 0x01) << 4;         // bit 4: toggle
                cmdByte |= (emptyBytes & 0x07) << 1;     // bits 3:1: empty bytes
                cmdByte |= (lastSegment ? 0x01 : 0x00);  // bit 0: C (last segment)

                segmentFrame.push_back(cmdByte);

                // segments data
                for (size_t i = 0; i < segmentLength; ++i)
                {
                    segmentFrame.push_back(data[offset + i]);
                }

                // padding with zeros if needed
                for (size_t i = segmentLength; i < 7; ++i)
                {
                    segmentFrame.push_back(0x00);
                }

                auto [segResponse, segError] =
                    transferCanOpenFrame(0x600 + m_canId, segmentFrame, segmentFrame.size());

                if (segError != mab::candleTypes::Error_t::OK)
                {
                    m_log.error("Segmented transfer failed at offset {}", offset);
                    return Error_t::TRANSFER_FAILED;
                }

                // Check server response: must be 0x20 | toggle
                if (segResponse.size() < OFF + 1 || (segResponse[OFF] & 0xE0) != 0x20)
                {
                    m_log.error("Malformed segment ACK.");
                    return Error_t::TRANSFER_FAILED;
                }

                if ((segResponse[OFF] & 0x10) != (toggle << 4))
                {
                    m_log.error("Unexpected toggle bit, corrupted transfer.");
                    return Error_t::TRANSFER_FAILED;
                }

                offset += segmentLength;
                toggle ^= 0x01;
            }

            m_log.debug("Motor Name successfully written.");
            return Error_t::OK;
        }

        /// @brief read a value from a can open register using SDO segmented transfer can frame (if
        /// data > 4 bit)
        /// @param index index from the object dictionary where the user want to read the value
        /// @param subindex subindex from the object dictionary where the user want to read the
        /// value
        /// @return Error on failure
        inline Error_t ReadLongOpenRegisters(int index, short subindex, std::vector<u8>& outData)
        {
            // // only for testing
            // // WriteMotorName();
            if (isReadable(index, subindex) != OK)
            {
                m_log.error("Object 0x%04x:0x%02x is not readable!", index, subindex);
                return Error_t::REQUEST_INVALID;
            }

            constexpr size_t OFF = 2;  // can message offset
            m_log.debug("Read Object (0x%lx:0x%x) via segmented SDO…", index, subindex);

            // ---------- 1) Initiation Request ----------
            std::vector<u8> initReq = {0x40,  // CCS=2: Initiate Upload
                                       u8(index & 0xFF),
                                       u8(index >> 8),
                                       u8(subindex),
                                       0x00,
                                       0x00,
                                       0x00,
                                       0x00};

            auto [rspInit, errInit] =
                transferCanOpenFrame(0x600 + m_canId, initReq, initReq.size());
            if (errInit != mab::candleTypes::Error_t::OK || rspInit.size() < OFF + 8)
            {
                m_log.error("Failed to initiate SDO read.");
                return Error_t::TRANSFER_FAILED;
            }

            u8   cmd         = rspInit[OFF];
            bool isExpedited = cmd & 0x02;
            bool hasSize     = cmd & 0x01;

            if (isExpedited)
            {
                m_log.warn("Data received in expedited mode, probably ≤ 4 bytes.");
                u8 n   = ((cmd >> 2) & 0x03);  // number of used bytes
                u8 len = 4 - n;

                outData.insert(
                    outData.end(), rspInit.begin() + OFF + 4, rspInit.begin() + OFF + 4 + len);
            }
            else
            {
                // ---------- 2) Segmented reading ----------
                u32 totalLen = 0;
                if (hasSize)
                {
                    totalLen = rspInit[OFF + 4] | (rspInit[OFF + 5] << 8) |
                               (rspInit[OFF + 6] << 16) | (rspInit[OFF + 7] << 24);
                    outData.reserve(totalLen);
                }

                bool toggle   = false;
                bool finished = false;

                while (!finished)
                {
                    std::vector<u8> segReq = {
                        u8(0x60 | (toggle ? 0x10 : 0x00)), 0, 0, 0, 0, 0, 0, 0};
                    auto [rspSeg, errSeg] =
                        transferCanOpenFrame(0x600 + m_canId, segReq, segReq.size());

                    if (errSeg != mab::candleTypes::Error_t::OK || rspSeg.size() < OFF + 1)
                    {
                        m_log.error("Error segment reading");
                        return Error_t::TRANSFER_FAILED;
                    }

                    u8 segCmd = rspSeg[OFF];
                    if ((segCmd & 0x10) != (toggle ? 0x10 : 0x00))
                    {
                        m_log.error("Bit toggle didn't expected, corrupt transfer.");
                        return Error_t::TRANSFER_FAILED;
                    }

                    bool last    = segCmd & 0x01;
                    u8   unused  = (segCmd >> 1) & 0x07;
                    u8   dataLen = 7 - unused;

                    if (rspSeg.size() < OFF + 1 + dataLen)
                    {
                        m_log.error("Incomplete data in the segment.");
                        return Error_t::TRANSFER_FAILED;
                    }

                    outData.insert(outData.end(),
                                   rspSeg.begin() + OFF + 1,
                                   rspSeg.begin() + OFF + 1 + dataLen);
                    finished = last;
                    toggle   = !toggle;
                }

                if (hasSize && outData.size() != totalLen)
                {
                    m_log.warn(
                        "Size of data read (%d) ≠ size announced (%d)", outData.size(), totalLen);
                }
            }

            // ---------- 3) Display ----------
            if (DataSizeOfEdsObject(index, subindex) == 0)
            {
                // if data size is 0, we assume it is a string
                std::string motorName(outData.begin(), outData.end());
                m_log.info("Data received (convert into string): '%s'", motorName.c_str());
            }
            else
            {
                m_log.info("Data received: %s",
                           std::string(outData.begin(), outData.end()).c_str());
            }

            return Error_t::OK;
        }

        /// @brief return a value from a can open register using SDO can frame
        /// @param index index from the object dictionary where the user want to read the value
        /// @param subindex subindex from the object dictionary where the user want to read the
        /// value
        /// @return the value contained in the register or -1 if error
        long GetValueFromOpenRegister(int index, short subindex)
        {
            if (isReadable(index, subindex) != OK)
            {
                m_log.error("Object 0x%04x:0x%02x is not writable!", index, subindex);
                return Error_t::REQUEST_INVALID;
            }
            // printf("Writing Open register...\n");
            m_log.debug("Read Open register...");
            // cout << index << endl;

            std::vector<u8> frame;
            frame.push_back(0x40);                // Command: initiate upload
            frame.push_back(((u8)index));         // Index LSB
            frame.push_back(((u8)(index >> 8)));  // Index MSB
            frame.push_back(subindex);            // Subindex
            frame.push_back(0x00);                // Padding
            frame.push_back(0x00);
            frame.push_back(0x00);
            frame.push_back(0x00);

            auto [response, error] = transferCanOpenFrame(0x600 + m_canId, frame, frame.size());

            long answerValue = 0;
            for (int i = 0; i <= 4; i++)
            {
                answerValue += (((long)response[4 + i]) << (8 * i));
                // cout << "answerValue:" << answerValue << "au rang" << i << endl;
            }

            // cout << "answerValue:" << answerValue << endl;

            // data display
            uint8_t cmd = response[0];

            if ((cmd & 0xF0) != 0x40)
            {
                std::cout << "Frame not recognized as an SDO Upload Expedited response." << endl;
                return -1;
            }
            if (error == mab::candleTypes::Error_t::OK)
            {
                return answerValue;
            }
            else
            {
                m_log.error("Error in the register write response!");
                return -1;
            }
        }

        /// @brief write a value in a can open register using SDO can frame
        /// @param index index from the object dictionary where the user want to write the value
        /// @param subindex subindex from the object dictionary where the user want to write the
        /// value
        /// @return Error on failure
        inline Error_t WriteOpenRegisters(
            int index, short subindex, long data, short size = 0, bool force = false)
        {
            if (!force)
            {
                if (isWritable(index, subindex) != OK)
                {
                    m_log.error("Object 0x%04x:0x%02x is not writable!", index, subindex);
                    return Error_t::REQUEST_INVALID;
                }
            }

            if (size == 0)
            {
                size = DataSizeOfEdsObject(index, subindex);
                if (size == -1)
                {
                    m_log.error("Object 0x%04x:0x%02x has an unsupported size (%d)!",
                                index,
                                subindex,
                                size);
                    return Error_t::REQUEST_INVALID;
                }
                else if (size == 0 || size > 4)
                {
                    m_log.error(
                        "Object 0x%04x:0x%02x has an unsupported size (%d), please use an "
                        "Segmented transfer !",
                        index,
                        subindex,
                        size);
                    return Error_t::REQUEST_INVALID;
                }
            }

            std::vector<u8> frame;
            if (size == 1)
                frame.push_back(0x2F);
            if (size == 2)
                frame.push_back(0x2B);
            if (size == 4)
                frame.push_back(0x23);
            frame.push_back(((u8)index));         // Index LSB
            frame.push_back(((u8)(index >> 8)));  // Index MSB
            frame.push_back(subindex);            // Subindex
            frame.push_back((u8)data);            // data
            frame.push_back((u8)(data >> 8));
            frame.push_back((u8)(data >> 16));
            frame.push_back((u8)(data >> 24));

            auto [response, error] = transferCanOpenFrame(0x600 + m_canId, frame, frame.size());

            if (error == mab::candleTypes::Error_t::OK)
            {
                return Error_t::OK;
            }
            else
            {
                m_log.error("Error in the register write response!");
                return Error_t::TRANSFER_FAILED;
            }
        }

        /// @brief write a value in a can open register using SDO can frame
        /// @param name name of the object to write
        /// @param data value to write
        /// @param size size of the data to write (1,2,4)
        /// @return Error on failure
        inline Error_t WriteOpenRegisters(const std::string& name,
                                          u32                data,
                                          u8                 size  = 0,
                                          bool               force = false)
        {
            bool debug    = m_log.isLevelEnabled(Logger::LogLevel_E::DEBUG);
            u32  index    = 0;
            u8   subIndex = 0;
            if (findObjectByName(name, index, subIndex) != OK)
            {
                m_log.error("%s not found in EDS file", name.c_str());
                return Error_t::UNKNOWN_OBJECT;
            }
            if (!force)
            {
                if (isWritable(index, subIndex) != OK)
                {
                    m_log.error("Object 0x%04x:0x%02x is not writable!", index, subIndex);
                    return Error_t::REQUEST_INVALID;
                }
            }
            auto err = WriteOpenRegisters(index, subIndex, data, size);
            if (debug)
                m_log.debug("Error:%d\n", ReadOpenRegisters(index, subIndex));
            return err;
        }

        /// @brief write a value in a can open register using PDO can frame
        /// @param index id of pdo to send (200/300/400/500 + node_id)
        /// @param subindex subindex from the object dictionary where the user want to write the
        /// value
        /// @return Error on failure
        inline Error_t WriteOpenPDORegisters(int index, std::vector<u8> data)
        {
            m_log.debug("Writing Open Pdo register...");

            auto [response, error] =
                transferCanOpenFrameNoRespondExpected(index, data, data.size());

            if (error == mab::candleTypes::Error_t::OK)
            {
                return Error_t::OK;
            }
            else
            {
                m_log.error("Error in the register write response!");
                return Error_t::TRANSFER_FAILED;
            }
        }

        /// @brief write a value in a can open register using PDO can frame
        /// @param index id of pdo to send (200/300/400/500 + node_id)
        /// @param subindex subindex from the object dictionary where the user want to write the
        /// value
        /// @return Error on failure
        inline Error_t SendCustomData(int index, std::vector<u8> data)
        {
            m_log.debug("Writing Custom data...");
            transferCanOpenFrameNoRespondExpected(index, data, data.size());
            return Error_t::OK;
        }

        /// @brief averages the latency of 1000 commands
        void testLatency();

        /// @brief try to communicate with canOpen frame (SDO) with all the possible id
        /// @param candle
        /// @return a vector with all id with a MD attach in CANopen communication
        static std::vector<canId_t> discoverOpenMDs(Candle* candle);

        /// @brief Return the size of the data of an EDS object
        /// @param index Index of the object in the Object Dictionary
        /// @param subIndex Subindex of the object in the Object Dictionary
        /// @return Size of the data in bytes, or 0 if the object is a string or -1 if the object is
        /// not found
        u8 DataSizeOfEdsObject(const u32 index, const u8 subIndex);

        /// @brief Display all information about the MD device
        /// @details This function prints the all the actual register value, device type, and all
        /// objects in the Object Dictionary.
        void printAllInfo();

      private:
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

        inline std::pair<std::vector<u8>, mab::candleTypes::Error_t> transferCanOpenFrame(
            int Id, std::vector<u8> frameToSend, size_t responseSize) const
        {
            if (m_candle == nullptr)
            {
                m_log.error("Candle empty!");
                return {{}, candleTypes::Error_t::DEVICE_NOT_CONNECTED};
            }
            auto result = getCandle()->transferCANFrame(
                Id, frameToSend, responseSize, 10 * m_timeout.value_or(DEFAULT_CAN_TIMEOUT));

            if (result.second != candleTypes::Error_t::OK)
            {
                m_log.error("Error while transfering CAN frame!");
            }
            return result;
        }

        inline std::pair<std::vector<u8>, mab::candleTypes::Error_t>
        transferCanOpenFrameNoRespondExpected(int             Id,
                                              std::vector<u8> frameToSend,
                                              size_t          responseSize) const
        {
            if (m_candle == nullptr)
            {
                m_log.error("Candle empty!");
                return {{}, candleTypes::Error_t::DEVICE_NOT_CONNECTED};
            }
            auto result = getCandle()->transferCANPDOFrame(
                Id, frameToSend, responseSize, 10 * m_timeout.value_or(DEFAULT_CAN_TIMEOUT));
            result.second = candleTypes::Error_t::OK;

            return result;
        }
    };

}  // namespace mab
