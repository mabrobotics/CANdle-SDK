#include "candletoolCO.hpp"

#include <algorithm>
#include <any>
#include <cstdint>
#include <numeric>
#include <unistd.h>
#include <string>
#include <array>
#include <type_traits>
#include <variant>
#include <vector>

#include "MDStatus.hpp"
#include "canLoader.hpp"
#include "mab_types.hpp"
#include "utilities.hpp"
#include "mabFileParser.hpp"
#include "candle_bootloader.hpp"
#include "I_communication_interface.hpp"
#include "mab_crc.hpp"

using namespace mab;

CandleToolCO::CandleToolCO(const mab::CANdleBaudrate_E baud)
{
    log.m_tag   = "CANDLETOOLco";
    log.m_layer = Logger::ProgramLayer_E::TOP;

    std::unique_ptr<I_CommunicationInterface> bus;

    mINI::INIFile      file(getCandletoolConfigPath());
    mINI::INIStructure ini;
    file.read(ini);

    std::string& device = ini["communication"]["device"];
    busString           = ini["communication"]["bus"];

    bus = std::make_unique<USB>(Candle::CANDLE_VID, Candle::CANDLE_PID, device);

    m_candle = attachCandle(baud, std::move(bus));
}

CandleToolCO::~CandleToolCO()
{
    detachCandle(m_candle);
}

void CandleToolCO::ping(const std::string& variant)
{
    if (variant == "all")
    {
        log.error("Not implemented");
        return;
    }
    std::vector<canId_t> mdIds;
    mdIds = MDCO::discoverOpenMDs(m_candle);
    log.info("Discovered MDs: ");
    for (const auto& id : mdIds)
    {
        log.info("- %d", id);
    }
}

void CandleToolCO::configCan(
    u16 id, u16 newId, const std::string& baud, u16 timeout, bool termination)
{
    MDCO mdco = MDCO(id, m_candle);
    long newbaud;
    if (baud == "1M")
        newbaud = 1000000;
    else if (baud == "500K")
        newbaud = 500000;
    else
    {
        log.error("Invalid baudrate for CANopen, only 1M and 500K is supported");
        return;
    }
    MDCO::Error_t err = mdco.newCanOpenConfig(newId, newbaud, termination);
    if (err != MDCO::OK)
    {
        log.error("Error setting CANopen config");
        return;
    }
}

void CandleToolCO::configSave(u16 id)
{
    MDCO          mdco = MDCO(id, m_candle);
    MDCO::Error_t err  = mdco.openSave();
    if (err != MDCO::OK)
    {
        log.error("Error saving config");
        return;
    }
}

void CandleToolCO::configZero(u16 id)
{
    MDCO          mdco = MDCO(id, m_candle);
    MDCO::Error_t err  = mdco.openZero();
    if (err != MDCO::OK)
    {
        log.error("Error zeroing");
        return;
    }
}

void CandleToolCO::configBandwidth(u16 id, f32 bandwidth)
{
    MDCO          mdco         = MDCO(id, m_candle);
    u16           newBandwidth = ((u16)bandwidth);
    MDCO::Error_t err          = mdco.canOpenBandwidth(newBandwidth);
    if (err != MDCO::OK)
    {
        log.error("Error setting bandwidth");
        return;
    }
}

void CandleToolCO::sendPdoSpeed(u16 id, i32 desiredSpeed)
{
    MDCO md = MDCO(id, m_candle);
    log.info("Sending SDO for motor setup!");

    moveParameter param;
    param.MaxCurrent   = 500;
    param.MaxTorque    = 500;
    param.RatedTorque  = 1000;
    param.RatedCurrent = 1000;
    param.MaxSpeed     = 200;
    MDCO::Error_t err  = md.setProfileParameters(param);
    if (err != MDCO::OK)
    {
        log.error("Error setting profile parameters");
        return;
    }
    err = md.enableDriver(CyclicSyncVelocity);
    if (err != MDCO::OK)
    {
        log.error("Error enabling driver");
        return;
    }

    log.info("Sending PDO for speed loop control");
    std::vector<u8> frameSetup = {0x0F, 0x00, 0x09};
    md.writeOpenPDORegisters(0x300 + id, frameSetup);

    std::vector<u8> frameSpeed = {0x0F,
                                  0x00,
                                  (u8)(desiredSpeed),
                                  (u8)(desiredSpeed >> 8),
                                  (u8)(desiredSpeed >> 16),
                                  (u8)(desiredSpeed >> 24)};
    md.writeOpenPDORegisters(0x500 + id, frameSpeed);
    auto start   = std::chrono::steady_clock::now();
    auto timeout = std::chrono::seconds((5));
    while (std::chrono::steady_clock::now() - start < timeout)
    {
    }
    if ((int)md.getValueFromOpenRegister(0x606C, 0x00) <= desiredSpeed + 5 &&
        (int)md.getValueFromOpenRegister(0x606C, 0x00) >= desiredSpeed - 5)
    {
        log.success("Velocity Target reached with +/- 5RPM");
    }
    else
    {
        log.error("Velocity Target not reached");
    }
    err = md.disableDriver();
    if (err != MDCO::OK)
    {
        log.error("Error disabling driver");
        return;
    }
}

void CandleToolCO::sendPdoPosition(u16 id, i32 DesiredPos)
{
    MDCO md = MDCO(id, m_candle);
    log.info("Sending SDO for motor setup!");
    moveParameter param;
    param.MaxCurrent   = 500;
    param.MaxTorque    = 500;
    param.RatedTorque  = 1000;
    param.RatedCurrent = 1000;
    param.MaxSpeed     = 200;
    MDCO::Error_t err  = md.setProfileParameters(param);
    if (err != MDCO::OK)
    {
        log.error("Error setting profile parameters");
        return;
    }
    err = md.enableDriver(CyclicSyncPosition);
    if (err != MDCO::OK)
    {
        log.error("Error enabling driver");
        return;
    }

    auto start   = std::chrono::steady_clock::now();
    auto timeout = std::chrono::seconds((1));

    log.info("Sending PDO for speed loop control");

    std::vector<u8> frameSetup = {0x0F, 0x00, 0x08};
    md.writeOpenPDORegisters(0x300 + id, frameSetup);
    std::vector<u8> framePosition = {0x0F,
                                     0x00,
                                     (u8)(DesiredPos),
                                     (u8)(DesiredPos >> 8),
                                     (u8)(DesiredPos >> 16),
                                     (u8)(DesiredPos >> 24)};
    md.writeOpenPDORegisters(0x400 + id, framePosition);

    log.debug("position ask : %d\n", DesiredPos);

    start           = std::chrono::steady_clock::now();
    auto lastSend   = start;
    timeout         = std::chrono::seconds(5);
    auto sendPeriod = std::chrono::milliseconds(100);

    while (std::chrono::steady_clock::now() - start < timeout &&
           !((int)md.getValueFromOpenRegister(0x6064, 0) > (DesiredPos - 100) &&
             (int)md.getValueFromOpenRegister(0x6064, 0) < (DesiredPos + 100)))
    {
        auto now = std::chrono::steady_clock::now();
        if (now - lastSend >= sendPeriod)
        {
            MDCO::Error_t err = md.writeOpenRegisters("Motor Target Position", DesiredPos, 4);
            if (err != MDCO::OK)
            {
                log.error("Error setting Motor Target Position");
                return;
            }
            lastSend = now;
        }
    }

    log.debug("position actual : %d\n", (int)md.getValueFromOpenRegister(0x6064, 0));

    if (((int)md.getValueFromOpenRegister(0x6064, 0) > (DesiredPos - 200) &&
         (int)md.getValueFromOpenRegister(0x6064, 0) < (DesiredPos + 200)))
    {
        log.success("Position reached in less than 5s");
    }
    else
    {
        log.error("Position not reached in less than 5s");
    }

    err = md.disableDriver();
    if (err != MDCO::OK)
    {
        log.error("Error disabling driver");
        return;
    }
}

