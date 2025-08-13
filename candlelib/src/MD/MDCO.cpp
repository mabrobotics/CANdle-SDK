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

    u8 MDCO::DataSizeOfEdsObject(const u32 index, const u8 subIndex)
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
        long value;
        for (int i = 0; i < (int)ObjectDictionary.size(); i++)
        {
            value =
                GetValueFromOpenRegister(ObjectDictionary[i].index, ObjectDictionary[i].subIndex);
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
                    DataSizeOfEdsObject(ObjectDictionary[i].index, ObjectDictionary[i].subIndex),
                    ObjectDictionary[i].accessType.c_str(),
                    ObjectDictionary[i].PDOMapping,
                    value);
        }
    }

    void MDCO::movePositionAcc(i32 targetPos,
                               i32 accLimit,
                               i32 dccLimit,
                               u32 MaxSpeed,
                               u16 MaxCurrent,
                               u32 RatedCurrent,
                               u16 MaxTorque,
                               u32 RatedTorque)
    {
        WriteOpenRegisters("Motor Max Acceleration", 10000, 4);
        WriteOpenRegisters("Motor Max Deceleration", 10000, 4);
        WriteOpenRegisters("Motor Profile Acceleration", accLimit, 4);
        WriteOpenRegisters("Motor Profile Deceleration", dccLimit, 4);
        WriteOpenRegisters("Max Current", MaxCurrent, 2);
        WriteOpenRegisters("Motor Rated Current", RatedCurrent, 4);
        WriteOpenRegisters("Max Motor Speed", MaxSpeed, 4);
        WriteOpenRegisters("Motor Max Torque", MaxTorque, 2);
        WriteOpenRegisters("Motor Rated Torque", RatedTorque, 4);
        WriteOpenRegisters("Modes Of Operation", 1, 1);
        WriteOpenRegisters("Controlword", 0x80, 2);
        WriteOpenRegisters("Controlword", 0x06, 2);
        WriteOpenRegisters("Controlword", 15, 2);
        m_log.debug("position asked : %d\n", targetPos);
        time_t start = time(nullptr);
        while (time(nullptr) - start < 5 &&
               !((int)GetValueFromOpenRegister(0x6064, 0) > (targetPos - 100) &&
                 (int)GetValueFromOpenRegister(0x6064, 0) < (targetPos + 100)))
        {
            WriteOpenRegisters("Motor Target Position", targetPos, 4);

            usleep(10000);
        }

        m_log.debug("actual position: %d\n", (int)GetValueFromOpenRegister(0x6064, 0));

        if (((int)GetValueFromOpenRegister(0x6064, 0) > (targetPos - 200) &&
             (int)GetValueFromOpenRegister(0x6064, 0) < (targetPos + 200)))
        {
            m_log.success("Position reached in less than 5s");
        }
        else
        {
            m_log.error("Position not reached in less than 5s");
        }
        m_log.debug("actual position: %d\n", (int)GetValueFromOpenRegister(0x6064, 0));
        WriteOpenRegisters("Controlword", 6, 2);
        WriteOpenRegisters("Modes Of Operation", 0, 1);
    }

    void MDCO::movePosition(u16 MaxCurrent,
                            u32 RatedCurrent,
                            u16 MaxTorque,
                            u32 RatedTorque,
                            u32 MaxSpeed,
                            i32 DesiredPos)
    {
        WriteOpenRegisters("Max Current", MaxCurrent);
        WriteOpenRegisters("Motor Rated Current", RatedCurrent);
        WriteOpenRegisters("Max Motor Speed", MaxSpeed);
        WriteOpenRegisters("Motor Max Torque", MaxTorque);
        WriteOpenRegisters("Motor Rated Torque", RatedTorque);
        WriteOpenRegisters("Modes Of Operation", 8);
        WriteOpenRegisters("Controlword", 0x80);
        WriteOpenRegisters("Controlword", 0x06);
        WriteOpenRegisters("Controlword", 15);
        m_log.debug("position asked : %d\n", DesiredPos);
        time_t start = time(nullptr);
        while (time(nullptr) - start < 5 &&
               !((int)GetValueFromOpenRegister(0x6064, 0) > (DesiredPos - 100) &&
                 (int)GetValueFromOpenRegister(0x6064, 0) < (DesiredPos + 100)))
        {
            WriteOpenRegisters("Motor Target Position", DesiredPos);
            usleep(10000);
        }
        m_log.debug("actual position: %d\n", (int)GetValueFromOpenRegister(0x6064, 0));
        if (((int)GetValueFromOpenRegister(0x6064, 0) > (DesiredPos - 200) &&
             (int)GetValueFromOpenRegister(0x6064, 0) < (DesiredPos + 200)))
        {
            m_log.success("Position reached in less than 5s");
        }
        else
        {
            m_log.error("Position not reached in less than 5s");
        }
        WriteOpenRegisters("Controlword", 6);
        WriteOpenRegisters("Modes Of Operation", 0);
    }

    void MDCO::moveSpeed(u16 MaxCurrent,
                         u32 RatedCurrent,
                         u16 MaxTorque,
                         u32 RatedTorque,
                         u32 MaxSpeed,
                         i32 DesiredSpeed)
    {
        WriteOpenRegisters("Max Current", MaxCurrent);
        WriteOpenRegisters("Motor Rated Current", RatedCurrent);
        WriteOpenRegisters("Max Motor Speed", MaxSpeed);
        WriteOpenRegisters("Motor Max Torque", MaxTorque);
        WriteOpenRegisters("Motor Rated Torque", RatedTorque);
        WriteOpenRegisters("Modes Of Operation", 9);
        WriteOpenRegisters("Controlword", 0x80);
        WriteOpenRegisters("Controlword", 0x06);
        WriteOpenRegisters("Controlword", 15);
        usleep(10000);
        time_t start = time(nullptr);
        while (time(nullptr) - start < 5)
        {
            WriteOpenRegisters("Motor Target Velocity", DesiredSpeed);
        }
        if ((int)GetValueFromOpenRegister(0x606C, 0x00) <= DesiredSpeed + 5 &&
            (int)GetValueFromOpenRegister(0x606C, 0x00) >= DesiredSpeed - 5)
        {
            m_log.success("Velocity Target reached with +/- 5RPM");
        }
        else
        {
            m_log.error("Velocity Target not reached");
        }
        WriteOpenRegisters("Motor Target Velocity", 0);
        WriteOpenRegisters("Controlword", 6);
        WriteOpenRegisters("Modes Of Operation", 0);
    }

    MDCO::Error_t MDCO::moveImpedance(i32 desiredSpeed,
                                      i32 targetPos,
                                      f32 kp,
                                      f32 kd,
                                      i16 torque,
                                      u32 MaxSpeed,
                                      u16 MaxCurrent,
                                      u32 RatedCurrent,
                                      u16 MaxTorque,
                                      u32 RatedTorque)
    {
        usleep(1000);
        WriteOpenRegisters("Max Current", MaxCurrent);
        WriteOpenRegisters("Motor Rated Current", RatedCurrent);
        WriteOpenRegisters("Max Motor Speed", MaxSpeed);
        WriteOpenRegisters("Motor Max Torque", MaxTorque);
        WriteOpenRegisters("Motor Rated Torque", RatedTorque);
        WriteOpenRegisters("Modes Of Operation", 0xFD);
        WriteOpenRegisters("Controlword", 0x80);
        WriteOpenRegisters("Controlword", 0x06);
        WriteOpenRegisters("Controlword", 15);
        WriteOpenRegisters("Target Velocity", desiredSpeed);
        WriteOpenRegisters("Target Position", targetPos);
        // kp
        uint32_t kp_bits;
        memcpy(&kp_bits, &kp, sizeof(float));
        WriteOpenRegisters("Kp_impedance", kp_bits);
        // kd
        uint32_t kd_bits;
        memcpy(&kd_bits, &kd, sizeof(float));
        WriteOpenRegisters("Kd_impedance", kd_bits);
        WriteOpenRegisters("Target Torque", torque);
        usleep(5000000);
        WriteOpenRegisters("Target Torque", 0);
        WriteOpenRegisters("Controlword", 0x80);
        WriteOpenRegisters("Controlword", 0x06);
        WriteOpenRegisters("Modes Of Operation", 0);
        WriteOpenRegisters("Controlword", 15);
        return MDCO::Error_t::OK;
    }

    MDCO::Error_t MDCO::blinkOpenTest()
    {
        WriteOpenRegisters("Controlword", 0x06, 2);
        WriteOpenRegisters("Modes Of Operation", 0xFE, 1);
        WriteOpenRegisters("Blink LEDs", 1, 1);
        return MDCO::Error_t::OK;
    }

    MDCO::Error_t MDCO::OpenReset()
    {
        WriteOpenRegisters("Controlword", 0x06, 2);
        WriteOpenRegisters("Modes Of Operation", 0xFE, 1);
        WriteOpenRegisters("Reset Controller", 1, 1);
        return MDCO::Error_t::OK;
    }

    MDCO::Error_t MDCO::clearOpenErrors(int level)
    {
        // restart node
        std::vector<u8> data;
        data.push_back(0x81);
        data.push_back(m_canId);
        WriteOpenPDORegisters(0x000, data);
        m_log.debug("waiting the node %d to restart\n", m_canId);
        // wait for the node to restart
        usleep(5000000);
        // clearing error register
        WriteOpenRegisters("Controlword", 0x06, 2);
        WriteOpenRegisters("Modes Of Operation", 0xFF, 1);
        usleep(100000);
        if (level == 1)
            return WriteOpenRegisters("Clear Errors", 1, 1);
        if (level == 2)
            return WriteOpenRegisters("Clear Warnings", 1, 1);
        if (level == 3)
        {
            if (!WriteOpenRegisters("Clear Errors", 1, 1))
            {
                return WriteOpenRegisters("Clear Errors", 1, 1);
            }
            else
                return MDCO::Error_t::TRANSFER_FAILED;
        }
        else
            return MDCO::Error_t::REQUEST_INVALID;
    }

    MDCO::Error_t MDCO::newCanOpenConfig(long newID, long newBaud, int newwatchdog)
    {
        WriteOpenRegisters("Can ID", newID, 4);
        WriteOpenRegisters("Can Baudrate", newBaud, 4);
        if (newwatchdog != 0)
        {
            WriteOpenRegisters("Can Watchdog", newwatchdog, 2);
        }
        return MDCO::Error_t::OK;
    }

    MDCO::Error_t MDCO::CanOpenBandwidth(int newBandwidth)
    {
        WriteOpenRegisters("Torque Bandwidth", newBandwidth, 2);
        return MDCO::Error_t::OK;
    }

    MDCO::Error_t MDCO::openSave()
    {
        WriteOpenRegisters("Controlword", 0x06, 2);
        WriteOpenRegisters("Save all parameters", 0x65766173, 4);
        return MDCO::Error_t::OK;
    }

    MDCO::Error_t MDCO::openZero()
    {
        WriteOpenRegisters("Controlword", 0x06, 2);
        WriteOpenRegisters("Modes Of Operation", 0xFE, 1);
        WriteOpenRegisters("Set Zero", 1, 1);

        if ((GetValueFromOpenRegister(0x6064, 0) > -50) &&
            (GetValueFromOpenRegister(0x6064, 0) < 50))
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

    MDCO::Error_t MDCO::testencoder(bool Main, bool output)
    {
        WriteOpenRegisters("Controlword", 0x06, 2);
        WriteOpenRegisters("Modes Of Operation", 0xFE, 1);
        if (Main)
            WriteOpenRegisters("Test Main Encoder", 1, 1);
        if (output)
            WriteOpenRegisters("Test Output Encoder", 1, 1);
        return MDCO::Error_t::OK;
    }

    MDCO::Error_t MDCO::encoderCalibration(bool Main, bool output)
    {
        WriteOpenRegisters("Controlword", 0x06, 2);
        WriteOpenRegisters("Modes Of Operation", 0xFE, 1);
        if (Main)
            WriteOpenRegisters("Run Calibration", 1, 1);
        if (output)
            WriteOpenRegisters("Run Output Encoder Calibration", 1, 1);
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
