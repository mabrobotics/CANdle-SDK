#include "MDCO.hpp"

namespace mab
{
    void MDCO::movePositionAcc(i32 targetPos,
                               i32 accLimit,
                               i32 dccLimit,
                               u32 MaxSpeed,
                               u16 MaxCurrent,
                               u32 RatedCurrent,
                               u16 MaxTorque,
                               u32 RatedTorque)
    {
        bool debug = m_log.isLevelEnabled(Logger::LogLevel_E::DEBUG);
        usleep(1000);
        // Max acceleration + Max Deceleration
        WriteOpenRegisters(0x60C5, 0, 10000, 4);
        if (debug)
            m_log.debug("Error:%d\n", ReadOpenRegisters(0x60C5, 0x00));
        WriteOpenRegisters(0x60C6, 0, 10000, 4);
        if (debug)
            m_log.debug("Error:%d\n", ReadOpenRegisters(0x60C6, 0x00));

        // profile acceleration + profile Deceleration
        WriteOpenRegisters(0x6083, 0, accLimit, 4);
        if (debug)
            m_log.debug("Error:%d\n", ReadOpenRegisters(0x6083, 0x00));
        WriteOpenRegisters(0x6084, 0, dccLimit, 4);
        if (debug)
            m_log.debug("Error:%d\n", ReadOpenRegisters(0x6084, 0x00));

        // Current Max + rated
        WriteOpenRegisters(0x6073, 0, MaxCurrent, 2);
        if (debug)
            m_log.debug("Error:%d\n", ReadOpenRegisters(0x6073, 0x00));
        WriteOpenRegisters(0x6075, 0, RatedCurrent, 4);
        if (debug)
            m_log.debug("Error:%d\n", ReadOpenRegisters(0x6075, 0x00));

        // Motor Max Speed
        WriteOpenRegisters(0x6080, 0, MaxSpeed, 4);
        if (debug)
            m_log.debug("Error:%d\n", ReadOpenRegisters(0x6080, 0x00));

        // Torques Max + rated
        WriteOpenRegisters(0x6072, 0, MaxTorque, 2);
        if (debug)
            m_log.debug("Error:%d\n", ReadOpenRegisters(0x6072, 0x00));
        WriteOpenRegisters(0x6076, 0, RatedTorque, 4);
        if (debug)
            m_log.debug("Error:%d\n", ReadOpenRegisters(0x6076, 0x00));

        // cyclic sync velocity
        WriteOpenRegisters(0x6060, 0, 1, 1);
        if (debug)
            m_log.debug("Error:%d\n", ReadOpenRegisters(0x6060, 0x00));

        // clear error
        WriteOpenRegisters(0x6040, 0, 0x80, 2);
        if (debug)
            m_log.debug("Error:%d\n", ReadOpenRegisters(0x6040, 0x00));

        // shutdown command
        WriteOpenRegisters(0x6040, 0, 6, 2);
        if (debug)
            m_log.debug("Error:%d\n", ReadOpenRegisters(0x6041, 0x00));

        // operation enable command
        WriteOpenRegisters(0x6040, 0, 15, 2);
        if (debug)
            m_log.debug("Error:%d\n", ReadOpenRegisters(0x6041, 0x00));

        m_log.debug("position asked : %d\n", targetPos);

        time_t start = time(nullptr);

        while (time(nullptr) - start < 5 &&
               !((int)GetValueFromOpenRegister(ODList[56].index, 0) > (targetPos - 100) &&
                 (int)GetValueFromOpenRegister(ODList[56].index, 0) < (targetPos + 100)))
        {
            WriteOpenRegisters(0x607A, 0, targetPos, 4);
            if (debug)
                m_log.debug("Error:%d\n", ReadOpenRegisters(ODList[56].index, 0));
            usleep(10000);
        }

        m_log.debug("actual position: %d\n", (int)GetValueFromOpenRegister(ODList[56].index, 0));

        if (((int)GetValueFromOpenRegister(ODList[56].index, 0) > (targetPos - 200) &&
             (int)GetValueFromOpenRegister(ODList[56].index, 0) < (targetPos + 200)))
        {
            m_log.success("Position reached in less than 5s");
        }
        else
        {
            m_log.error("Position not reached in less than 5s");
        }

        m_log.debug("actual position: %d\n", (int)GetValueFromOpenRegister(ODList[56].index, 0));

        // shutdown command
        WriteOpenRegisters(0x6040, 0, 6, 2);
        if (debug)
            m_log.debug("Error:%d\n", ReadOpenRegisters(0x6041, 0x00));

        // idle
        WriteOpenRegisters(0x6060, 0, 0, 1);
        if (debug)
            m_log.debug("Error:%d\n", ReadOpenRegisters(0x6061, 0x00));
        if (debug)
            m_log.debug("Error:%d\n", ReadOpenRegisters(ODList[56].index, 0));
    }