void CandleToolCO::SendCustomPdo(u16 id, const edsObject& Desiregister, u64 data)
{
    MDCO          mdco = MDCO(id, m_candle);
    MDCO::Error_t err;
    if (Desiregister.index == (0x1600))
    {
        std::vector<u8> frame = {(u8)(data >> 8), (u8)(data)};
        err                   = mdco.writeOpenPDORegisters(0x200 + id, frame);
        if (err != MDCO::OK)
        {
            log.error("Error sending custom PDO 0x1600");
            return;
        }
    }
    else if (Desiregister.index == (0x1601))
    {
        std::vector<u8> frame = {(u8)(data >> 16), (u8)(data >> 8), (u8)(data)};
        err                   = mdco.writeOpenPDORegisters(0x300 + id, frame);
        if (err != MDCO::OK)
        {
            log.error("Error sending custom PDO 0x1601");
            return;
        }
    }
    else if (Desiregister.index == (0x1602))
    {
        std::vector<u8> frame = {(u8)(data >> 40),
                                 (u8)(data >> 32),
                                 (u8)(data >> 24),
                                 (u8)(data >> 16),
                                 (u8)(data >> 8),
                                 (u8)(data)};
        err                   = mdco.writeOpenPDORegisters(0x400 + id, frame);
        if (err != MDCO::OK)
        {
            log.error("Error sending custom PDO 0x1602");
            return;
        }
    }
    else if (Desiregister.index == (0x1603))
    {
        std::vector<u8> frame = {(u8)(data >> 40),
                                 (u8)(data >> 32),
                                 (u8)(data >> 24),
                                 (u8)(data >> 16),
                                 (u8)(data >> 8),
                                 (u8)(data)};
        err                   = mdco.writeOpenPDORegisters(0x500 + id, frame);
        if (err != MDCO::OK)
        {
            log.error("Error sending custom PDO 0x1603");
            return;
        }
    }
    else
    {
        log.error("Please enter a index between 0x1600 & 0x1603 (Transmit PDO)");
        return;
    }
}

void CandleToolCO::setupCalibration(u16 id)
{
    MDCO          mdco = MDCO(id, m_candle);
    MDCO::Error_t err  = mdco.encoderCalibration(1, 0);
    if (err != MDCO::OK)
    {
        log.error("Error running main encoder calibration");
        return;
    }
}

void CandleToolCO::setupCalibrationOutput(u16 id)
{
    MDCO          mdco = MDCO(id, m_candle);
    MDCO::Error_t err  = mdco.encoderCalibration(0, 1);
    if (err != MDCO::OK)
    {
        log.error("Error running output encoder calibration");
        return;
    }
}

std::string CandleToolCO::validateAndGetFinalConfigPath(const std::string& cfgPath)
{
    std::string finalConfigPath        = cfgPath;
    std::string pathRelToDefaultConfig = getMotorsConfigPath() + cfgPath;
    if (!fileExists(finalConfigPath))
    {
        if (!fileExists(pathRelToDefaultConfig))
        {
            log.error("Neither \"%s\", nor \"%s\", exists!.",
                      cfgPath.c_str(),
                      pathRelToDefaultConfig.c_str());
            exit(1);
        }
        finalConfigPath = pathRelToDefaultConfig;
    }

    if (!isConfigValid(finalConfigPath))
    {
        log.error("\"%s\" in not a valid motor .cfg file.", finalConfigPath.c_str());
        log.warn("Valid file must have .cfg extension, and size of < 1MB");
        exit(1);
    }
    std::string defaultConfigPath =
        finalConfigPath.substr(0, finalConfigPath.find_last_of('/') + 1) + "default.cfg";

    if (!fileExists(defaultConfigPath))
    {
        log.warn("No default config found at expected location \"%s\"", defaultConfigPath.c_str());
        log.warn("Cannot check completeness of the config file. Proceed with upload? [y/n]");
        if (!getConfirmation())
            exit(0);
    }

    if (fileExists(defaultConfigPath) && !isCanOpenConfigComplete(finalConfigPath))
    {
        log.m_layer = Logger::ProgramLayer_E::TOP;
        log.error("\"%s\" is incomplete.", finalConfigPath.c_str());
        log.info("Generate updated file with all required fields? [y/n]");
        // TODO: that is so dumb
        //  if (!getConfirmation())
        //      exit(0);
        finalConfigPath = generateUpdatedConfigFile(finalConfigPath);
        log.info("Generated updated file \"%s\"", finalConfigPath.c_str());
        log.info("Setup MD with newly generated config? [y/n]");
        // TODO: that is so dumb, again!
        // if (!getConfirmation())
        //     exit(0);
    }
    return finalConfigPath;
}

