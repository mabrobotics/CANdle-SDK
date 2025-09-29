#include "MDCO.hpp"

namespace mab
{

    // Convert any trivially copyable type to u32
    template <typename T>
    u32 MDCO::toU32(T value)
    {
        static_assert(std::is_trivially_copyable_v<T>, "T must be trivially copyable");
        if constexpr (sizeof(T) <= sizeof(u32))
        {
            // Zero-initialize u32
            u32 result = 0;
            // Copy bytes of value into result
            std::array<std::byte, sizeof(T)> bytes =
                std::bit_cast<std::array<std::byte, sizeof(T)>>(value);
            std::memcpy(&result, bytes.data(), sizeof(T));
            return result;
        }
        else
        {
            m_log.error("Value too large to fit in u32");
            return 0;
        }
    }

    MDCO::Error_t MDCO::findObjectByName(const std::string& searchTerm, u32& index, u8& subIndex)
    {
        bool objectFound = false;
        // for loop over all objects contained in the object dictionary
        for (const edsObject& obj : this->ObjectDictionary)
        {
            if (obj.ParameterName.size() == searchTerm.size())
            {
                bool equal = true;
                // compare each character of the two strings, case insensitive
                for (size_t i = 0; i < obj.ParameterName.size(); ++i)
                {
                    if (tolower(obj.ParameterName[i]) != tolower(searchTerm[i]))
                    {
                        equal = false;
                        break;
                    }
                }
                // if they are equal, return the index and subindex
                if (equal)
                {
                    index    = obj.index;
                    subIndex = obj.subIndex;
                    // if the object was already found once, we warn the user that multiple objects
                    // have the same name
                    if (objectFound)
                    {
                        m_log.warn(
                            "Multiple objects found with name '%s'. Using the first one found.\n",
                            searchTerm.c_str());
                    }
                    else
                    {
                        m_log.debug("Object found: %s (0x%04X, subIndex: %d)\n",
                                    obj.ParameterName.c_str(),
                                    obj.index,
                                    obj.subIndex);
                        objectFound = true;
                    }
                }
            }
        }
        // if the object was found at least once, return OK else UNKNOWN_OBJECT
        if (objectFound)
        {
            return OK;
        }

        return UNKNOWN_OBJECT;
    }

    MDCO::Error_t MDCO::isWritable(const u32 index, const u8 subIndex)
    {
        // for loop over all objects contained in the object dictionary
        for (const edsObject& obj : this->ObjectDictionary)
        {
            // if the index and subindex match
            if (index == obj.index && subIndex == obj.subIndex)
            {
                // check if the access type contains 'w' or 'W'
                if (obj.accessType.find('w') != std::string::npos ||
                    obj.accessType.find('W') != std::string::npos)
                {
                    return MDCO::Error_t::OK;
                }
                else
                {
                    return MDCO::Error_t::REQUEST_INVALID;
                }
            }
        }

        return MDCO::Error_t::UNKNOWN_OBJECT;
    }

    MDCO::Error_t MDCO::isReadable(const u32 index, const u8 subIndex)
    {
        // for loop over all objects contained in the object dictionary
        for (const edsObject& obj : this->ObjectDictionary)
        {
            // if the index and subindex match
            if (index == obj.index && subIndex == obj.subIndex)
            {
                // check if the access type contains 'r' or 'R'
                if (obj.accessType.find('r') != std::string::npos ||
                    obj.accessType.find('R') != std::string::npos)
                {
                    return MDCO::Error_t::OK;
                }
                else
                {
                    return MDCO::Error_t::REQUEST_INVALID;
                }
            }
        }

        return MDCO::Error_t::UNKNOWN_OBJECT;
    }

    i8 MDCO::dataSizeOfEdsObject(const u32 index, const u8 subIndex)
    {
        // for loop over all objects contained in the object dictionary
        for (const edsObject& obj : this->ObjectDictionary)
        {
            // if the index and subindex match
            if (index == obj.index && subIndex == obj.subIndex)
            {
                // if dataype is boolean, u8 or i8
                if (obj.DataType == 0x0001 || obj.DataType == 0x0002 || obj.DataType == 0x0005)
                {
                    return 1;
                }
                // if dataype is u16 or i16
                else if (obj.DataType == 0x0003 || obj.DataType == 0x0006)
                {
                    return 2;
                }
                // if dataype is u32, i32 or real32
                else if (obj.DataType == 0x0004 || obj.DataType == 0x0007 || obj.DataType == 0x0008)
                {
                    return 4;
                }
                // if dataype is u64, i64 or real64
                else if (obj.DataType == 0x0011 || obj.DataType == 0x0015 || obj.DataType == 0x001B)
                {
                    return 8;
                }
                else if (obj.DataType == 0x0009 || obj.DataType == 0x000A ||
                         obj.DataType == 0x000B || obj.DataType == 0x000F)
                {
                    return 0;  // String => size is variable and unknown
                }
            }
        }

        return -1;  // Invalid index or subindex
    }