    void MDCO::movePosition(u16 MaxCurrent,
                            u32 RatedCurrent,
                            u16 MaxTorque,
                            u32 RatedTorque,
                            u32 MaxSpeed,
                            i32 DesiredPos)
    {
        bool debug = m_log.isLevelEnabled(Logger::LogLevel_E::DEBUG);
        usleep(1000);
        // Current Max + rated
        WriteOpenRegisters(0x6073, 0, MaxCurrent, 2);
        if (debug)
            m_log.debug("Error:%d\n", ReadOpenRegisters(0x6073, 0x00));
        WriteOpenRegisters(0x6075, 0, RatedCurrent, 4);
        if (debug)
            m_log.debug("Error:%d\n", ReadOpenRegisters(0x6075, 0x00));
        // Motor Max Speed
        WriteOpenRegisters(0x6080, 0, MaxSpeed, 4);
        if (debug)
            m_log.debug("Error:%d\n", ReadOpenRegisters(0x6080, 0x00));
        // Torques Max + rated
        WriteOpenRegisters(0x6072, 0, MaxTorque, 2);
        if (debug)
            m_log.debug("Error:%d\n", ReadOpenRegisters(0x6072, 0x00));
        WriteOpenRegisters(0x6076, 0, RatedTorque, 4);
        if (debug)
            m_log.debug("Error:%d\n", ReadOpenRegisters(0x6076, 0x00));
        // cyclic sync velocity
        WriteOpenRegisters(0x6060, 0, 8, 1);
        if (debug)
            m_log.debug("Error:%d\n", ReadOpenRegisters(0x6060, 0x00));
        // clear error
        WriteOpenRegisters(0x6040, 0, 0x80, 2);
        if (debug)
            m_log.debug("Error:%d\n", ReadOpenRegisters(0x6040, 0x00));
        // shutdown command
        WriteOpenRegisters(0x6040, 0, 6, 2);
        if (debug)
            m_log.debug("Error:%d\n", ReadOpenRegisters(0x6040, 0x00));
        // operation enable command
        WriteOpenRegisters(0x6040, 0, 15, 2);
        if (debug)
            m_log.debug("Error:%d\n", ReadOpenRegisters(0x6040, 0x00));

        m_log.debug("position asked : %d\n", DesiredPos);

        time_t start = time(nullptr);

        while (time(nullptr) - start < 5 &&
               !((int)GetValueFromOpenRegister(ODList[56].index, 0) > (DesiredPos - 100) &&
                 (int)GetValueFromOpenRegister(ODList[56].index, 0) < (DesiredPos + 100)))
        {
            //  target velocity
            WriteOpenRegisters(0x607A, 0, DesiredPos, 4);
            if (debug)
                m_log.debug("Error:%d\n", ReadOpenRegisters(0x607A, 0x00));
            usleep(10000);
        }

        m_log.debug("actual position: %d\n", (int)GetValueFromOpenRegister(ODList[56].index, 0));

        if (((int)GetValueFromOpenRegister(ODList[56].index, 0) > (DesiredPos - 200) &&
             (int)GetValueFromOpenRegister(ODList[56].index, 0) < (DesiredPos + 200)))
        {
            m_log.success("Position reached in less than 5s");
        }
        else
        {
            m_log.error("Position not reached in less than 5s");
        }

        // shutdown command
        WriteOpenRegisters(0x6040, 0, 6, 2);
        if (debug)
            m_log.debug("Error:%d\n", ReadOpenRegisters(0x6040, 0x00));

        // idle
        WriteOpenRegisters(0x6060, 0, 0, 1);
        if (debug)
            m_log.debug("Error:%d\n", ReadOpenRegisters(0x6060, 0x00));
    }