void CandleToolCO::setupMotor(u16 id, const std::string& cfgPath, bool force)
{
    std::string finalConfigPath = cfgPath;
    if (!force)
        finalConfigPath = validateAndGetFinalConfigPath(cfgPath);
    else
    {
        log.warn("Omitting config validation on user request!");
        if (!fileExists(finalConfigPath))
        {
            finalConfigPath = getMotorsConfigPath() + cfgPath;
            if (!fileExists(finalConfigPath))
            {
                log.error("Neither \"%s\", nor \"%s\", exists!.",
                          cfgPath.c_str(),
                          finalConfigPath.c_str());
                exit(1);
            }
        }
    }

    log.info("Uploading config from \"%s\"", finalConfigPath.c_str());
    mINI::INIFile      motorCfg(finalConfigPath);
    mINI::INIStructure cfg;
    motorCfg.read(cfg);
    mINI::INIFile      file(getCandletoolConfigPath());
    mINI::INIStructure ini;
    file.read(ini);
    mab::MDCO mdco = MDCO(id, m_candle);

    std::string motor_name            = "";
    int         motor_polepairs       = 0;
    int         motor_kv              = 0;
    float       motor_torqueconstant  = 0.0;
    float       motor_gearratio       = 0.0;
    int         motor_torquebandwidth = 0;
    int         motor_shutdowntemp    = 0;

    int limits_ratedtorque     = 0;
    int limits_maxtorque       = 0;
    int limits_ratedcurrent    = 0;
    int limits_maxcurrent      = 0;
    int limits_maxvelocity     = 0;
    int limits_maxposition     = 0;
    int limits_minposition     = 0;
    int limits_maxacceleration = 0;
    int limits_maxdeceleration = 0;

    int profile_acceleration = 0;
    int profile_deceleration = 0;
    int profile_velocity     = 0;

    int outputencoder_outputencoder     = 0;
    int outputencoder_outputencodermode = 0;

    float positionpid_kp     = 0.0;
    float positionpid_ki     = 0.0;
    float positionpid_kd     = 0.0;
    float positionpid_windup = 0.0;

    float velocitypid_kp     = 0.0;
    float velocitypid_ki     = 0.0;
    float velocitypid_kd     = 0.0;
    float velocitypid_windup = 0.0;

    float impedancepd_kp = 0.0;
    float impedancepd_kd = 0.0;

    std::ifstream infile(finalConfigPath);

    if (!infile.is_open())
    {
        std::cerr << "Error: Unable to open config file\n";
        return;
    }

    std::string section, line;
    while (std::getline(infile, line))
    {
        if (line.empty() || line[0] == '#')
            continue;
        if (line.front() == '[' && line.back() == ']')
        {
            section = line.substr(1, line.size() - 2);
            clean(section);
            continue;
        }
        std::istringstream iss(line);
        std::string        left, right;
        if (!std::getline(iss, left, '='))
            continue;
        if (!std::getline(iss, right))
            continue;
        clean(left);
        clean(right);

        std::string fullkey = section.empty() ? left : section + "_" + left;
        if (fullkey == "motor_name")
            motor_name = right;
        else if (fullkey == "motor_polepairs")
            motor_polepairs = std::stoi(right);
        else if (fullkey == "motor_kv")
            motor_kv = std::stoi(right);
        else if (fullkey == "motor_torqueconstant")
            motor_torqueconstant = std::stod(right);
        else if (fullkey == "motor_gearratio")
            motor_gearratio = std::stod(right);
        else if (fullkey == "motor_torquebandwidth")
            motor_torquebandwidth = std::stoi(right);
        else if (fullkey == "motor_shutdowntemp")
            motor_shutdowntemp = std::stoi(right);

        else if (fullkey == "limits_ratedtorque")
            limits_ratedtorque = std::stoi(right);
        else if (fullkey == "limits_maxtorque")
            limits_maxtorque = std::stoi(right);
        else if (fullkey == "limits_ratedcurrent")
            limits_ratedcurrent = std::stoi(right);
        else if (fullkey == "limits_maxcurrent")
            limits_maxcurrent = std::stoi(right);
        else if (fullkey == "limits_maxvelocity")
            limits_maxvelocity = std::stoi(right);
        else if (fullkey == "limits_maxposition")
            limits_maxposition = std::stoi(right);
        else if (fullkey == "limits_minposition")
            limits_minposition = std::stoi(right);
        else if (fullkey == "limits_maxacceleration")
            limits_maxacceleration = std::stoi(right);

        else if (fullkey == "profile_acceleration")
            profile_acceleration = std::stoi(right);
        else if (fullkey == "profile_deceleration")
            profile_deceleration = std::stoi(right);
        else if (fullkey == "profile_velocity")
            profile_velocity = std::stoi(right);

        else if (fullkey == "outputencoder_outputencoder")
            outputencoder_outputencoder = std::stoi(right);
        else if (fullkey == "outputencoder_outputencodermode")
            outputencoder_outputencodermode = std::stoi(right);

        else if (fullkey == "positionpid_kp")
            positionpid_kp = std::stod(right);
        else if (fullkey == "positionpid_ki")
            positionpid_ki = std::stod(right);
        else if (fullkey == "positionpid_kd")
            positionpid_kd = std::stod(right);
        else if (fullkey == "positionpid_windup")
            positionpid_windup = std::stod(right);

        else if (fullkey == "velocitypid_kp")
            velocitypid_kp = std::stod(right);
        else if (fullkey == "velocitypid_ki")
            velocitypid_ki = std::stod(right);
        else if (fullkey == "velocitypid_kd")
            velocitypid_kd = std::stod(right);
        else if (fullkey == "velocitypid_windup")
            velocitypid_windup = std::stod(right);

        else if (fullkey == "impedancepd_kp")
            impedancepd_kp = std::stod(right);
        else if (fullkey == "impedancepd_kd")
            impedancepd_kd = std::stod(right);
    }
    infile.close();

    std::stringstream ss;

    ss << " ---------- value read from config file ---------- " << '\n';
    ss << "motor_name = " << motor_name << '\n'
       << "motor_polepairs = " << motor_polepairs << '\n'
       << "motor_kv = " << motor_kv << '\n'
       << "motor_torqueconstant = " << motor_torqueconstant << '\n'
       << "motor_gearratio = " << motor_gearratio << '\n'
       << "motor_torquebandwidth = " << motor_torquebandwidth << '\n'
       << "motor_shutdowntemp = " << motor_shutdowntemp << "\n\n"

       << "limits_ratedtorque = " << limits_ratedtorque << '\n'
       << "limits_maxtorque = " << limits_maxtorque << '\n'
       << "limits_ratedcurrent = " << limits_ratedcurrent << '\n'
       << "limits_maxcurrent = " << limits_maxcurrent << '\n'
       << "limits_maxvelocity = " << limits_maxvelocity << '\n'
       << "limits_maxposition = " << limits_maxposition << '\n'
       << "limits_minposition = " << limits_minposition << '\n'
       << "limits_maxacceleration = " << limits_maxacceleration << '\n'
       << "limits_maxdeceleration = " << limits_maxdeceleration << "\n\n"

       << "profile_acceleration = " << profile_acceleration << '\n'
       << "profile_deceleration = " << profile_deceleration << '\n'
       << "profile_velocity = " << profile_velocity << "\n\n"

       << "outputencoder_outputencoder = " << outputencoder_outputencoder << '\n'
       << "outputencoder_outputencodermode = " << outputencoder_outputencodermode << "\n\n"

       << "positionpid_kp = " << positionpid_kp << '\n'
       << "positionpid_ki = " << positionpid_ki << '\n'
       << "positionpid_kd = " << positionpid_kd << '\n'
       << "positionpid_windup = " << positionpid_windup << "\n\n"

       << "velocitypid_kp = " << velocitypid_kp << '\n'
       << "velocitypid_ki = " << velocitypid_ki << '\n'
       << "velocitypid_kd = " << velocitypid_kd << '\n'
       << "velocitypid_windup = " << velocitypid_windup << "\n\n"

       << "impedancepd_kp = " << impedancepd_kp << '\n'
       << "impedancepd_kd = " << impedancepd_kd << "\n\n";

    log.info("%s\n", ss.str().c_str());

    MDCO::Error_t err;
    err = mdco.writeLongOpenRegisters(0x2000, 0x06, motor_name);
    if (err != MDCO::OK)
    {
        log.error("Error setting motor_name");
        return;
    }
    err = mdco.writeOpenRegisters(0x2000, 0x01, motor_polepairs, 4);
    if (err != MDCO::OK)
    {
        log.error("Error setting motor_polepairs");
        return;
    }
    uint32_t motor_torqueconstant_as_long;
    std::memcpy(&motor_torqueconstant_as_long, &motor_torqueconstant, sizeof(float));
    err = mdco.writeOpenRegisters(0x2000, 0x02, motor_torqueconstant_as_long, 4);
    if (err != MDCO::OK)
    {
        log.error("Error setting motor_torqueconstant");
        return;
    }
    uint32_t motor_gearratio_as_long;
    std::memcpy(&motor_gearratio_as_long, &motor_gearratio, sizeof(float));
    err = mdco.writeOpenRegisters(0x2000, 0x08, motor_gearratio_as_long, 4);
    if (err != MDCO::OK)
    {
        log.error("Error setting motor_gearratio");
        return;
    }
    err = mdco.writeOpenRegisters(0x2000, 0x05, motor_torquebandwidth, 2);
    if (err != MDCO::OK)
    {
        log.error("Error setting motor_torquebandwidth");
        return;
    }
    err = mdco.writeOpenRegisters(0x2000, 0x07, motor_shutdowntemp, 1);
    if (err != MDCO::OK)
    {
        log.error("Error setting motor_shutdowntemp");
        return;
    }
    err = mdco.writeOpenRegisters(0x2005, 0x03, outputencoder_outputencodermode, 1);
    if (err != MDCO::OK)
    {
        log.error("Error setting outputencoder_outputencodermode");
        return;
    }
    err = mdco.writeOpenRegisters(0x607D, 0x01, limits_minposition);
    if (err != MDCO::OK)
    {
        log.error("Error setting limits_minposition");
        return;
    }
    err = mdco.writeOpenRegisters(0x607D, 0x02, limits_maxposition);
    if (err != MDCO::OK)
    {
        log.error("Error setting limits_maxposition");
        return;
    }
    err = mdco.writeOpenRegisters(0x6076, 0x00, 1000);
    if (err != MDCO::OK)
    {
        log.error("Error setting limits_ratedtorque");
        return;
    }
    err = mdco.writeOpenRegisters(0x6072, 0x00, limits_maxtorque);
    if (err != MDCO::OK)
    {
        log.error("Error setting limits_maxtorque");
        return;
    }
    err = mdco.writeOpenRegisters(0x6075, 0x00, 1000);
    if (err != MDCO::OK)
    {
        log.error("Error setting limits_ratedcurrent");
        return;
    }
    err = mdco.writeOpenRegisters(0x6073, 0x00, limits_maxcurrent);
    if (err != MDCO::OK)
    {
        log.error("Error setting limits_maxcurrent");
        return;
    }
    err = mdco.writeOpenRegisters(0x6080, 0x00, limits_maxvelocity);
    if (err != MDCO::OK)
    {
        log.error("Error setting limits_maxvelocity");
        return;
    }
    err = mdco.writeOpenRegisters(0x60C5, 0x00, limits_maxacceleration);
    if (err != MDCO::OK)
    {
        log.error("Error setting limits_maxacceleration");
        return;
    }
    err = mdco.writeOpenRegisters(0x60C6, 0x00, limits_maxdeceleration);
    if (err != MDCO::OK)
    {
        log.error("Error setting limits_maxdeceleration");
        return;
    }
    uint32_t positionpid_kp_as_long;
    std::memcpy(&positionpid_kp_as_long, &positionpid_kp, sizeof(float));
    err = mdco.writeOpenRegisters(0x2002, 0x01, positionpid_kp_as_long, 4);
    if (err != MDCO::OK)
    {
        log.error("Error setting positionpid_kp");
        return;
    }
    uint32_t positionpid_ki_as_long;
    std::memcpy(&positionpid_ki_as_long, &positionpid_ki, sizeof(float));
    err = mdco.writeOpenRegisters(0x2002, 0x02, positionpid_ki_as_long, 4);
    if (err != MDCO::OK)
    {
        log.error("Error setting positionpid_ki");
        return;
    }
    uint32_t positionpid_kd_as_long;
    std::memcpy(&positionpid_kd_as_long, &positionpid_kd, sizeof(float));
    err = mdco.writeOpenRegisters(0x2002, 0x03, positionpid_kd_as_long, 4);
    if (err != MDCO::OK)
    {
        log.error("Error setting positionpid_kd");
        return;
    }
    uint32_t positionpid_windup_as_long;
    std::memcpy(&positionpid_windup_as_long, &positionpid_windup, sizeof(float));
    err = mdco.writeOpenRegisters(0x2002, 0x04, positionpid_windup_as_long, 4);
    if (err != MDCO::OK)
    {
        log.error("Error setting positionpid_windup");
        return;
    }
    uint32_t velocitypid_kp_as_long;
    std::memcpy(&velocitypid_kp_as_long, &velocitypid_kp, sizeof(float));
    err = mdco.writeOpenRegisters(0x2001, 0x01, velocitypid_kp_as_long, 4);
    if (err != MDCO::OK)
    {
        log.error("Error setting velocitypid_kp");
        return;
    }
    uint32_t velocitypid_ki_as_long;
    std::memcpy(&velocitypid_ki_as_long, &velocitypid_ki, sizeof(float));
    err = mdco.writeOpenRegisters(0x2001, 0x02, velocitypid_ki_as_long, 4);
    if (err != MDCO::OK)
    {
        log.error("Error setting velocitypid_ki");
        return;
    }
    uint32_t velocitypid_kd_as_long;
    std::memcpy(&velocitypid_kd_as_long, &velocitypid_kd, sizeof(float));
    err = mdco.writeOpenRegisters(0x2001, 0x03, velocitypid_kd_as_long, 4);
    if (err != MDCO::OK)
    {
        log.error("Error setting velocitypid_kd");
        return;
    }
    uint32_t velocitypid_windup_as_long;
    std::memcpy(&velocitypid_windup_as_long, &velocitypid_windup, sizeof(float));
    err = mdco.writeOpenRegisters(0x2001, 0x04, velocitypid_windup_as_long, 4);
    if (err != MDCO::OK)
    {
        log.error("Error setting velocitypid_windup");
        return;
    }
    uint32_t impedancepd_kp_as_long;
    std::memcpy(&impedancepd_kp_as_long, &impedancepd_kp, sizeof(float));
    err = mdco.writeOpenRegisters(0x200C, 0x01, impedancepd_kp, 4);
    if (err != MDCO::OK)
    {
        log.error("Error setting impedancepd_kp");
        return;
    }
    uint32_t impedancepd_kd_as_long;
    std::memcpy(&impedancepd_kd_as_long, &impedancepd_kd, sizeof(float));
    err = mdco.writeOpenRegisters(0x200C, 0x02, impedancepd_kd, 4);
    if (err != MDCO::OK)
    {
        log.error("Error setting impedancepd_kd");
        return;
    }
}