    void MDCO::printAllInfo()
    {
        i32 value;
        // loop over all objects in the object dictionary and print their info
        for (i16 i = 0; i < (i16)ObjectDictionary.size(); i++)
        {
            value =
                getValueFromOpenRegister(ObjectDictionary[i].index, ObjectDictionary[i].subIndex);
            if (value != -1)
                m_log.info(
                    "----------Object Name:%s----------\nindex:%X, sub-index:%X, Storage "
                    "Location:%s, "
                    "Data length(bytes):%d, Access:%s, PDO Mapping:%d, actual value(Raw data "
                    "received):%lld\n"
                    "-----------------------------------\n\n",
                    ObjectDictionary[i].ParameterName.c_str(),
                    ObjectDictionary[i].index,
                    ObjectDictionary[i].subIndex,
                    ObjectDictionary[i].StorageLocation.c_str(),
                    dataSizeOfEdsObject(ObjectDictionary[i].index, ObjectDictionary[i].subIndex),
                    ObjectDictionary[i].accessType.c_str(),
                    ObjectDictionary[i].PDOMapping,
                    value);
        }
    }

    MDCO::Error_t MDCO::setProfileParameters(moveParameter& param)
    {
        // set all the parameters needed to configure the motor for moving log an error message if
        // transfer failed
        Error_t err;
        err = writeOpenRegisters("Motor Max Acceleration", param.accLimit);
        if (err != OK)
        {
            m_log.error("Error setting Max Acceleration");
            return err;
        }
        err = writeOpenRegisters("Motor Max Deceleration", param.dccLimit);
        if (err != OK)
        {
            m_log.error("Error setting Max Deceleration");
            return err;
        }
        err = writeOpenRegisters("Max Current", param.MaxCurrent);
        if (err != OK)
        {
            m_log.error("Error setting Max Current");
            return err;
        }
        err = writeOpenRegisters("Motor Rated Current", param.RatedCurrent);
        if (err != OK)
        {
            m_log.error("Error setting Rated Current");
            return err;
        }
        err = writeOpenRegisters("Max Motor Speed", param.MaxSpeed);
        if (err != OK)
        {
            m_log.error("Error setting Max Motor Speed");
            return err;
        }
        err = writeOpenRegisters("Motor Max Torque", param.MaxTorque);
        if (err != OK)
        {
            m_log.error("Error setting Max Torque");
            return err;
        }
        err = writeOpenRegisters("Motor Rated Torque", param.RatedTorque);
        if (err != OK)
        {
            m_log.error("Error setting Rated Torque");
            return err;
        }
        return OK;
    }

    MDCO::Error_t MDCO::enableDriver(ModesOfOperation mode)
    {
        // set the mode of operation and enable the driver, log an error message if transfer failed
        Error_t err;
        err = writeOpenRegisters("Modes Of Operation", mode, 1);
        if (err != OK)
        {
            m_log.error("Error setting Mode of Operation");
            return err;
        }
        err = writeOpenRegisters("Controlword", 0x80, 2);
        if (err != OK)
        {
            m_log.error("Error setting Controlword to 0x80");
            return err;
        }
        err = writeOpenRegisters("Controlword", 0x06, 2);
        if (err != OK)
        {
            m_log.error("Error setting Controlword to 0x06");
            return err;
        }
        err = writeOpenRegisters("Controlword", 15, 2);
        if (err != OK)
        {
            m_log.error("Error setting Controlword to 15");
            return err;
        }
        return OK;
    }

    MDCO::Error_t MDCO::disableDriver()
    {
        // disable the driver, log an error message if transfer failed
        Error_t err;
        err = writeOpenRegisters("Motor Target Velocity", 0);
        if (err != OK)
        {
            m_log.error("Error setting Motor Target Velocity to 0");
            return err;
        }
        err = writeOpenRegisters("Controlword", 6);
        if (err != OK)
        {
            m_log.error("Error setting Controlword to 6");
            return err;
        }
        err = writeOpenRegisters("Modes Of Operation", 0);
        if (err != OK)
        {
            m_log.error("Error setting Modes Of Operation to 0");
            return err;
        }
        return OK;
    }

