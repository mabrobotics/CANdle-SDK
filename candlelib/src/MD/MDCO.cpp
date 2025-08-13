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

    void MDCO::movePositionAcc(i32 targetPos, moveParameter param)
    {
        writeOpenRegisters("Motor Max Acceleration", 10000, 4);
        writeOpenRegisters("Motor Max Deceleration", 10000, 4);
        writeOpenRegisters("Motor Profile Acceleration", param.accLimit, 4);
        writeOpenRegisters("Motor Profile Deceleration", param.dccLimit, 4);
        writeOpenRegisters("Max Current", param.MaxCurrent, 2);
        writeOpenRegisters("Motor Rated Current", param.RatedCurrent, 4);
        writeOpenRegisters("Max Motor Speed", param.MaxSpeed, 4);
        writeOpenRegisters("Motor Max Torque", param.MaxTorque, 2);
        writeOpenRegisters("Motor Rated Torque", param.RatedTorque, 4);
        writeOpenRegisters("Modes Of Operation", 1, 1);
        writeOpenRegisters("Controlword", 0x80, 2);
        writeOpenRegisters("Controlword", 0x06, 2);
        writeOpenRegisters("Controlword", 15, 2);
        m_log.debug("position asked : %d\n", targetPos);
        time_t start = time(nullptr);
        while (time(nullptr) - start < 5 &&
               !((i16)getValueFromOpenRegister(0x6064, 0) > (targetPos - 100) &&
                 (i16)getValueFromOpenRegister(0x6064, 0) < (targetPos + 100)))
        {
            writeOpenRegisters("Motor Target Position", targetPos, 4);

            usleep(10000);
        }

        m_log.debug("actual position: %d\n", (i16)getValueFromOpenRegister(0x6064, 0));

        if (((i16)getValueFromOpenRegister(0x6064, 0) > (targetPos - 200) &&
             (i16)getValueFromOpenRegister(0x6064, 0) < (targetPos + 200)))
        {
            m_log.success("Position reached in less than 5s");
        }
        else
        {
            m_log.error("Position not reached in less than 5s");
        }
        m_log.debug("actual position: %d\n", (i16)getValueFromOpenRegister(0x6064, 0));
        writeOpenRegisters("Controlword", 6, 2);
        writeOpenRegisters("Modes Of Operation", 0, 1);
    }

    void MDCO::movePosition(moveParameter param, i32 DesiredPos)
    {
        writeOpenRegisters("Max Current", param.MaxCurrent);
        writeOpenRegisters("Motor Rated Current", param.RatedCurrent);
        writeOpenRegisters("Max Motor Speed", param.MaxSpeed);
        writeOpenRegisters("Motor Max Torque", param.MaxTorque);
        writeOpenRegisters("Motor Rated Torque", param.RatedTorque);
        writeOpenRegisters("Modes Of Operation", 8);
        writeOpenRegisters("Controlword", 0x80);
        writeOpenRegisters("Controlword", 0x06);
        writeOpenRegisters("Controlword", 15);
        m_log.debug("position asked : %d\n", DesiredPos);
        time_t start = time(nullptr);
        while (time(nullptr) - start < 5 &&
               !((i16)getValueFromOpenRegister(0x6064, 0) > (DesiredPos - 100) &&
                 (i16)getValueFromOpenRegister(0x6064, 0) < (DesiredPos + 100)))
        {
            writeOpenRegisters("Motor Target Position", DesiredPos);
            usleep(10000);
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
        writeOpenRegisters("Controlword", 6);
        writeOpenRegisters("Modes Of Operation", 0);
    }

    void MDCO::moveSpeed(moveParameter param, i32 DesiredSpeed)
    {
        writeOpenRegisters("Max Current", param.MaxCurrent);
        writeOpenRegisters("Motor Rated Current", param.RatedCurrent);
        writeOpenRegisters("Max Motor Speed", param.MaxSpeed);
        writeOpenRegisters("Motor Max Torque", param.MaxTorque);
        writeOpenRegisters("Motor Rated Torque", param.RatedTorque);
        writeOpenRegisters("Modes Of Operation", 9);
        writeOpenRegisters("Controlword", 0x80);
        writeOpenRegisters("Controlword", 0x06);
        writeOpenRegisters("Controlword", 15);
        usleep(10000);
        time_t start = time(nullptr);
        while (time(nullptr) - start < 5)
        {
            writeOpenRegisters("Motor Target Velocity", DesiredSpeed);
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
        writeOpenRegisters("Motor Target Velocity", 0);
        writeOpenRegisters("Controlword", 6);
        writeOpenRegisters("Modes Of Operation", 0);
    }

    MDCO::Error_t MDCO::moveImpedance(
        i32 desiredSpeed, i32 targetPos, f32 kp, f32 kd, i16 torque, moveParameter param)
    {
        usleep(1000);
        writeOpenRegisters("Max Current", param.MaxCurrent);
        writeOpenRegisters("Motor Rated Current", param.RatedCurrent);
        writeOpenRegisters("Max Motor Speed", param.MaxSpeed);
        writeOpenRegisters("Motor Max Torque", param.MaxTorque);
        writeOpenRegisters("Motor Rated Torque", param.RatedTorque);
        writeOpenRegisters("Modes Of Operation", 0xFD);
        writeOpenRegisters("Controlword", 0x80);
        writeOpenRegisters("Controlword", 0x06);
        writeOpenRegisters("Controlword", 15);
        writeOpenRegisters("Target Velocity", desiredSpeed);
        writeOpenRegisters("Target Position", targetPos);
        // kp
        u32 kp_bits;
        memcpy(&kp_bits, &kp, sizeof(float));
        writeOpenRegisters("Kp_impedance", kp_bits);
        // kd
        u32 kd_bits;
        memcpy(&kd_bits, &kd, sizeof(float));
        writeOpenRegisters("Kd_impedance", kd_bits);
        writeOpenRegisters("Target Torque", torque);
        usleep(5000000);
        writeOpenRegisters("Target Torque", 0);
        writeOpenRegisters("Controlword", 0x80);
        writeOpenRegisters("Controlword", 0x06);
        writeOpenRegisters("Modes Of Operation", 0);
        writeOpenRegisters("Controlword", 15);
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
        data.push_back(0x81);
        data.push_back(m_canId);
        writeOpenPDORegisters(0x000, data);
        m_log.debug("waiting the node %d to restart\n", m_canId);
        // wait for the node to restart
        usleep(5000000);
        // clearing error register
        writeOpenRegisters("Controlword", 0x06, 2);
        writeOpenRegisters("Modes Of Operation", 0xFF, 1);
        usleep(100000);
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