void CandleToolCO::clean(std::string& s)
{
    size_t write_pos    = 0;
    bool   seenNonSpace = false;
    for (size_t read_pos = 0; read_pos < s.size(); ++read_pos)
    {
        unsigned char c = s[read_pos];
        if (std::isspace(c))
        {
            if (!seenNonSpace)
                continue;  // skip leading spaces
            continue;      // skip all spaces (internal & trailing)
        }
        seenNonSpace   = true;
        s[write_pos++] = std::tolower(c);
    }
    s.resize(write_pos);
}

void CandleToolCO::heartbeatTest(u32 MasterId, u32 SlaveId, u32 HeartbeatTimeout)
{
    if (MasterId > 0x7F || SlaveId > 0x7f)
    {
        log.error("id > 0x7F");
        return;
    }
    MDCO            md         = MDCO(SlaveId, m_candle);
    MDCO            mdproducer = MDCO(MasterId, m_candle);
    std::vector<u8> frame      = {0x05};
    long            DataSlave;
    u8              bytes1 = ((u8)(MasterId >> 8));
    u8              bytes2 = ((u8)(MasterId));
    u8              bytes3 = ((u8)(HeartbeatTimeout >> 8));
    u8              bytes4 = ((u8)(HeartbeatTimeout));
    DataSlave              = bytes4 + (bytes3 << 8) + (bytes2 << 16) + (bytes1 << 24);
    mdproducer.sendCustomData(0x700 + MasterId, frame);
    if (md.getValueFromOpenRegister(0x1003, 0x00) != 00)
    {
        log.error("The Driver is in fault state before testing the hearbeat");
        return;
    }

    md.writeOpenRegisters(0x1016, 0x01, DataSlave, 4);
    mdproducer.sendCustomData(0x700 + MasterId, frame);
    auto start   = std::chrono::steady_clock::now();
    auto timeout = std::chrono::milliseconds((HeartbeatTimeout / 100));
    while (std::chrono::steady_clock::now() - start < timeout)
    {
    }
    // verify if error state before Hearbeattimeout
    if (md.getValueFromOpenRegister(0x1001, 0) != 0)
    {
        log.error("The driver enter fault mode before the Heartbeat timeout");
        return;
    }
    start   = std::chrono::steady_clock::now();
    timeout = std::chrono::milliseconds((HeartbeatTimeout * 2));
    while (std::chrono::steady_clock::now() - start < timeout)
    {
    }
    // verify if error state after Hearbeattimeout
    if (md.getValueFromOpenRegister(0x1001, 0) != 0)
    {
        log.success("The driver enter fault mode after the Heartbeat timeout");
        return;
    }
    mdproducer.sendCustomData(0x700 + MasterId, frame);
    if (md.getValueFromOpenRegister(0x1001, 0) != 0)
    {
        log.error("The driver still in error mode");
    }
}