    void MDCO::movePosition(i32 DesiredPos, i16 timeoutMillis)
    {
        auto start      = std::chrono::steady_clock::now();
        auto lastSend   = start;
        auto timeout    = std::chrono::milliseconds(timeoutMillis);  // total movement timeout
        auto sendPeriod = std::chrono::milliseconds(10);             // send every 10ms

        // while timeout non reach or position not reached
        while (std::chrono::steady_clock::now() - start < timeout &&
               !((i16)getValueFromOpenRegister(0x6064, 0) > (DesiredPos - 100) &&
                 (i16)getValueFromOpenRegister(0x6064, 0) < (DesiredPos + 100)))
        {
            auto now = std::chrono::steady_clock::now();
            if (now - lastSend >= sendPeriod)  // send position message every sendPeriod time
            {
                Error_t err = writeOpenRegisters("Motor Target Position", DesiredPos, 4);
                if (err != OK)
                {
                    m_log.error("Error setting Motor Target Position");
                    return;
                }
                lastSend = now;
            }
        }
        m_log.debug("actual position: %d\n", (i16)getValueFromOpenRegister(0x6064, 0));
        // check if position reached with a tolerance of 200 [inc]
        if (((i16)getValueFromOpenRegister(0x6064, 0) > (DesiredPos - 200) &&
             (i16)getValueFromOpenRegister(0x6064, 0) < (DesiredPos + 200)))
        {
            m_log.success("Position reached in less than 5s");
        }
        else
        {
            m_log.error("Position not reached in less than 5s");
        }
    }

    void MDCO::moveSpeed(i32 DesiredSpeed, i16 timeoutMillis)
    {
        auto start      = std::chrono::steady_clock::now();
        auto lastSend   = start;
        auto timeout    = std::chrono::milliseconds(timeoutMillis);  // total movement timeout
        auto sendPeriod = std::chrono::milliseconds(10);             // send every 10ms

        // while timeout non reach
        while (std::chrono::steady_clock::now() - start < timeout)
        {
            auto now = std::chrono::steady_clock::now();
            // send speed message every sendPeriod time
            if (now - lastSend >= sendPeriod)
            {
                Error_t err = writeOpenRegisters("Motor Target Velocity", DesiredSpeed);
                if (err != OK)
                {
                    m_log.error("Error setting Motor Target Velocity");
                    return;
                }
                lastSend = now;
            }
        }
        // check if speed reached with a tolerance of 5 [RPM]
        if ((i16)getValueFromOpenRegister(0x606C, 0x00) <= DesiredSpeed + 5 &&
            (i16)getValueFromOpenRegister(0x606C, 0x00) >= DesiredSpeed - 5)
        {
            m_log.success("Velocity Target reached with +/- 5RPM");
        }
        else
        {
            m_log.error("Velocity Target not reached");
        }
    }

    MDCO::Error_t MDCO::moveImpedance(i32            desiredSpeed,
                                      i32            targetPos,
                                      moveParameter& param,
                                      i16            timeoutMillis)
    {
        Error_t err;
        // kp
        u32 kp_bits;
        memcpy(&kp_bits, &(param.kp), sizeof(float));
        err = writeOpenRegisters("Kp_impedance", kp_bits);
        if (err != OK)
        {
            m_log.error("Error setting Kp_impedance");
            return err;
        }
        // kd
        u32 kd_bits;
        memcpy(&kd_bits, &(param.kd), sizeof(float));
        err = writeOpenRegisters("Kd_impedance", kd_bits);
        if (err != OK)
        {
            m_log.error("Error setting Kd_impedance");
            return err;
        }
        err = writeOpenRegisters("Position Demand Value", targetPos);
        if (err != OK)
        {
            m_log.error("Error setting Position Demand Value");
            return err;
        }
        err = writeOpenRegisters("Velocity Demand Value", desiredSpeed);
        if (err != OK)
        {
            m_log.error("Error setting Velocity Demand Value");
            return err;
        }
        err = writeOpenRegisters("Torque Demand Value", param.torqueff);
        if (err != OK)
        {
            m_log.error("Error setting Torque Demand Value");
            return err;
        }
        auto start   = std::chrono::steady_clock::now();
        auto timeout = std::chrono::milliseconds((timeoutMillis));  // total movement timeout
        while (std::chrono::steady_clock::now() - start < timeout)
        {
        }
        err = writeOpenRegisters("Torque Demand Value", 0);
        if (err != OK)
        {
            m_log.error("Error resetting Torque Demand Value");
            return err;
        }
        return MDCO::Error_t::OK;
    }

