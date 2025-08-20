#include "MDCO.hpp"

namespace mab
{

    MDCO::Error_t MDCO::findObjectByName(const std::string& searchTerm, u32& index, u8& subIndex)
    {
        bool objectFound = false;
        for (const edsObject& obj : this->ObjectDictionary)
        {
            if (obj.ParameterName.size() == searchTerm.size())
            {
                bool equal = true;
                for (size_t i = 0; i < obj.ParameterName.size(); ++i)
                {
                    if (tolower(obj.ParameterName[i]) != tolower(searchTerm[i]))
                    {
                        equal = false;
                        break;
                    }
                }
                if (equal)
                {
                    index    = obj.index;
                    subIndex = obj.subIndex;
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
        if (objectFound)
        {
            return OK;
        }

        return UNKNOWN_OBJECT;
    }

    MDCO::Error_t MDCO::isWritable(const u32 index, const u8 subIndex)
    {
        for (const edsObject& obj : this->ObjectDictionary)
        {
            if (index == obj.index && subIndex == obj.subIndex)
            {
                if (obj.accessType.find('w') != std::string::npos ||
                    obj.accessType.find('W') != std::string::npos)
                {
                    return MDCO::Error_t::OK;
                }
                else
                {
                    // m_log.error("Object 0x%04X subIndex %d is not writable.", index, subIndex);
                    return MDCO::Error_t::REQUEST_INVALID;
                }
            }
        }

        return MDCO::Error_t::UNKNOWN_OBJECT;
    }

    MDCO::Error_t MDCO::isReadable(const u32 index, const u8 subIndex)
    {
        for (const edsObject& obj : this->ObjectDictionary)
        {
            if (index == obj.index && subIndex == obj.subIndex)
            {
                if (obj.accessType.find('r') != std::string::npos ||
                    obj.accessType.find('R') != std::string::npos)
                {
                    return MDCO::Error_t::OK;
                }
                else
                {
                    // m_log.error("Object 0x%04X subIndex %d is not readable.", index, subIndex);
                    return MDCO::Error_t::REQUEST_INVALID;
                }
            }
        }

        return MDCO::Error_t::UNKNOWN_OBJECT;
    }

    u8 MDCO::dataSizeOfEdsObject(const u32 index, const u8 subIndex)
    {
        for (const edsObject& obj : this->ObjectDictionary)
        {
            if (index == obj.index && subIndex == obj.subIndex)
            {
                if (obj.DataType == 0x0001 || obj.DataType == 0x0002 || obj.DataType == 0x0005)
                {
                    return 1;
                }
                else if (obj.DataType == 0x0003 || obj.DataType == 0x0006)
                {
                    return 2;
                }
                else if (obj.DataType == 0x0004 || obj.DataType == 0x0007 || obj.DataType == 0x0008)
                {
                    return 4;
                }
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

    MDCO::Error_t MDCO::setProfileParameters(moveParameter param)
    {
        writeOpenRegisters("Motor Max Acceleration", param.accLimit);
        writeOpenRegisters("Motor Max Deceleration", param.dccLimit);
        writeOpenRegisters("Max Current", param.MaxCurrent);
        writeOpenRegisters("Motor Rated Current", param.RatedCurrent);
        writeOpenRegisters("Max Motor Speed", param.MaxSpeed);
        writeOpenRegisters("Motor Max Torque", param.MaxTorque);
        writeOpenRegisters("Motor Rated Torque", param.RatedTorque);
        return OK;
    }

    MDCO::Error_t MDCO::enableDriver(ModesOfOperation mode)
    {
        writeOpenRegisters("Modes Of Operation", mode, 1);
        writeOpenRegisters("Controlword", 0x80, 2);
        writeOpenRegisters("Controlword", 0x06, 2);
        writeOpenRegisters("Controlword", 15, 2);
        return OK;
    }

    MDCO::Error_t MDCO::disableDriver()
    {
        writeOpenRegisters("Motor Target Velocity", 0);
        writeOpenRegisters("Controlword", 6);
        writeOpenRegisters("Modes Of Operation", 0);
        return OK;
    }

    void MDCO::movePosition(i32 DesiredPos)
    {
        auto start      = std::chrono::steady_clock::now();
        auto lastSend   = start;
        auto timeout    = std::chrono::seconds(5);
        auto sendPeriod = std::chrono::milliseconds(10);

        while (std::chrono::steady_clock::now() - start < timeout &&
               !((i16)getValueFromOpenRegister(0x6064, 0) > (DesiredPos - 100) &&
                 (i16)getValueFromOpenRegister(0x6064, 0) < (DesiredPos + 100)))
        {
            auto now = std::chrono::steady_clock::now();
            if (now - lastSend >= sendPeriod)
            {
                writeOpenRegisters("Motor Target Position", DesiredPos, 4);
                lastSend = now;
            }
        }
        m_log.debug("actual position: %d\n", (i16)getValueFromOpenRegister(0x6064, 0));
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

    void MDCO::moveSpeed(i32 DesiredSpeed)
    {
        auto start      = std::chrono::steady_clock::now();
        auto lastSend   = start;
        auto timeout    = std::chrono::seconds(5);
        auto sendPeriod = std::chrono::milliseconds(10);

        while (std::chrono::steady_clock::now() - start < timeout)
        {
            auto now = std::chrono::steady_clock::now();
            if (now - lastSend >= sendPeriod)
            {
                writeOpenRegisters("Motor Target Velocity", DesiredSpeed);
                lastSend = now;
            }
        }

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

    MDCO::Error_t MDCO::moveImpedance(i32 desiredSpeed, i32 targetPos, moveParameter param)
    {
        // kp
        u32 kp_bits;
        memcpy(&kp_bits, &(param.kp), sizeof(float));
        writeOpenRegisters("Kp_impedance", kp_bits);
        // kd
        u32 kd_bits;
        memcpy(&kd_bits, &(param.kd), sizeof(float));
        writeOpenRegisters("Kd_impedance", kd_bits);
        writeOpenRegisters("Target Torque", param.torqueff);
        auto start   = std::chrono::steady_clock::now();
        auto timeout = std::chrono::seconds((5));
        while (std::chrono::steady_clock::now() - start < timeout)
        {
        }
        writeOpenRegisters("Target Torque", 0);
        return MDCO::Error_t::OK;
    }

    MDCO::Error_t MDCO::blinkOpenTest()
    {
        writeOpenRegisters("Controlword", 0x06, 2);
        writeOpenRegisters("Modes Of Operation", 0xFE, 1);
        writeOpenRegisters("Blink LEDs", 1, 1);
        return MDCO::Error_t::OK;
    }

    MDCO::Error_t MDCO::openReset()
    {
        writeOpenRegisters("Controlword", 0x06, 2);
        writeOpenRegisters("Modes Of Operation", 0xFE, 1);
        writeOpenRegisters("Reset Controller", 1, 1);
        return MDCO::Error_t::OK;
    }

    MDCO::Error_t MDCO::clearOpenErrors(i16 level)
    {
        // restart node
        std::vector<u8> data;
        data.reserve(2);
        data.push_back(0x81);
        data.push_back(m_canId);
        writeOpenPDORegisters(0x000, data);
        m_log.debug("waiting the node %d to restart\n", m_canId);
        // wait for the node to restart
        auto start   = std::chrono::steady_clock::now();
        auto timeout = std::chrono::milliseconds((5000));
        while (std::chrono::steady_clock::now() - start < timeout)
        {
        }
        // clearing error register
        writeOpenRegisters("Controlword", 0x06, 2);
        writeOpenRegisters("Modes Of Operation", 0xFF, 1);
        start   = std::chrono::steady_clock::now();
        timeout = std::chrono::milliseconds((100));
        while (std::chrono::steady_clock::now() - start < timeout)
        {
        }
        if (level == 1)
            return writeOpenRegisters("Clear Errors", 1, 1);
        if (level == 2)
            return writeOpenRegisters("Clear Warnings", 1, 1);
        if (level == 3)
        {
            if (!writeOpenRegisters("Clear Errors", 1, 1))
            {
                return writeOpenRegisters("Clear Errors", 1, 1);
            }
            else
                return MDCO::Error_t::TRANSFER_FAILED;
        }
        else
            return MDCO::Error_t::REQUEST_INVALID;
    }

    MDCO::Error_t MDCO::newCanOpenConfig(i32 newID, i32 newBaud, i16 newwatchdog)
    {
        writeOpenRegisters("Can ID", newID, 4);
        writeOpenRegisters("Can Baudrate", newBaud, 4);
        if (newwatchdog != 0)
        {
            writeOpenRegisters("Can Watchdog", newwatchdog, 2);
        }
        return MDCO::Error_t::OK;
    }

    MDCO::Error_t MDCO::canOpenBandwidth(i16 newBandwidth)
    {
        writeOpenRegisters("Torque Bandwidth", newBandwidth, 2);
        return MDCO::Error_t::OK;
    }

    MDCO::Error_t MDCO::testHeartbeat()
    {
        // TODO: find a better way to do this, it seems to work but it's clearly not the best way to
        // do it. A better way could be by implemented a listen mode on the USB.cpp file
        uint32_t heartbeat_id = 0x700 + (uint32_t)this->m_canId;

        m_log.info("Waiting for a heartbeat message with can id 0x%03X...", heartbeat_id);

        std::vector<uint8_t> frame;
        frame.reserve(1);
        frame.push_back(0);

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

            // if a correct message is received
            if (error == mab::candleTypes::Error_t::OK && response.size() >= 3)
            {
                // if the message is a Heartbeat
                if (response[0] == 0x04 && response[1] == 0x01 && response[2] == 0x05)
                {
                    m_log.success("heartbeat received");

                    auto now_us = std::chrono::duration_cast<std::chrono::microseconds>(
                                      std::chrono::steady_clock::now() - start_time)
                                      .count();

                    // first heartbeat
                    if (firstHeartbeatReceived == 0)
                    {
                        firstHeartbeatReceived = now_us;

                        result = transferCanOpenFrameNoRespondExpected(
                            heartbeat_id, frame, 1, /*timeoutMs=*/10);
                        response = result.first;
                        error    = result.second;

                        // keep sending message until the last heartbeat disappears on the bus
                        while ((response.size() > 1 && response[1] == 0x01) &&
                               (std::chrono::steady_clock::now() - start_time < timeout))
                        {
                            result = transferCanOpenFrameNoRespondExpected(
                                heartbeat_id, frame, 1, /*timeoutMs=*/10);
                            response = result.first;
                            error    = result.second;

                            // if we lost communication with the MD
                            if (error != mab::candleTypes::Error_t::OK || response.size() <= 1)
                                break;
                        }
                    }
                    else
                    {
                        // second heartbeat — calculating delta time
                        auto delta_us = std::chrono::duration_cast<std::chrono::microseconds>(
                                            std::chrono::steady_clock::now() - start_time)
                                            .count() -
                                        firstHeartbeatReceived;

                        m_log.success("heartbeat received with in between time of %.6lf s",
                                      static_cast<double>(delta_us) / 1'000'000.0);
                        return OK;
                    }
                }
            }
        }

        // timeout conditions
        if (firstHeartbeatReceived == 0)
            m_log.error("No heartbeat has been received after 5s.\n");
        else
            m_log.success("One heartbeat has been received in the last 5s");
        return OK;
    }

    MDCO::Error_t MDCO::openSave()
    {
        writeOpenRegisters("Controlword", 0x06, 2);
        writeOpenRegisters("Save all parameters", 0x65766173, 4);
        return MDCO::Error_t::OK;
    }

    MDCO::Error_t MDCO::openZero()
    {
        writeOpenRegisters("Controlword", 0x06, 2);
        writeOpenRegisters("Modes Of Operation", 0xFE, 1);
        writeOpenRegisters("Set Zero", 1, 1);

        if ((getValueFromOpenRegister(0x6064, 0) > -50) &&
            (getValueFromOpenRegister(0x6064, 0) < 50))
        {
            m_log.success("Zero update");
            return MDCO::Error_t::OK;
        }
        else
        {
            m_log.error("Zero not update");
            return MDCO::Error_t::OK;
        }
    }

    MDCO::Error_t MDCO::testEncoder(bool Main, bool output)
    {
        writeOpenRegisters("Controlword", 0x06, 2);
        writeOpenRegisters("Modes Of Operation", 0xFE, 1);
        if (Main)
            writeOpenRegisters("Test Main Encoder", 1, 1);
        if (output)
            writeOpenRegisters("Test Output Encoder", 1, 1);
        return MDCO::Error_t::OK;
    }

    MDCO::Error_t MDCO::encoderCalibration(bool Main, bool output)
    {
        writeOpenRegisters("Controlword", 0x06, 2);
        writeOpenRegisters("Modes Of Operation", 0xFE, 1);
        if (Main)
            writeOpenRegisters("Run Calibration", 1, 1);
        if (output)
            writeOpenRegisters("Run Output Encoder Calibration", 1, 1);
        return MDCO::Error_t::OK;
    }

    std::vector<canId_t> MDCO::discoverOpenMDs(Candle* candle)
    {
        constexpr canId_t MIN_VALID_ID = 0x01;  // ids less than that are reserved for special
        constexpr canId_t MAX_VALID_ID = 0x7F;  // 0x600-0x580

        std::vector<u8> frame;
        frame.reserve(8);
        frame.push_back(0x40);  // Command: initiate upload
        frame.push_back(0x00);  // Index LSB
        frame.push_back(0x10);  // Index MSB
        frame.push_back(0x00);  // Subindex
        frame.push_back(0x00);  // Padding
        frame.push_back(0x00);
        frame.push_back(0x00);
        frame.push_back(0x00);

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