    void MDCO::moveSpeed(u16 MaxCurrent,
                         u32 RatedCurrent,
                         u16 MaxTorque,
                         u32 RatedTorque,
                         u32 MaxSpeed,
                         i32 DesiredSpeed)
    {
        bool debug = m_log.isLevelEnabled(Logger::LogLevel_E::DEBUG);

        usleep(1000);
        // Current Max + rated
        WriteOpenRegisters(0x6073, 0, MaxCurrent, 2);
        if (debug)
            m_log.debug("Error:%d\n", ReadOpenRegisters(0x6073, 0x00));

        WriteOpenRegisters(0x6075, 0, RatedCurrent, 4);
        if (debug)
            m_log.debug("Error:%d\n", ReadOpenRegisters(0x6075, 0x00));

        // Motor Max Speed
        WriteOpenRegisters(0x6080, 0, MaxSpeed, 4);
        if (debug)
            m_log.debug("Error:%d\n", ReadOpenRegisters(0x6080, 0x00));

        // Torques Max + rated
        WriteOpenRegisters(0x6072, 0, MaxTorque, 2);
        if (debug)
            m_log.debug("Error:%d\n", ReadOpenRegisters(0x6072, 0x00));
        WriteOpenRegisters(0x6076, 0, RatedTorque, 4);
        if (debug)
            m_log.debug("Error:%d\n", ReadOpenRegisters(0x6076, 0x00));

        // cyclic sync velocity
        WriteOpenRegisters(0x6060, 0, 9, 1);
        if (debug)
            m_log.debug("Error:%d\n", ReadOpenRegisters(0x6060, 0x00));

        // clear error
        WriteOpenRegisters(0x6040, 0, 0x80, 2);
        if (debug)
            m_log.debug("Error:%d\n", ReadOpenRegisters(0x6040, 0x00));

        // shutdown command
        WriteOpenRegisters(0x6040, 0, 6, 2);
        if (debug)
            m_log.debug("Error:%d\n", ReadOpenRegisters(0x6040, 0x00));

        // operation enable command
        WriteOpenRegisters(0x6040, 0, 15, 2);
        if (debug)
            m_log.debug("Error:%d\n", ReadOpenRegisters(0x6040, 0x00));

        usleep(10000);

        time_t start = time(nullptr);
        while (time(nullptr) - start < 5)
        {
            // target velocity
            WriteOpenRegisters(0x60FF, 0, DesiredSpeed, 4);
            if (debug)
                m_log.debug("Error:%d\n", ReadOpenRegisters(0x606c, 0x00));
            usleep(10000);
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
        WriteOpenRegisters(0x60FF, 0, 0, 4);
        if (debug)
            m_log.debug("Error:%d\n", ReadOpenRegisters(0x60FF, 0x00));
        usleep(100000);

        // shutdown command
        WriteOpenRegisters(0x6040, 0, 6, 2);
        if (debug)
            m_log.debug("Error:%d\n", ReadOpenRegisters(0x6040, 0x00));

        // idle
        WriteOpenRegisters(0x6060, 0, 0, 1);
        if (debug)
            m_log.debug("Error:%d\n", ReadOpenRegisters(0x6060, 0x00));
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
        bool debug = m_log.isLevelEnabled(Logger::LogLevel_E::DEBUG);
        usleep(1000);
        // Current Max + rated
        WriteOpenRegisters(0x6073, 0, MaxCurrent, 2);
        if (debug)
            m_log.debug("Error:%d\n", ReadOpenRegisters(0x6073, 0x00));
        WriteOpenRegisters(0x6075, 0, RatedCurrent, 4);
        if (debug)
            m_log.debug("Error:%d\n", ReadOpenRegisters(0x6075, 0x00));

        // Motor Max Speed
        WriteOpenRegisters(0x6080, 0, MaxSpeed, 4);
        if (debug)
            m_log.debug("Error:%d\n", ReadOpenRegisters(0x6080, 0x00));

        // Torques Max + rated
        WriteOpenRegisters(0x6072, 0, MaxTorque, 2);
        if (debug)
            m_log.debug("Error:%d\n", ReadOpenRegisters(0x6072, 0x00));
        WriteOpenRegisters(0x6076, 0, RatedTorque, 4);
        if (debug)
            m_log.debug("Error:%d\n", ReadOpenRegisters(0x6076, 0x00));

        // cyclic sync velocity
        WriteOpenRegisters(0x6060, 0, 0xFD, 1);
        if (debug)
            m_log.debug("Error:%d\n", ReadOpenRegisters(0x6060, 0x00));

        // clear error
        WriteOpenRegisters(0x6040, 0, 0x80, 2);
        if (debug)
            m_log.debug("Error:%d\n", ReadOpenRegisters(0x6040, 0x00));

        // shutdown command
        WriteOpenRegisters(0x6040, 0, 6, 2);
        if (debug)
            m_log.debug("Error:%d\n", ReadOpenRegisters(0x6040, 0x00));

        // operation enable command
        WriteOpenRegisters(0x6040, 0, 15, 2);
        if (debug)
            m_log.debug("Error:%d\n", ReadOpenRegisters(0x6040, 0x00));

        // desired speed
        WriteOpenRegisters(0x606B, 0x00, desiredSpeed, 4);
        if (debug)
            m_log.debug("Error:%d\n", ReadOpenRegisters(0x606B, 0x00));
        // desired position
        WriteOpenRegisters(0x6062, 0x00, targetPos, 4);
        if (debug)
            m_log.debug("Error:%d\n", ReadOpenRegisters(0x6062, 0x00));
        // kp
        uint32_t kp_bits;
        memcpy(&kp_bits, &kp, sizeof(float));
        WriteOpenRegisters(0x200C, 0x01, kp_bits, 4);
        if (debug)
            m_log.debug("Error:%d\n", ReadOpenRegisters(0x200C, 0x01));
        // ReadOpenRegisters(0x200C, 01);

        // kd
        uint32_t kd_bits;
        memcpy(&kd_bits, &kd, sizeof(float));
        WriteOpenRegisters(0x200C, 0x02, kd_bits, 4);
        if (debug)
            m_log.debug("Error:%d\n", ReadOpenRegisters(0x200C, 0x02));
        // ReadOpenRegisters(0x200C, 2);

        // torque
        WriteOpenRegisters(0x6074, 0x00, torque, 2);
        if (debug)
            m_log.debug("Error:%d\n", ReadOpenRegisters(0x6074, 0x00));

        usleep(5000000);
        WriteOpenRegisters(0x6074, 0x00, 0, 2);
        if (debug)
            m_log.debug("Error:%d\n", ReadOpenRegisters(0x6074, 0x00));

        // clear error
        WriteOpenRegisters(0x6040, 0, 0x80, 2);
        if (debug)
            m_log.debug("Error:%d\n", ReadOpenRegisters(0x6040, 0x00));

        // shutdown command
        WriteOpenRegisters(0x6040, 0, 6, 2);
        if (debug)
            m_log.debug("Error:%d\n", ReadOpenRegisters(0x6040, 0x00));

        // idle
        WriteOpenRegisters(0x6060, 0, 0, 1);
        if (debug)
            m_log.debug("Error:%d\n", ReadOpenRegisters(0x6060, 0x00));

        // operation enable command
        WriteOpenRegisters(0x6040, 0, 15, 2);
        if (debug)
            m_log.debug("Error:%d\n", ReadOpenRegisters(0x6040, 0x00));

        return MDCO::Error_t::OK;
    }

    MDCO::Error_t MDCO::blinkOpenTest()
    {
        bool debug = m_log.isLevelEnabled(Logger::LogLevel_E::DEBUG);
        WriteOpenRegisters(0x6040, 0, 6, 2);
        if (debug)
            m_log.debug("Error:%d\n", ReadOpenRegisters(0x6040, 0x00));
        WriteOpenRegisters(0x6060, 0, 0xFE, 1);
        if (debug)
            m_log.debug("Error:%d\n", ReadOpenRegisters(0x6060, 0x00));
        WriteOpenRegisters(0x2003, 1, 1, 1);
        if (debug)
            m_log.debug("Error:%d\n", ReadOpenRegisters(0x2003, 0x00));

        return MDCO::Error_t::OK;
    }

    MDCO::Error_t MDCO::OpenReset()
    {
        bool debug = m_log.isLevelEnabled(Logger::LogLevel_E::DEBUG);
        WriteOpenRegisters(0x6040, 0, 6, 2);
        if (debug)
            m_log.debug("Error:%d\n", ReadOpenRegisters(0x6041, 0x00));
        WriteOpenRegisters(0x6060, 0, 0xFE, 1);
        if (debug)
            m_log.debug("Error:%d\n", ReadOpenRegisters(0x6061, 0x00));
        WriteOpenRegisters(0x2003, 2, 1, 1);
        if (debug)
            m_log.debug("Error:%d\n", ReadOpenRegisters(0x2003, 0x00));
        return MDCO::Error_t::OK;
    }

    MDCO::Error_t MDCO::clearOpenErrors(int level)
    {
        bool debug = m_log.isLevelEnabled(Logger::LogLevel_E::DEBUG);
        // restart node
        std::vector<u8> data;
        data.push_back(0x81);
        data.push_back(m_canId);
        WriteOpenPDORegisters(0x000, data);
        m_log.debug("waiting the node %d to restart\n", m_canId);
        // wait for the node to restart
        usleep(5000000);
        // clearing error register
        WriteOpenRegisters(0x6040, 0, 6, 2);
        if (debug)
            m_log.debug("Error:%d\n", ReadOpenRegisters(0x6041, 0x00));
        WriteOpenRegisters(0x6060, 0, 0xFF, 1);
        if (debug)
            m_log.debug("Error:%d\n", ReadOpenRegisters(0x6061, 0x00));
        usleep(100000);
        if (level == 1)
            return WriteOpenRegisters(0x2003, 0xb, 1, 1);
        if (level == 2)
            return WriteOpenRegisters(0x2003, 0xc, 1, 1);
        if (level == 3)
        {
            if (!WriteOpenRegisters(0x2003, 0xb, 1, 1))
            {
                return WriteOpenRegisters(0x2003, 0xb, 1, 1);
            }
            else
                return MDCO::Error_t::TRANSFER_FAILED;
        }
        else
            return MDCO::Error_t::REQUEST_INVALID;
    }

    MDCO::Error_t MDCO::newCanOpenConfig(long newID, long newBaud, int newwatchdog)
    {
        bool debug = m_log.isLevelEnabled(Logger::LogLevel_E::DEBUG);
        WriteOpenRegisters(0x2000, 0x0B, newBaud, 4);
        if (debug)
            m_log.debug("Error:%d\n", ReadOpenRegisters(0x2000, 0x0B));
        WriteOpenRegisters(0x2000, 0x0A, newID, 4);
        if (debug)
            m_log.debug("Error:%d\n", ReadOpenRegisters(0x2000, 0x0A));

        if (newwatchdog != 0)
        {
            WriteOpenRegisters(0x2000, 0x0C, newwatchdog, 2);
            if (debug)
                m_log.debug("Error:%d\n", ReadOpenRegisters(0x2000, 0x0C));
        }
        if (debug)
            m_log.debug("Error:%d\n", ReadOpenRegisters(0x2000, 0x0A));
        if (debug)
            m_log.debug("Error:%d\n", ReadOpenRegisters(0x2000, 0x0B));
        if (debug)
            m_log.debug("Error:%d\n", ReadOpenRegisters(0x2000, 0x0C));
        return MDCO::Error_t::OK;
    }

    MDCO::Error_t MDCO::CanOpenBandwidth(int newBandwidth)
    {
        bool debug = m_log.isLevelEnabled(Logger::LogLevel_E::DEBUG);
        if (debug)
            m_log.debug("Error:%d\n", ReadOpenRegisters(0x2000, 0x05));
        WriteOpenRegisters(0x2000, 0x05, newBandwidth, 2);
        if (debug)
            m_log.debug("Error:%d\n", ReadOpenRegisters(0x2000, 0x05));
        return MDCO::Error_t::OK;
    }

    MDCO::Error_t MDCO::openSave()
    {
        bool debug = m_log.isLevelEnabled(Logger::LogLevel_E::DEBUG);
        WriteOpenRegisters(0x6040, 0, 6, 2);
        if (debug)
            m_log.debug("Error:%d\n", ReadOpenRegisters(0x6041, 0x00));
        WriteOpenRegisters(0X1010, 1, 0x65766173, 4);
        m_log.debug("New values are save");
        return MDCO::Error_t::OK;
    }

    MDCO::Error_t MDCO::openZero()
    {
        bool debug = m_log.isLevelEnabled(Logger::LogLevel_E::DEBUG);
        WriteOpenRegisters(0x6040, 0, 6, 2);
        if (debug)
            m_log.debug("Error:%d\n", ReadOpenRegisters(0x6041, 0x00));
        WriteOpenRegisters(0x6060, 0, 0xFE, 1);
        if (debug)
            m_log.debug("Error:%d\n", ReadOpenRegisters(0x6061, 0x00));
        WriteOpenRegisters(0x2003, 5, 1, 1);

        if ((GetValueFromOpenRegister(ODList[56].index, 0) > -50) &&
            (GetValueFromOpenRegister(ODList[56].index, 0) < 50))
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
        WriteOpenRegisters(0x6040, 0, 6, 2);
        WriteOpenRegisters(0x6060, 0, 0xFE, 1);
        if (Main)
            WriteOpenRegisters(0x2003, 8, 1, 1);
        if (output)
            WriteOpenRegisters(0x2003, 7, 1, 1);
        return MDCO::Error_t::OK;
    }

    MDCO::Error_t MDCO::encoderCalibration(bool Main, bool output)
    {
        WriteOpenRegisters(0x6040, 0, 6, 2);
        WriteOpenRegisters(0x6060, 0, 0xFE, 1);
        if (Main)
            WriteOpenRegisters(0x2003, 3, 1, 1);
        if (output)
            WriteOpenRegisters(0x2003, 4, 1, 1);
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

            if (response[6] == 0x92)
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