    MDCO::Error_t MDCO::blinkOpenTest()
    {
        // blink the motor led, log an error message if transfer failed
        Error_t err;
        err = writeOpenRegisters("Controlword", 0x06, 2);
        if (err != OK)
        {
            m_log.error("Error setting Controlword");
            return err;
        }
        err = writeOpenRegisters("Modes Of Operation", 0xFE, 1);
        if (err != OK)
        {
            m_log.error("Error setting Modes Of Operation");
            return err;
        }
        err = writeOpenRegisters("Blink LEDs", 1, 1);
        if (err != OK)
        {
            m_log.error("Error setting Blink LEDs");
            return err;
        }
        return MDCO::Error_t::OK;
    }

    MDCO::Error_t MDCO::openReset()
    {
        // reset the motor via SDO message, log an error message if transfer failed
        Error_t err;
        err = writeOpenRegisters("Controlword", 0x06, 2);
        if (err != OK)
        {
            m_log.error("Error setting Controlword");
            return err;
        }
        err = writeOpenRegisters("Modes Of Operation", 0xFE, 1);
        if (err != OK)
        {
            m_log.error("Error setting Modes Of Operation");
            return err;
        }
        err = writeOpenRegisters("Reset Controller", 1, 1);
        if (err != OK)
        {
            m_log.error("Error setting Reset Controller");
            return err;
        }
        return MDCO::Error_t::OK;
    }

    MDCO::Error_t MDCO::clearOpenErrors(i16 level)
    {
        Error_t err;
        // restart node
        std::vector<u8> data = {0x81, (u8)m_canId};
        err                  = writeOpenPDORegisters(0x000, data);
        if (err != OK)
        {
            m_log.error("Error restarting node");
        }
        m_log.debug("waiting the node %d to restart\n", m_canId);
        // wait for the node to restart
        auto start   = std::chrono::steady_clock::now();
        auto timeout = std::chrono::milliseconds((5000));
        while (std::chrono::steady_clock::now() - start < timeout)
        {
        }
        // clearing error register
        err = writeOpenRegisters("Controlword", 0x06, 2);
        if (err != OK)
        {
            m_log.error("Error setting Controlword");
            return err;
        }
        err = writeOpenRegisters("Modes Of Operation", 0xFF, 1);
        if (err != OK)
        {
            m_log.error("Error setting Modes Of Operation");
            return err;
        }
        start   = std::chrono::steady_clock::now();
        timeout = std::chrono::milliseconds((100));
        while (std::chrono::steady_clock::now() - start < timeout)
        {
        }
        // clear errors or warnings or both depending on the level
        if (level == 1)
            return writeOpenRegisters("Clear Errors", 1, 1);
        if (level == 2)
            return writeOpenRegisters("Clear Warnings", 1, 1);
        if (level == 3)
        {
            err = writeOpenRegisters("Clear Errors", 1, 1);
            if (err == OK)
            {
                return err;
            }
            else
                return MDCO::Error_t::TRANSFER_FAILED;
        }
        else
            return MDCO::Error_t::REQUEST_INVALID;
    }

    MDCO::Error_t MDCO::newCanOpenConfig(i32 newID, i32 newBaud, u32 watchdog)
    {
        // set new can configuration, log an error message if transfer failed
        Error_t err;
        err = writeOpenRegisters("Can ID", newID, 4);
        if (err != OK)
        {
            m_log.error("Error setting Can ID");
            return err;
        }
        err = writeOpenRegisters("Can Baudrate", newBaud, 4);
        if (err != OK)
        {
            m_log.error("Error setting Can Baudrate");
            return err;
        }
        err = writeOpenRegisters("Can Watchdog", watchdog);
        if (err != OK)
        {
            m_log.error("Error setting Can Watchdog");
            return err;
        }
        return MDCO::Error_t::OK;
    }