void CandleToolCO::SendSync(u16 id)
{
    MDCO            mdco(id, m_candle);
    long            SyncMessageValue = mdco.getValueFromOpenRegister(0x1005, 0x0);
    std::vector<u8> data;
    MDCO::Error_t   err;
    if (SyncMessageValue != -1)
    {
        err = mdco.writeOpenPDORegisters((int)SyncMessageValue, data);
        if (err != MDCO::OK)
        {
            log.error("Error sending sync message");
            return;
        }
        log.success("Sync message send with value:0x%x (default value is 0x80)", SyncMessageValue);
    }
    else
        log.error("MD with ID:0x%x is not detected", SyncMessageValue);
}

void CandleToolCO::setupReadConfig(u16 id, const std::string& cfgName)
{
    mINI::INIStructure readIni; /**< mINI structure for read data */
    MDCO               mdco = MDCO(id, m_candle);
    MDRegisters_S      regs; /**< read register */
    std::string        configName = cfgName;
    mdco.m_timeout                = 10;

    long raw_data = 0;
    /*────────────────────────────
      VARIABLES  –  section [motor]
    ────────────────────────────*/
    std::vector<u8> motor_name;
    mdco.readLongOpenRegisters(0x2000, 0x06, motor_name);
    int   motor_kv         = 100;
    u32   motor_pole_pairs = mdco.getValueFromOpenRegister(0x2000, 0x01);
    float motor_torque_constant;
    raw_data = (float)mdco.getValueFromOpenRegister(0x2000, 0x02);
    std::memcpy(&motor_torque_constant, &raw_data, sizeof(float));
    raw_data = (float)mdco.getValueFromOpenRegister(0x2000, 0x08);
    float motor_gear_ratio;
    std::memcpy(&motor_gear_ratio, &raw_data, sizeof(float));
    u16 motor_torque_bandwidth = mdco.getValueFromOpenRegister(0x2000, 0x05);
    u32 motor_shutdown_temp    = mdco.getValueFromOpenRegister(0x2000, 0x07);

    /*────────────────────────────
      section [limits]
    ────────────────────────────*/
    u16 limits_max_current      = mdco.getValueFromOpenRegister(0x6073, 0x00);
    u32 limits_rated_current    = mdco.getValueFromOpenRegister(0x6075, 0x00);
    u16 limits_max_torque       = mdco.getValueFromOpenRegister(0x6072, 0x00);
    u32 limits_rated_torque     = mdco.getValueFromOpenRegister(0x6076, 0x00);
    u32 limits_max_velocity     = mdco.getValueFromOpenRegister(0x6080, 0x00);
    u32 limits_max_position     = mdco.getValueFromOpenRegister(0x607D, 0x02);
    u32 limits_min_position     = mdco.getValueFromOpenRegister(0x607D, 0x01);
    u32 limits_max_acceleration = mdco.getValueFromOpenRegister(0x60C5, 0x00);
    u32 limits_max_deceleration = mdco.getValueFromOpenRegister(0x60C6, 0x00);

    /*────────────────────────────
      section [profile]
    ────────────────────────────*/
    u32 profile_acceleration = 0.0f;
    raw_data                 = mdco.getValueFromOpenRegister(0x2008, 0x04);
    std::memcpy(&profile_acceleration, &raw_data, sizeof(float));
    u32 profile_deceleration = 0.0f;
    raw_data                 = mdco.getValueFromOpenRegister(0x2008, 0x05);
    std::memcpy(&profile_deceleration, &raw_data, sizeof(float));
    u32 profile_velocity = 0.0f;
    raw_data             = mdco.getValueFromOpenRegister(0x2008, 0x03);
    std::memcpy(&profile_velocity, &raw_data, sizeof(float));

    /*────────────────────────────
      section [output encoder]
    ────────────────────────────*/
    short output_encoder = 0;
    raw_data             = mdco.getValueFromOpenRegister(0x2005, 0x01);
    std::memcpy(&output_encoder, &raw_data, sizeof(short));
    short output_encoder_mode = 0;
    raw_data                  = mdco.getValueFromOpenRegister(0x2005, 0x03);
    std::memcpy(&output_encoder_mode, &raw_data, sizeof(short));

    /*────────────────────────────
      section [position pid]
    ────────────────────────────*/
    float pos_kp = 0.0f;
    raw_data     = mdco.getValueFromOpenRegister(0x2002, 0x01);
    std::memcpy(&pos_kp, &raw_data, sizeof(float));
    float pos_ki = 0.0f;
    raw_data     = mdco.getValueFromOpenRegister(0x2002, 0x02);
    std::memcpy(&pos_ki, &raw_data, sizeof(float));
    float pos_kd = 0.0f;
    raw_data     = mdco.getValueFromOpenRegister(0x2002, 0x03);
    std::memcpy(&pos_kd, &raw_data, sizeof(float));
    float pos_windup = 0.0f;
    raw_data         = mdco.getValueFromOpenRegister(0x2002, 0x04);
    std::memcpy(&pos_windup, &raw_data, sizeof(float));

    /*────────────────────────────
      section [velocity pid]
    ────────────────────────────*/
    float vel_kp = 0.0f;
    raw_data     = mdco.getValueFromOpenRegister(0x2001, 0x01);
    std::memcpy(&vel_kp, &raw_data, sizeof(float));
    float vel_ki = 0.0f;
    raw_data     = mdco.getValueFromOpenRegister(0x2001, 0x02);
    std::memcpy(&vel_ki, &raw_data, sizeof(float));
    float vel_kd = 0.0f;
    raw_data     = mdco.getValueFromOpenRegister(0x2001, 0x03);
    std::memcpy(&vel_kd, &raw_data, sizeof(float));
    float vel_windup = 0.0f;
    raw_data         = mdco.getValueFromOpenRegister(0x2001, 0x04);
    std::memcpy(&vel_windup, &raw_data, sizeof(float));

    /*────────────────────────────
      section [impedance pd]
    ────────────────────────────*/
    float imp_kp = 0.0f;
    raw_data     = mdco.getValueFromOpenRegister(0x200C, 0x01);
    std::memcpy(&imp_kp, &raw_data, sizeof(float));
    float imp_kd = 0.0f;
    raw_data     = mdco.getValueFromOpenRegister(0x200C, 0x02);
    std::memcpy(&imp_kd, &raw_data, sizeof(float));

    // ────────────────────────────────────────────────────────────
    // ÉCRITURE DU FICHIER CONFIG
    // ────────────────────────────────────────────────────────────
    std::ofstream cfg(configName);
    if (!cfg.is_open())
    {
        std::cerr << "Impossible to open" << configName << " in wrting mode.\n";
        return;
    }

    cfg << std::fixed << std::setprecision(5);

    // ----- [motor] -----
    cfg << "[motor]\n";
    cfg << "name = " << std::string(motor_name.begin(), motor_name.end()) << '\n';
    cfg << "pole pairs = " << motor_pole_pairs << '\n';
    cfg << "kv = " << motor_kv << '\n';
    cfg << "torque constant = " << motor_torque_constant << '\n';
    cfg << "gear ratio = " << motor_gear_ratio << '\n';
    cfg << "torque bandwidth = " << motor_torque_bandwidth << '\n';
    cfg << "shutdown temp = " << motor_shutdown_temp << "\n\n";

    // ----- [limits] -----
    cfg << "[limits]\n";
    cfg << "rated current = " << limits_rated_current << '\n';
    cfg << "max current = " << limits_max_current << '\n';
    cfg << "rated torque = " << limits_rated_torque << '\n';
    cfg << "max torque = " << limits_max_torque << '\n';
    cfg << "max velocity = " << limits_max_velocity << '\n';
    cfg << "max position = " << limits_max_position << '\n';
    cfg << "min position = " << limits_min_position << '\n';
    cfg << "max acceleration = " << limits_max_acceleration << '\n';
    cfg << "max deceleration = " << limits_max_deceleration << "\n\n";

    // ----- [profile] -----
    cfg << "[profile]\n";
    cfg << "acceleration = " << profile_acceleration << '\n';
    cfg << "deceleration = " << profile_deceleration << '\n';
    cfg << "velocity = " << profile_velocity << "\n\n";

    // ----- [output encoder] -----
    cfg << "[output encoder]\n";
    cfg << "output encoder = " << output_encoder << '\n';
    cfg << "output encoder mode = " << output_encoder_mode << "\n\n";

    // ----- [position pid] -----
    cfg << "[position pid]\n";
    cfg << "kp = " << pos_kp << '\n';
    cfg << "ki = " << pos_ki << '\n';
    cfg << "kd = " << pos_kd << '\n';
    cfg << "windup = " << pos_windup << "\n\n";

    // ----- [velocity pid] -----
    cfg << "[velocity pid]\n";
    cfg << "kp = " << vel_kp << '\n';
    cfg << "ki = " << vel_ki << '\n';
    cfg << "kd = " << vel_kd << '\n';
    cfg << "windup = " << vel_windup << "\n\n";

    // ----- [impedance pd] -----
    cfg << "[impedance pd]\n";
    cfg << "kp = " << imp_kp << '\n';
    cfg << "kd = " << imp_kd << "\n\n";

    cfg.close();
    log.info("Fichier %s généré avec succès.\n ", configName.c_str());
}