    MDCO::Error_t MDCO::testHeartbeat()
    {
        // send heartbeat and check if the motor responds correctly, log an error message if
        // transfer
        uint32_t heartbeat_id = 0x700 + (uint32_t)this->m_canId;
        m_log.info("Waiting for a heartbeat message with can id 0x%03X...", heartbeat_id);

        std::vector<u8>           frame = {0x00};
        std::vector<uint8_t>      response;
        mab::candleTypes::Error_t error;

        uint64_t firstHeartbeatReceived = 0;

        // Chrono setup
        auto start_time = std::chrono::steady_clock::now();
        auto timeout    = std::chrono::seconds(5);

        while (std::chrono::steady_clock::now() - start_time < timeout)
        {
            // send Heartbeat CANopen frame with high frequency
            auto result =
                transferCanOpenFrameNoRespondExpected(heartbeat_id, frame, 1, /*timeoutMs=*/10);
            response = result.first;
            error    = result.second;

            // if a incorrect message is received, ignore it
            if (error != mab::candleTypes::Error_t::OK)
                continue;

            // if the received message is not a Heartbeat, ignore it
            if ((int)response.size() <= 4)
                continue;

            // heartbeat received
            m_log.success("heartbeat received");

            auto now_us = std::chrono::duration_cast<std::chrono::microseconds>(
                              std::chrono::steady_clock::now() - start_time)
                              .count();

            // first heartbeat
            if (firstHeartbeatReceived == 0)
            {
                firstHeartbeatReceived = now_us;

                // keep sending message until the last heartbeat disappears on the bus or timeout
                do
                {
                    result = transferCanOpenFrameNoRespondExpected(
                        heartbeat_id, frame, 1, /*timeoutMs=*/10);
                    response = result.first;
                    error    = result.second;
                } while ((error == mab::candleTypes::Error_t::OK && response.size() > 1 &&
                          response[1] == 0x01) &&
                         (std::chrono::steady_clock::now() - start_time < timeout));

                continue;  // keep waiting for the second heartbeat
            }

            // second heartbeat — calculating delta time
            auto delta_us = std::chrono::duration_cast<std::chrono::microseconds>(
                                std::chrono::steady_clock::now() - start_time)
                                .count() -
                            firstHeartbeatReceived;

            m_log.success("heartbeat received with in between time of %.6lf s",
                          static_cast<double>(delta_us) / 1'000'000.0);
            return OK;
        }

        // ---------- Timeout ----------
        if (firstHeartbeatReceived == 0)
            m_log.error("No heartbeat has been received after 5s.\n");
        else
            m_log.success("One heartbeat has been received in the last 5s");

        return OK;
    }

    MDCO::Error_t MDCO::openSave()
    {
        // save the motor configuration via SDO message, log an error message if transfer failed
        Error_t err;
        err = writeOpenRegisters("Controlword", 0x06, 2);
        if (err != OK)
        {
            m_log.error("Error setting Controlword for openSave");
            return err;
        }
        err = writeOpenRegisters(
            "Save all parameters", 0x65766173, 4);  // 0x65766173="save" in ASCII and little endian
        if (err != OK)
        {
            m_log.error("Error saving all parameters");
            return err;
        }
        // wait for motor to restart
        auto start   = std::chrono::steady_clock::now();
        auto timeout = std::chrono::milliseconds((2000));
        while (std::chrono::steady_clock::now() - start < timeout)
        {
        }
        return MDCO::Error_t::OK;
    }

    MDCO::Error_t MDCO::openZero()
    {
        // set the motor zero position to the actual position via SDO message, log an error message
        // if transfer failed
        Error_t err;
        err = writeOpenRegisters("Controlword", 0x06, 2);
        if (err != OK)
        {
            m_log.error("Error setting Controlword for openZero");
            return err;
        }
        err = writeOpenRegisters("Modes Of Operation", 0xFE, 1);
        if (err != OK)
        {
            m_log.error("Error setting Modes Of Operation for openZero");
            return err;
        }
        err = writeOpenRegisters("Set Zero", 1, 1);
        if (err != OK)
        {
            m_log.error("Error setting Set Zero");
            return err;
        }
        // verify that the zero position has been set correctly
        if ((getValueFromOpenRegister(0x6064, 0) > -50) &&
            (getValueFromOpenRegister(0x6064, 0) < 50))
        {
            m_log.success("Zero update");
            return MDCO::Error_t::OK;
        }
        else
        {
            m_log.error("Zero not update");
            return MDCO::Error_t::TRANSFER_FAILED;
        }
    }

    MDCO::Error_t MDCO::testEncoder(bool Main, bool output)
    {
        // test the motor encoders via SDO message, log an error message if transfer failed
        Error_t err;
        err = writeOpenRegisters("Controlword", 0x06, 2);
        if (err != OK)
        {
            m_log.error("Error setting Controlword for testEncoder");
            return err;
        }
        err = writeOpenRegisters("Modes Of Operation", 0xFE, 1);
        if (err != OK)
        {
            m_log.error("Error setting Modes Of Operation for testEncoder");
            return err;
        }
        if (Main)
        {
            err = writeOpenRegisters("Test Main Encoder", 1, 1);
            if (err != OK)
            {
                m_log.error("Error setting Test Main Encoder");
                return err;
            }
        }
        if (output)
        {
            err = writeOpenRegisters("Test Output Encoder", 1, 1);
            if (err != OK)
            {
                m_log.error("Error setting Test Output Encoder");
                return err;
            }
        }
        return MDCO::Error_t::OK;
    }