void CandleToolCO::setupInfo(u16 id, bool printAll)
{
    MDCO mdco = MDCO(id, m_candle);

    if (!printAll)
    {
        long devicetype = mdco.getValueFromOpenRegister(0x1000, 0);
        log.info("Device type:", devicetype);
        long int    hexValue = mdco.getValueFromOpenRegister(0x1008, 0);
        std::string motorName;
        for (int i = 0; i <= 3; i++)
        {
            char c = (hexValue >> (8 * i)) & 0xFF;
            motorName += c;
        }
        log.info("Manufacturer Device Name: %s", motorName.c_str());
        long Firmware = mdco.getValueFromOpenRegister(0x200A, 3);
        log.info("Firmware version: %li", Firmware);
        long Bootloder = mdco.getValueFromOpenRegister(0x200B, 4);
        log.info("Bootloder version: %li", Bootloder);
    }
    else
    {
        mdco.printAllInfo();
    }
}

void CandleToolCO::testMove(u16 id, f32 targetPosition, moveParameter& param)
{
    MDCO mdco(id, m_candle);
    long DesiredPosition = (long)(targetPosition);

    MDCO::Error_t err;
    err = mdco.setProfileParameters(param);
    if (err != MDCO::OK)
    {
        log.error("Error setting profile parameters");
        return;
    }
    err = mdco.enableDriver(CyclicSyncPosition);
    if (err != MDCO::OK)
    {
        log.error("Error enabling driver");
        return;
    }
    mdco.movePosition(DesiredPosition);
    err = mdco.disableDriver();
    if (err != MDCO::OK)
    {
        log.error("Error disabling driver");
        return;
    }
}

void CandleToolCO::testMoveAbsolute(u16 id, i32 targetPos, moveParameter& param)
{
    MDCO mdco(id, m_candle);
    long desiredPos = ((long)targetPos);

    MDCO::Error_t err;
    err = mdco.setProfileParameters(param);
    if (err != MDCO::OK)
    {
        log.error("Error setting profile parameters");
        return;
    }
    err = mdco.enableDriver(ProfilePosition);
    if (err != MDCO::OK)
    {
        log.error("Error enabling driver");
        return;
    }
    mdco.movePosition(desiredPos);
    err = mdco.disableDriver();
    if (err != MDCO::OK)
    {
        log.error("Error disabling driver");
        return;
    }
}

void CandleToolCO::testMoveSpeed(u16 id, moveParameter& param, i32 DesiredSpeed)
{
    MDCO mdco(id, m_candle);

    MDCO::Error_t err;
    err = mdco.setProfileParameters(param);
    if (err != MDCO::OK)
    {
        log.error("Error setting profile parameters");
        return;
    }
    err = mdco.enableDriver(CyclicSyncVelocity);
    if (err != MDCO::OK)
    {
        log.error("Error enabling driver");
        return;
    }
    mdco.moveSpeed(DesiredSpeed);
    err = mdco.disableDriver();
    if (err != MDCO::OK)
    {
        log.error("Error disabling driver");
        return;
    }
}

void CandleToolCO::testMoveImpedance(u16 id, i32 desiredSpeed, f32 targetPos, moveParameter& param)
{
    MDCO mdco(id, m_candle);

    MDCO::Error_t err;
    err = mdco.setProfileParameters(param);
    if (err != MDCO::OK)
    {
        log.error("Error setting profile parameters");
        return;
    }
    err = mdco.enableDriver(Impedance);
    if (err != MDCO::OK)
    {
        log.error("Error enabling driver");
        return;
    }
    err = mdco.moveImpedance(desiredSpeed, targetPos, param, 5000);
    if (err != MDCO::OK)
    {
        log.error("Error moving impedance");
        return;
    }
    err = mdco.disableDriver();
    if (err != MDCO::OK)
    {
        log.error("Error disabling driver");
        return;
    }
}

void CandleToolCO::testLatency(u16 id)
{
    MDCO md(id, m_candle);
    u64  latence_totale = 0;
    bool testOk         = true;

    for (int i = 0; i < 100; ++i)
    {
        auto start = std::chrono::steady_clock::now();

        if (md.readOpenRegisters(0x1000, 0) != MDCO::Error_t::OK)
        {
            testOk = false;
        }

        auto end   = std::chrono::steady_clock::now();
        auto duree = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        latence_totale += static_cast<u64>(duree);
    }

    if (testOk)
    {
        u64 moyenne = latence_totale / 100;
        log.info("---------------Latence---------------\n");
        log.info("Total: %lu µs\n", latence_totale);
        log.info("Result (average of 100 attempts): %lu µs\n", moyenne);
    }

    else
    {
        log.error("MD driver not answering");
    }
}

void CandleToolCO::testEncoderOutput(u16 id)
{
    MDCO          mdco(id, m_candle);
    MDCO::Error_t err = mdco.testEncoder(false, true);
    if (err != MDCO::OK)
    {
        log.error("Error running output encoder test");
        return;
    }
}

void CandleToolCO::testEncoderMain(u16 id)
{
    MDCO          mdco(id, m_candle);
    MDCO::Error_t err = mdco.testEncoder(true, false);
    if (err != MDCO::OK)
    {
        log.error("Error running main encoder test");
        return;
    }
}

void CandleToolCO::SDOsegmentedRead(u16 id, u16 reg, u8 subIndex)
{
    MDCO            mdco(id, m_candle);
    std::vector<u8> data;
    MDCO::Error_t   err = mdco.readLongOpenRegisters(reg, subIndex, data);
    if (err != MDCO::OK)
    {
        log.error("Error reading segmented SDO 0x%04X subindex %d", reg, subIndex);
        return;
    }
}

void CandleToolCO::SDOsegmentedWrite(u16 id, u16 reg, u8 subIndex, std::string& data)
{
    MDCO          mdco(id, m_candle);
    MDCO::Error_t err = mdco.writeLongOpenRegisters(reg, subIndex, data);
    if (err != MDCO::OK)
    {
        log.error("Error writing segmented SDO 0x%04X subindex %d", reg, subIndex);
        return;
    }
}

void CandleToolCO::registerRead(u16 id, u16 regAdress, u8 subIndex, bool force)
{
    MDCO            mdco(id, m_candle);
    std::vector<u8> data;
    int             dataSize = mdco.dataSizeOfEdsObject(regAdress, subIndex);
    if (!force)
    {
        if (dataSize == 1 || dataSize == 2 || dataSize == 4)
            mdco.readOpenRegisters(regAdress, subIndex, force);
        else if (dataSize == 8 || dataSize == 0)
            mdco.readLongOpenRegisters(regAdress, subIndex, data);
        else
        {
            log.error("Unknown register size for register 0x%04X subindex %d", regAdress, subIndex);
        }
    }
    else
        mdco.readOpenRegisters(regAdress, subIndex, force);
}

void CandleToolCO::registerWrite(
    u16 id, u16 regAdress, const std::string& value, u8 subIndex, u8 dataSize, bool force)
{
    MDCO          mdco(id, m_candle);
    MDCO::Error_t err;
    // if no value is given, we read the value from the object dictionary
    if (dataSize == 0)
    {
        if (force)
        {
            log.error("Please enter the data syze in bytes if you use the -f flag");
            return;
        }
        else
            dataSize = mdco.dataSizeOfEdsObject(regAdress, subIndex);
    }

    if (dataSize == 1 || dataSize == 2 || dataSize == 4)
    {
        unsigned long data = strtoul((value.c_str()), nullptr, 16);
        err                = mdco.writeOpenRegisters(regAdress, subIndex, data, dataSize, force);
        if (err != MDCO::OK)
        {
            log.error("Error writing register 0x%04X subindex %d", regAdress, subIndex);
            return;
        }
        return;
    }
    else if (dataSize == 8)
    {
        err = mdco.writeLongOpenRegisters(regAdress, subIndex, value, force);
        if (err != MDCO::OK)
        {
            log.error("Error writing long register 0x%04X subindex %d", regAdress, subIndex);
            return;
        }
        return;
    }
    else if (dataSize == 0)
    {
        err = mdco.writeLongOpenRegisters(regAdress, subIndex, value, force);
        if (err != MDCO::OK)
        {
            log.error("Error writing string register 0x%04X subindex %d", regAdress, subIndex);
            return;
        }
        return;
    }
    else
    {
        log.error("Wrong/unknow data size");
        return;
    }
}