    MDCO::Error_t MDCO::encoderCalibration(bool Main, bool output)
    {
        // calibrate the motor encoders via SDO message, log an error message if transfer failed
        Error_t err;
        err = writeOpenRegisters("Controlword", 0x06, 2);
        if (err != OK)
        {
            m_log.error("Error setting Controlword for encoderCalibration");
            return err;
        }
        err = writeOpenRegisters("Modes Of Operation", 0xFE, 1);
        if (err != OK)
        {
            m_log.error("Error setting Modes Of Operation for encoderCalibration");
            return err;
        }
        if (Main)
        {
            err = writeOpenRegisters("Run Calibration", 1, 1);
            if (err != OK)
            {
                m_log.error("Error setting Run Calibration");
                return err;
            }
        }
        if (output)
        {
            err = writeOpenRegisters("Run Output Encoder Calibration", 1, 1);
            if (err != OK)
            {
                m_log.error("Error setting Run Output Encoder Calibration");
                return err;
            }
        }
        return MDCO::Error_t::OK;
    }

    MDCO::Error_t MDCO::readOpenRegisters(i16 index, u8 subindex, bool force)
    {
        // check if the object is readable unless force is true
        if (!force)
        {
            if (isReadable(index, subindex) != OK)
            {
                m_log.error("Object 0x%04x:0x%02x is not Readable!", index, subindex);
                return REQUEST_INVALID;
            }
        }

        m_log.debug("Read Open register...");
        std::vector<u8> frame = {
            0x40,
            ((u8)index),
            ((u8)(index >> 8)),
            subindex,
            0,
            0,
            0,
            0,
        };

        //  message sending via transferCanFrame
        auto [response, error] =
            transferCanOpenFrame(SDO_REQUEST_BASE + m_canId, frame, frame.size());

        // data display
        std::stringstream ss;

        ss << "\n\n ---- Received CAN Frame Info ----" << "\n";

        u8 cmd = response[0];
        if ((cmd & 0xF0) != 0x40)
        {
            ss << "Frame not recognized as an SDO Upload Expedited response." << "\n";
            return Error_t::TRANSFER_FAILED;
        }
        else
        {
            // FLAGS Extraction
            u8 n       = (cmd & 0x0C) >> 2;  // bits 1-0
            u8 dataLen = 4 - n;

            // Index et Subindex
            u16 index    = response[2] << 8 | response[1];
            u8  subindex = response[3];

            // data display
            ss << "Index      : 0x" << std::hex << std::setw(4) << std::setfill('0') << index
               << "\n";

            ss << "Subindex   : 0x" << std::hex << std::setw(2) << std::setfill('0')
               << (i16)subindex << "\n";

            ss << "Data (" << std::dec << (i16)dataLen << " byte(s)): 0x";

            for (i16 i = dataLen - 1; i >= 0; --i)
            {
                ss << std::hex << std::setw(2) << std::setfill('0') << (i16)response[4 + i];
            }
            ss << "\n" << "------------------------" << "\n";
            m_log.info("%s\n", ss.str().c_str());
        }

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

    MDCO::Error_t MDCO::writeLongOpenRegisters(i16                index,
                                               short              subindex,
                                               const std::string& dataString,
                                               bool               force)
    {
        // check if the object is writable unless force is true
        if (!force)
        {
            if (isWritable(index, subindex) != OK)
            {
                m_log.error("Object 0x%04x:0x%02x is not writable!", index, subindex);
                return REQUEST_INVALID;
            }
        }
        std::string motorName = dataString;

        m_log.debug("Writing Motor Name to 0x2000:0x06 via segmented SDO...");

        // 1. prepare data to send clip data to 20 bytes (Motor Name)
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
            transferCanOpenFrame(SDO_REQUEST_BASE + m_canId, initFrame, initFrame.size());

        if (initError != mab::candleTypes::Error_t::OK)
        {
            m_log.error("Failed to initiate segmented SDO download.");
            return Error_t::TRANSFER_FAILED;
        }

        if (initResponse.size() < 1 || (initResponse[0] & 0xE0) != 0x60)
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
                transferCanOpenFrame(SDO_REQUEST_BASE + m_canId, segmentFrame, segmentFrame.size());

            if (segError != mab::candleTypes::Error_t::OK)
            {
                m_log.error("Segmented transfer failed at offset {}", offset);
                return Error_t::TRANSFER_FAILED;
            }

            // Check server response: must be 0x20 | toggle
            if (segResponse.size() < 1 || (segResponse[0] & 0xE0) != 0x20)
            {
                m_log.error("Malformed segment ACK.");
                return Error_t::TRANSFER_FAILED;
            }

            if ((segResponse[0] & 0x10) != (toggle << 4))
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

    MDCO::Error_t MDCO::readLongOpenRegisters(i16              index,
                                              short            subindex,
                                              std::vector<u8>& outData,
                                              bool             silent)
    {  // check if the object is readable
        if (isReadable(index, subindex) != OK)
        {
            m_log.error("Object 0x%04x:0x%02x is not readable!", index, subindex);
            return Error_t::REQUEST_INVALID;
        }

        m_log.debug("Read Object (0x%lx:0x%x) via segmented SDO…", index, subindex);

        // ---------- 1) Initiation Request ----------
        std::vector<u8> initReq = {
            0x40, u8(index & 0xFF), u8(index >> 8), u8(subindex), 0x00, 0x00, 0x00, 0x00};

        auto [rspInit, errInit] = transferCanOpenFrame(0x600 + m_canId, initReq, initReq.size());
        if (errInit != mab::candleTypes::Error_t::OK || rspInit.size() < 8)
        {
            m_log.error("Failed to initiate SDO read.");
            return Error_t::TRANSFER_FAILED;
        }

        u8   cmd         = rspInit[0];
        bool isExpedited = cmd & 0x02;
        bool hasSize     = cmd & 0x01;

        // ---------- 2a) Expedited transfer ----------
        if (isExpedited)
        {
            m_log.warn("Data received in expedited mode, probably ≤ 4 bytes.");
            u8 n   = ((cmd >> 2) & 0x03);  // number of unused bytes
            u8 len = 4 - n;

            outData.insert(outData.end(), rspInit.begin() + 4, rspInit.begin() + 4 + len);

            // ---------- 3) Display ----------
            if (dataSizeOfEdsObject(index, subindex) == 0)
            {
                std::string motorName(outData.begin(), outData.end());
                if (!silent)
                    m_log.info("Data received (string): '%s'", motorName.c_str());
            }
            else
            {
                if (!silent)
                    m_log.info("Data received: %s",
                               std::string(outData.begin(), outData.end()).c_str());
            }
            return Error_t::OK;
        }

        // ---------- 2b) Segmented transfer ----------
        u32 totalLen = 0;
        if (hasSize)
        {
            totalLen = rspInit[4] | (rspInit[5] << 8) | (rspInit[6] << 16) | (rspInit[7] << 24);
            outData.reserve(totalLen);
        }

        bool toggle   = false;
        bool finished = false;

        while (!finished)
        {
            std::vector<u8> segReq = {u8(0x60 | (toggle ? 0x10 : 0x00)), 0, 0, 0, 0, 0, 0, 0};
            auto [rspSeg, errSeg] =
                transferCanOpenFrame(SDO_REQUEST_BASE + m_canId, segReq, segReq.size());

            if (errSeg != mab::candleTypes::Error_t::OK || rspSeg.size() < 1)
            {
                m_log.error("Error segment reading");
                return Error_t::TRANSFER_FAILED;
            }

            u8 segCmd = rspSeg[0];
            if ((segCmd & 0x10) != (toggle ? 0x10 : 0x00))
            {
                m_log.error("Bit toggle unexpected, corrupt transfer.");
                return Error_t::TRANSFER_FAILED;
            }

            bool last    = segCmd & 0x01;
            u8   unused  = (segCmd >> 1) & 0x07;
            u8   dataLen = 7 - unused;

            if ((i16)rspSeg.size() < (1 + dataLen))
            {
                m_log.error("Incomplete data in the segment.");
                return Error_t::TRANSFER_FAILED;
            }

            outData.insert(outData.end(), rspSeg.begin() + 1, rspSeg.begin() + 1 + dataLen);
            finished = last;
            toggle   = !toggle;
        }

        if (hasSize && outData.size() != totalLen)
        {
            m_log.warn("Size of data read (%d) ≠ size announced (%d)", outData.size(), totalLen);
        }

        // ---------- 3) Display ----------

        if (dataSizeOfEdsObject(index, subindex) == 0)
        {
            std::string motorName(outData.begin(), outData.end());
            if (!silent)
                m_log.info("Data received (string): '%s'", motorName.c_str());
        }
        else
        {
            if (!silent)
                m_log.info("Data received: %s",
                           std::string(outData.begin(), outData.end()).c_str());
        }

        return Error_t::OK;
    }

    i32 MDCO::getValueFromOpenRegister(i16 index, u8 subindex)
    {
        // check if the object is readable
        if (isReadable(index, subindex) != OK)
        {
            m_log.error("Object 0x%04x:0x%02x is not writable!", index, subindex);
            return Error_t::REQUEST_INVALID;
        }
        m_log.debug("Read Open register...");
        // send sdo upload expedited request
        std::vector<u8> frame = {
            0x40,
            ((u8)index),
            ((u8)(index >> 8)),
            subindex,
            0,
            0,
            0,
            0,
        };

        auto [response, error] =
            transferCanOpenFrame(SDO_REQUEST_BASE + m_canId, frame, frame.size());

        i32 answerValue = 0;
        for (i16 i = 0; i <= 4; i++)
        {
            answerValue += (((i32)response[4 + i]) << (8 * i));
        }

        // data display
        u8 cmd = response[0];

        if ((cmd & 0xF0) != 0x40)
        {
            m_log.error("Frame not recognized as an SDO Upload Expedited response.");
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

    MDCO::Error_t MDCO::writeOpenRegisters(
        i16 index, short subindex, i32 data, short size, bool force)
    {
        // check if the object is writable unless force is true
        if (!force)
        {
            if (isWritable(index, subindex) != OK)
            {
                m_log.error("Object 0x%04x:0x%02x is not writable!", index, subindex);
                return Error_t::REQUEST_INVALID;
            }
        }
        // if size is 0, then read the size from the EDS file
        if (size == 0)
        {
            size = dataSizeOfEdsObject(index, subindex);
            if (size == -1)
            {
                // size= -1 mean object not found in EDS file
                m_log.error(
                    "Object 0x%04x:0x%02x has an unsupported size (%d)!", index, subindex, size);
                return Error_t::REQUEST_INVALID;
            }
            else if (size == 0 || size > 4)
            {
                // size=0 mean object is string or array, size>4 not supported in expedited transfer
                m_log.error(
                    "Object 0x%04x:0x%02x has an unsupported size (%d), please use an "
                    "Segmented transfer !",
                    index,
                    subindex,
                    size);
                return Error_t::REQUEST_INVALID;
            }
        }
        // send sdo download expedited request
        std::vector<u8> frame;
        frame.reserve(8);
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

        auto [response, error] =
            transferCanOpenFrame(SDO_REQUEST_BASE + m_canId, frame, frame.size());

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

    MDCO::Error_t MDCO::writeOpenRegisters(const std::string& name, u32 data, u8 size, bool force)
    {  // search index and subindex from the EDS file corresponding to the name
        u32 index    = 0;
        u8  subIndex = 0;
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
        auto err = writeOpenRegisters(index, subIndex, data, size);
        if (m_log.g_m_verbosity == Logger::Verbosity_E::VERBOSITY_3)
            m_log.debug("Error:%d\n", readOpenRegisters(index, subIndex));
        return err;
    }

    MDCO::Error_t MDCO::writeOpenPDORegisters(i16 index, std::vector<u8> data)
    {
        // send PDO write request, PDO is slave/master communication mode so the motor will not
        // respond
        m_log.debug("Writing Open Pdo register...");

        auto [response, error] = transferCanOpenFrameNoRespondExpected(index, data, data.size());

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

    MDCO::Error_t MDCO::sendCustomData(i16 index, std::vector<u8> data)
    {
        m_log.debug("Writing Custom data...");
        transferCanOpenFrameNoRespondExpected(index, data, data.size());
        return Error_t::OK;
    }

    std::vector<canId_t> MDCO::discoverOpenMDs(Candle* candle)
    {
        constexpr canId_t MIN_VALID_ID = 0x01;  // ids less than that are reserved for special
        constexpr canId_t MAX_VALID_ID = 0x7F;  // 0x600-0x580=0x7F

        std::vector<u8> frame = {
            0x40,  // Command: initiate upload
            0x00,  // Index LSB
            0x10,  // Index MSB
            0,     // Subindex
            0,     // Padding
            0,
            0,
            0,
        };

        Logger               log(Logger::ProgramLayer_E::TOP, "MD_DISCOVERY");
        std::vector<canId_t> ids;

        if (candle == nullptr)
        {
            log.error("Candle is empty!");
            return std::vector<canId_t>();
        }

        log.info("Looking for MDs");

        for (canId_t id = MIN_VALID_ID; id < MAX_VALID_ID; id++)
        {
            log.debug("Trying to bind MD with id %d", id);
            log.progress(float(id) / float(MAX_VALID_ID));
            // workaround for ping error spam
            Logger::Verbosity_E prevVerbosity =
                Logger::g_m_verbosity.value_or(Logger::Verbosity_E::VERBOSITY_1);
            Logger::g_m_verbosity = Logger::Verbosity_E::SILENT;
            MDCO md(id, candle);
            auto [response, error] = md.transferCanOpenFrame(0x600 + id, frame, frame.size());

            if (response[4] == 0x92)
                ids.push_back(id);

            Logger::g_m_verbosity = prevVerbosity;
        }
        for (canId_t id : ids)
        {
            log.info("Discovered MD device with ID: %d", id);
        }
        if (ids.size() > 0)
            return ids;

        log.warn("Have not found any MD devices on the CAN bus!");
        return ids;
    }
}  // namespace mab