void CandleToolCO::SendTime(uint16_t id)
{
    MDCO mdco(id, m_candle);

    auto now = std::chrono::system_clock::now();

    std::tm epoch_tm  = {};
    epoch_tm.tm_year  = 84;
    epoch_tm.tm_mon   = 0;
    epoch_tm.tm_mday  = 1;
    epoch_tm.tm_hour  = 0;
    epoch_tm.tm_min   = 0;
    epoch_tm.tm_sec   = 0;
    epoch_tm.tm_isdst = -1;

    auto        epoch_time_t = std::mktime(&epoch_tm);
    auto        epoch_tp     = std::chrono::system_clock::from_time_t(epoch_time_t);
    auto        days_since  = std::chrono::duration_cast<std::chrono::days>(now - epoch_tp).count();
    std::time_t now_time_t  = std::chrono::system_clock::to_time_t(now);
    std::tm     local_tm    = *std::localtime(&now_time_t);
    std::tm     midnight_tm = local_tm;

    midnight_tm.tm_hour = 0;
    midnight_tm.tm_min  = 0;
    midnight_tm.tm_sec  = 0;

    auto midnight_time_t       = std::mktime(&midnight_tm);
    auto midnight_tp           = std::chrono::system_clock::from_time_t(midnight_time_t);
    long millis_since_midnight = static_cast<long>(
        std::chrono::duration_cast<std::chrono::milliseconds>(now - midnight_tp).count());

    log.info("The actual time according to your computer is: %s", std::asctime(&local_tm));
    log.info("Number of days since 1st January 1984: %ld", days_since);
    log.info("Number of millis since midnight: %ld", millis_since_midnight);

    long TimeMessageId = mdco.getValueFromOpenRegister(0x1012, 0x00);

    std::vector<u8> frame = {
        ((u8)millis_since_midnight),
        ((u8)(millis_since_midnight >> 8)),
        ((u8)(millis_since_midnight >> 16)),
        ((u8)(millis_since_midnight >> 24)),
        ((u8)days_since),
        ((u8)(days_since >> 8)),
    };

    MDCO::Error_t err = mdco.writeOpenPDORegisters(TimeMessageId, frame);
    if (err != MDCO::OK)
    {
        log.error("Error sending time message");
        return;
    }
}
void CandleToolCO::blink(u16 id)
{
    MDCO mdco(id, m_candle);
    if (mdco.blinkOpenTest() != MDCO::Error_t::OK)
    {
        log.error("Failed to blink MD device with ID %d", id);
        return;
    }
    log.success("Blinking MD device with ID %d", id);
}

void CandleToolCO::encoder(u16 id)
{
    MDCO          mdco(id, m_candle);
    MDCO::Error_t err = mdco.readOpenRegisters(0x6064, 0);
    if (err != MDCO::OK)
    {
        log.error("Error reading encoder value");
        return;
    }
}

void CandleToolCO::clearErrors(u16 id, const std::string& level)
{
    MDCO mdco(id, m_candle);
    log.info("envoie en canOpen clear error \n");
    MDCO::Error_t err = MDCO::OK;
    if (level == "error")
        err = mdco.clearOpenErrors(1);
    else if (level == "warning")
        err = mdco.clearOpenErrors(2);
    else if (level == "all")
        err = mdco.clearOpenErrors(3);
    else
    {
        log.error("Unknown command");
        return;
    }
    if (err != MDCO::OK)
    {
        log.error("Error clearing errors (%s)", level.c_str());
        return;
    }
}

void CandleToolCO::reset(u16 id)
{
    MDCO          mdco(id, m_candle);
    MDCO::Error_t err = mdco.openReset();
    if (err != MDCO::OK)
    {
        log.error("Error resetting MD device with ID %d", id);
        return;
    }
}

u8 CandleToolCO::getNumericParamFromList(std::string& param, const std::vector<std::string>& list)
{
    int i = 0;
    for (auto& type : list)
    {
        if (type == param)
            return i;
        i++;
    }
    return 0;
}

void CandleToolCO::edsLoad(const std::string& edsFilePath)
{
    edsParser MyEdsParser;
    MyEdsParser.load(edsFilePath);
}

void CandleToolCO::edsUnload()
{
    edsParser MyEdsParser;
    MyEdsParser.unload();
}

void CandleToolCO::edsDisplay()
{
    edsParser MyEdsParser;
    MyEdsParser.display();
}

void CandleToolCO::edsGenerateMarkdown(const std::string& path)
{
    edsParser MyEdsParser;
    MyEdsParser.generateMarkdown(path);
}

void CandleToolCO::edsGenerateHtml(const std::string& path)
{
    edsParser MyEdsParser;
    MyEdsParser.generateHtml(path);
}

void CandleToolCO::edsGenerateCpp(const std::string& path)
{
    edsParser MyEdsParser;
    MyEdsParser.generateCpp(path);
}

void CandleToolCO::edsGet(u32 index, u8 subindex)
{
    edsParser MyEdsParser;
    MyEdsParser.get(index, subindex);
}

void CandleToolCO::edsFind(const std::string& research)
{
    edsParser MyEdsParser;
    MyEdsParser.find(research);
}

void CandleToolCO::edsAddObject(const edsObject& obj)
{
    edsParser MyEdsParser;
    MyEdsParser.addObject(obj);
}

void CandleToolCO::edsDeleteObject(u32 index, u8 subindex)
{
    edsParser MyEdsParser;
    MyEdsParser.deleteObject(index, subindex);
}

void CandleToolCO::edsModifyCorrection(const edsObject& obj, u16 id, u8 subIndex)
{
    edsParser MyEdsParser;
    MyEdsParser.modifyObject(obj, id, subIndex);
}

void CandleToolCO::SendNMT(u8 id, u8 command)
{
    MDCO            mdco(id, m_candle);
    std::vector<u8> data = {command, id};
    MDCO::Error_t   err  = mdco.writeOpenPDORegisters(0x000, data);
    if (err != MDCO::OK)
    {
        log.error("Error sending NMT command %d to node %d", command, id);
        return;
    }
}
void CandleToolCO::ReadHeartbeat(u16 id)
{
    MDCO          mdco(id, m_candle);
    MDCO::Error_t err = mdco.testHeartbeat();
    if (err != MDCO::OK)
    {
        log.error("Error reading heartbeat for node %d", id);
        return;
    }
}