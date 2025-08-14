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
#include "configHelpers.hpp"
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
    // log.info("CandleSDK Version: %s", mab::Candle::getVersion().c_str());

    std::unique_ptr<I_CommunicationInterface> bus;

    mINI::INIFile      file(getCandletoolConfigPath());
    mINI::INIStructure ini;
    file.read(ini);

    std::string& device = ini["communication"]["device"];
    busString           = ini["communication"]["bus"];

    // if (busString == "SPI")
    // {
    //     bus = nullptr;  // TODO: placeholder
    // }
    // else if (busString == "USB")
    bus = std::make_unique<USB>(Candle::CANDLE_VID, Candle::CANDLE_PID, device);

    m_candle = attachCandle(baud, std::move(bus));
    // TODO: move this to be more stateless and be able to start w/o candle attached
}

CandleToolCO::~CandleToolCO()
{
    detachCandle(m_candle);
}

void CandleToolCO::ping(const std::string& variant)
{
    if (variant == "all")
    {
        // TODO: implement all variant later, change string variant to enum to avoid undefined
        log.error("Not implemented");
        // TODO: implement all variant later, change string variant to enum to avoid undefined
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
    MDRegisters_S mdRegisters;
    MDCO          mdco = MDCO(id, m_candle);

    long newbaud;
    if (baud == "1M")
        newbaud = 1;
    if (baud == "2M")
        newbaud = 2;
    if (baud == "5M")
        newbaud = 5;
    if (baud == "8M")
        newbaud = 8;
    mdco.newCanOpenConfig(newId, newbaud, termination);
}

void CandleToolCO::configSave(u16 id)
{
    MDCO mdco = MDCO(id, m_candle);

    mdco.openSave();
}

void CandleToolCO::configZero(u16 id)
{
    MDCO mdco = MDCO(id, m_candle);

    mdco.openZero();
}

void CandleToolCO::configBandwidth(u16 id, f32 bandwidth)
{
    MDCO mdco = MDCO(id, m_candle);

    u16 newBandwidth = ((u16)bandwidth);
    mdco.canOpenBandwidth(newBandwidth);
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
    md.setProfileParameters(param);
    md.enableDriver(CyclicSyncVelocity);

    log.info("Sending PDO for speed loop control");
    std::vector<u8> frameSetup;
    frameSetup.reserve(3);
    frameSetup.push_back(0x0F);
    frameSetup.push_back(0x00);
    frameSetup.push_back(0x09);
    md.writeOpenPDORegisters(0x300 + id, frameSetup);

    std::vector<u8> frameSpeed;
    frameSpeed.reserve(6);
    frameSpeed.push_back(0x0F);
    frameSpeed.push_back(0x00);
    frameSpeed.push_back((u8)(desiredSpeed));
    frameSpeed.push_back((u8)(desiredSpeed >> 8));
    frameSpeed.push_back((u8)(desiredSpeed >> 16));
    frameSpeed.push_back((u8)(desiredSpeed >> 24));

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
    md.disableDriver();
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
    md.setProfileParameters(param);
    md.enableDriver(CyclicSyncPosition);

    auto start   = std::chrono::steady_clock::now();
    auto timeout = std::chrono::seconds((1));

    log.info("Sending PDO for speed loop control");

    std::vector<u8> frameSetup;
    frameSetup.reserve(3);
    frameSetup.push_back(0x0F);
    frameSetup.push_back(0x00);
    frameSetup.push_back(0x08);
    md.writeOpenPDORegisters(0x300 + id, frameSetup);
    std::vector<u8> framePosition;
    framePosition.reserve(3);
    framePosition.push_back(0x0F);
    framePosition.push_back(0x00);
    framePosition.push_back((u8)(DesiredPos));
    framePosition.push_back((u8)(DesiredPos >> 8));
    framePosition.push_back((u8)(DesiredPos >> 16));
    framePosition.push_back((u8)(DesiredPos >> 24));

    log.debug("position ask : %d\n", DesiredPos);

    start           = std::chrono::steady_clock::now();
    auto lastSend   = start;
    timeout         = std::chrono::seconds(5);
    auto sendPeriod = std::chrono::milliseconds(10);

    while (std::chrono::steady_clock::now() - start < timeout &&
           !((int)md.getValueFromOpenRegister(0x6064, 0) > (DesiredPos - 100) &&
             (int)md.getValueFromOpenRegister(0x6064, 0) < (DesiredPos + 100)))
    {
        auto now = std::chrono::steady_clock::now();
        if (now - lastSend >= sendPeriod)
        {
            md.writeOpenRegisters("Motor Target Position", DesiredPos, 4);
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

    md.disableDriver();
}

void CandleToolCO::SendCustomPdo(u16 id, const edsObject& Desiregister, u64 data)
{
    // TO DO: finish this
    MDCO mdco = MDCO(id, m_candle);
    // if the the index start with 0x1A00
    if (Desiregister.index == (0x1600))
    {
        std::vector<u8> frame;
        frame.reserve(2);
        frame.push_back((u8)(data >> 8));
        frame.push_back(data);
        mdco.writeOpenPDORegisters(0x200 + id, frame);
    }
    else if (Desiregister.index == (0x1601))
    {
        std::vector<u8> frame;
        frame.reserve(3);
        frame.push_back((u8)(data >> 16));
        frame.push_back((u8)(data >> 8));
        frame.push_back((u8)data);
        mdco.writeOpenPDORegisters(0x300 + id, frame);
    }
    else if (Desiregister.index == (0x1602))
    {
        std::vector<u8> frame;
        frame.reserve(6);
        frame.push_back((u8)(data >> 40));
        frame.push_back((u8)(data >> 32));
        frame.push_back((u8)(data >> 24));
        frame.push_back((u8)(data >> 16));
        frame.push_back((u8)(data >> 8));
        frame.push_back((u8)data);
        mdco.writeOpenPDORegisters(0x400 + id, frame);
    }
    else if (Desiregister.index == (0x1603))
    {
        std::vector<u8> frame;
        frame.reserve(6);
        frame.push_back((u8)(data >> 40));
        frame.push_back((u8)(data >> 32));
        frame.push_back((u8)(data >> 24));
        frame.push_back((u8)(data >> 16));
        frame.push_back((u8)(data >> 8));
        frame.push_back((u8)data);
        mdco.writeOpenPDORegisters(0x500 + id, frame);
    }
    else
    {
        log.error("Please enter a index between 0x1600 & 0x1603 (Transmit PDO)");
    }
}

void CandleToolCO::setupCalibration(u16 id)
{
    MDCO mdco = MDCO(id, m_candle);
    mdco.encoderCalibration(1, 0);
}

void CandleToolCO::setupCalibrationOutput(u16 id)
{
    MDCO mdco = MDCO(id, m_candle);
    mdco.encoderCalibration(0, 1);
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
    if (!fileExists(getDefaultConfigPath()))
    {
        log.warn("No default config found at expected location \"%s\"",
                 getDefaultConfigPath().c_str());
        log.warn("Cannot check completeness of the config file. Proceed with upload? [y/n]");
        if (!getConfirmation())
            exit(0);
    }
    if (fileExists(getDefaultConfigPath()) && !isConfigComplete(finalConfigPath))
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

    std::string motor_name           = "";   // impossible without segmented transfert
    int         motor_polepairs      = 0;    // 0x2000 0x01 u32
    int         motor_kv             = 0;    // no object refered to kv
    float       motor_torqueconstant = 0.0;  // 0x2000 0x02 f32
    float       motor_gearratio      = 0.0;  // 0x2000 0x08 f32
    int         motor_maxcurrent     = 0;  // not usefull because it's not save after motor shutdown
    int         motor_torquebandwidth = 0;  // 0x2000 0x05 u16
    int         motor_shutdowntemp    = 0;  // 0x2000 0x07 u8

    float limits_maxtorque       = 0;  // not usefull because it's not save after motor shutdown
    int   limits_maxvelocity     = 0;  // not usefull because it's not save after motor shutdown
    int   limits_maxposition     = 0;  // not usefull because it's not save after motor shutdown
    int   limits_minposition     = 0;  // not usefull because it's not save after motor shutdown
    int   limits_maxacceleration = 0;  // not usefull because it's not save after motor shutdown
    int   limits_maxdeceleration = 0;  // not usefull because it's not save after motor shutdown

    int profile_acceleration = 0;  // not usefull because it's not save after motor shutdown
    int profile_deceleration = 0;  // not usefull because it's not save after motor shutdown
    int profile_velocity     = 0;  // not usefull because it's not save after motor shutdown

    int outputencoder_outputencoder     = 0;  // ?
    int outputencoder_outputencodermode = 0;  // 0x2005 0x03 u8

    float positionpid_kp     = 0.0;  // 0x2002 0x01 f32
    float positionpid_ki     = 0.0;  // 0x2002 0x02 f32
    float positionpid_kd     = 0.0;  // 0x2002 0x03 f32
    float positionpid_windup = 0.0;  // 0x2002 0x04 f32

    float velocitypid_kp     = 0.0;  // 0x2001 0x01 f32
    float velocitypid_ki     = 0.0;  // 0x2001 0x02 f32
    float velocitypid_kd     = 0.0;  // 0x2001 0x03 f32
    float velocitypid_windup = 0.0;  // 0x2001 0x04 f32

    float impedancepd_kp = 0.0;  // 0x200C 0x01 f32
    float impedancepd_kd = 0.0;  // 0x200C 0x02 f32

    std::string homing_mode        = "";  // cf 0x2004 0x06 u32 => hard
    float       homing_maxtravel   = 0.0;
    float       homing_maxtorque   = 0.0;
    float       homing_maxvelocity = 0.0;

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
        else if (fullkey == "motor_maxcurrent")
            motor_maxcurrent = std::stoi(right);
        else if (fullkey == "motor_torquebandwidth")
            motor_torquebandwidth = std::stoi(right);
        else if (fullkey == "motor_shutdowntemp")
            motor_shutdowntemp = std::stoi(right);

        else if (fullkey == "limits_maxtorque")
            limits_maxtorque = std::stod(right);
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

        else if (fullkey == "homing_mode")
            homing_mode = right;
        else if (fullkey == "homing_maxtravel")
            homing_maxtravel = std::stod(right);
        else if (fullkey == "homing_maxtorque")
            homing_maxtorque = std::stod(right);
        else if (fullkey == "homing_maxvelocity")
            homing_maxvelocity = std::stod(right);
    }
    infile.close();

    std::stringstream ss;

    ss << " ---------- value read from config file ---------- " << '\n';
    ss << "motor_name = " << motor_name << '\n'
       << "motor_polepairs = " << motor_polepairs << '\n'
       << "motor_kv = " << motor_kv << '\n'
       << "motor_torqueconstant = " << motor_torqueconstant << '\n'
       << "motor_gearratio = " << motor_gearratio << '\n'
       << "motor_maxcurrent = " << motor_maxcurrent << '\n'
       << "motor_torquebandwidth = " << motor_torquebandwidth << '\n'
       << "motor_shutdowntemp = " << motor_shutdowntemp << "\n\n"

       << "limits_maxtorque = " << limits_maxtorque << '\n'
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
       << "impedancepd_kd = " << impedancepd_kd << "\n\n"

       << "homing_mode = " << homing_mode << '\n'
       << "homing_maxtravel = " << homing_maxtravel << '\n'
       << "homing_maxtorque = " << homing_maxtorque << '\n'
       << "homing_maxvelocity = " << homing_maxvelocity << '\n';
    log.info("%s\n", ss.str().c_str());
    mdco.writeLongOpenRegisters(0x2000, 0x06, motor_name);
    mdco.writeOpenRegisters(0x2000, 0x01, motor_polepairs, 4);
    uint32_t motor_torqueconstant_as_long;
    std::memcpy(&motor_torqueconstant_as_long, &motor_torqueconstant, sizeof(float));
    mdco.writeOpenRegisters(0x2000, 0x02, motor_torqueconstant_as_long, 4);
    uint32_t motor_gearratio_as_long;
    std::memcpy(&motor_gearratio_as_long, &motor_gearratio, sizeof(float));
    mdco.writeOpenRegisters(0x2000, 0x08, motor_gearratio_as_long, 4);
    mdco.writeOpenRegisters(0x2000, 0x05, motor_torquebandwidth, 2);
    mdco.writeOpenRegisters(0x2000, 0x07, motor_shutdowntemp, 1);
    mdco.writeOpenRegisters(0x2005, 0x03, outputencoder_outputencodermode, 1);
    mdco.writeOpenRegisters(0x607D, 0x01, limits_minposition);
    mdco.writeOpenRegisters(0x607D, 0x02, limits_maxposition);
    mdco.writeOpenRegisters(0x6076, 0x00, 1000);
    mdco.writeOpenRegisters(0x6072, 0x00, (long)(1000 * limits_maxtorque));
    mdco.writeOpenRegisters(0x6075, 0x00, 1000);
    mdco.writeOpenRegisters(0x6073, 0x00, (long)(1000 * motor_maxcurrent));
    mdco.writeOpenRegisters(0x6080, 0x00, limits_maxvelocity);
    mdco.writeOpenRegisters(0x60C5, 0x00, limits_maxacceleration);
    mdco.writeOpenRegisters(0x60C6, 0x00, limits_maxdeceleration);
    uint32_t positionpid_kp_as_long;
    std::memcpy(&positionpid_kp_as_long, &positionpid_kp, sizeof(float));
    mdco.writeOpenRegisters(0x2002, 0x01, positionpid_kp_as_long, 4);
    uint32_t positionpid_ki_as_long;
    std::memcpy(&positionpid_ki_as_long, &positionpid_ki, sizeof(float));
    mdco.writeOpenRegisters(0x2002, 0x02, positionpid_ki_as_long, 4);
    uint32_t positionpid_kd_as_long;
    std::memcpy(&positionpid_kd_as_long, &positionpid_kd, sizeof(float));
    mdco.writeOpenRegisters(0x2002, 0x03, positionpid_kd_as_long, 4);
    uint32_t positionpid_windup_as_long;
    std::memcpy(&positionpid_windup_as_long, &positionpid_windup, sizeof(float));
    mdco.writeOpenRegisters(0x2002, 0x04, positionpid_windup_as_long, 4);
    uint32_t velocitypid_kp_as_long;
    std::memcpy(&velocitypid_kp_as_long, &velocitypid_kp, sizeof(float));
    mdco.writeOpenRegisters(0x2001, 0x01, velocitypid_kp_as_long, 4);
    uint32_t velocitypid_ki_as_long;
    std::memcpy(&velocitypid_ki_as_long, &velocitypid_ki, sizeof(float));
    mdco.writeOpenRegisters(0x2001, 0x02, velocitypid_ki_as_long, 4);
    uint32_t velocitypid_kd_as_long;
    std::memcpy(&velocitypid_kd_as_long, &velocitypid_kd, sizeof(float));
    mdco.writeOpenRegisters(0x2001, 0x03, velocitypid_kd_as_long, 4);
    uint32_t velocitypid_windup_as_long;
    std::memcpy(&velocitypid_windup_as_long, &velocitypid_windup, sizeof(float));
    mdco.writeOpenRegisters(0x2001, 0x04, velocitypid_windup_as_long, 4);
    uint32_t impedancepd_kp_as_long;
    std::memcpy(&impedancepd_kp_as_long, &impedancepd_kp, sizeof(float));
    mdco.writeOpenRegisters(0x200C, 0x01, impedancepd_kp, 4);
    uint32_t impedancepd_kd_as_long;
    std::memcpy(&impedancepd_kd_as_long, &impedancepd_kd, sizeof(float));
    mdco.writeOpenRegisters(0x200C, 0x02, impedancepd_kd, 4);
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
    std::vector<u8> frame;
    frame.push_back(0x05);
    long DataSlave;
    u8   bytes1 = ((u8)(MasterId >> 8));
    u8   bytes2 = ((u8)(MasterId));
    u8   bytes3 = ((u8)(HeartbeatTimeout >> 8));
    u8   bytes4 = ((u8)(HeartbeatTimeout));
    DataSlave   = bytes4 + (bytes3 << 8) + (bytes2 << 16) + (bytes1 << 24);
    mdproducer.sendCustomData(0x700 + MasterId, frame);
    if (md.getValueFromOpenRegister(0x1003, 0x00) != 00)
    {
        log.error("The Driver is in fault state before testing the hearbeat");
        return;
    }

    // md.writeOpenRegisters(0x1016, 0x00, 0x1, 1);
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
    if (SyncMessageValue != -1)
    {
        mdco.writeOpenPDORegisters((int)SyncMessageValue, data);
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
    float motor_max_current = 0;
    if (mdco.getValueFromOpenRegister(0x6075, 0x00))
    {
        motor_max_current = mdco.getValueFromOpenRegister(0x6073, 0x00) /
                            mdco.getValueFromOpenRegister(0x6075, 0x00);
    }
    u16 motor_torque_bandwidth = mdco.getValueFromOpenRegister(0x2000, 0x05);
    u32 motor_shutdown_temp    = mdco.getValueFromOpenRegister(0x2000, 0x07);

    /*────────────────────────────
      section [limits]
    ────────────────────────────*/
    float limits_max_torque;
    if (mdco.getValueFromOpenRegister(0x6076, 0x00))
    {
        limits_max_torque = mdco.getValueFromOpenRegister(0x6072, 0x00) /
                            mdco.getValueFromOpenRegister(0x6076, 0x00);
    }
    float limits_max_velocity;
    raw_data = (float)mdco.getValueFromOpenRegister(0x6080, 0x00);
    std::memcpy(&limits_max_velocity, &raw_data, sizeof(float));
    float limits_max_position;
    raw_data = (float)mdco.getValueFromOpenRegister(0x607D, 0x02);
    std::memcpy(&limits_max_position, &raw_data, sizeof(float));
    float limits_min_position;
    raw_data = (float)mdco.getValueFromOpenRegister(0x607D, 0x01);
    std::memcpy(&limits_min_position, &raw_data, sizeof(float));
    float limits_max_acceleration;
    raw_data = (float)mdco.getValueFromOpenRegister(0x60C5, 0x00);
    std::memcpy(&limits_max_acceleration, &raw_data, sizeof(float));
    float limits_max_deceleration;
    raw_data = (float)mdco.getValueFromOpenRegister(0x60C6, 0x00);
    std::memcpy(&limits_max_deceleration, &raw_data, sizeof(float));

    /*────────────────────────────
      section [profile]
    ────────────────────────────*/
    float profile_acceleration;
    raw_data = mdco.getValueFromOpenRegister(0x2008, 0x04);
    std::memcpy(&profile_acceleration, &raw_data, sizeof(float));
    float profile_deceleration;
    raw_data = mdco.getValueFromOpenRegister(0x2008, 0x05);
    std::memcpy(&profile_deceleration, &raw_data, sizeof(float));
    float profile_velocity;
    raw_data = mdco.getValueFromOpenRegister(0x2008, 0x03);
    std::memcpy(&profile_velocity, &raw_data, sizeof(float));

    /*────────────────────────────
      section [output encoder]
    ────────────────────────────*/
    short output_encoder;
    raw_data = mdco.getValueFromOpenRegister(0x2005, 0x01);
    std::memcpy(&output_encoder, &raw_data, sizeof(short));
    short output_encoder_mode;
    raw_data = mdco.getValueFromOpenRegister(0x2005, 0x03);
    std::memcpy(&output_encoder_mode, &raw_data, sizeof(short));

    /*────────────────────────────
      section [position pid]
    ────────────────────────────*/
    float pos_kp;
    raw_data = mdco.getValueFromOpenRegister(0x2002, 0x01);
    std::memcpy(&pos_kp, &raw_data, sizeof(float));
    float pos_ki;
    raw_data = mdco.getValueFromOpenRegister(0x2002, 0x02);
    std::memcpy(&pos_ki, &raw_data, sizeof(float));
    float pos_kd;
    raw_data = mdco.getValueFromOpenRegister(0x2002, 0x03);
    std::memcpy(&pos_kd, &raw_data, sizeof(float));
    float pos_windup;
    raw_data = mdco.getValueFromOpenRegister(0x2002, 0x04);
    std::memcpy(&pos_windup, &raw_data, sizeof(float));

    /*────────────────────────────
      section [velocity pid]
    ────────────────────────────*/
    float vel_kp;
    raw_data = mdco.getValueFromOpenRegister(0x2001, 0x01);
    std::memcpy(&vel_kp, &raw_data, sizeof(float));
    float vel_ki;
    raw_data = mdco.getValueFromOpenRegister(0x2001, 0x02);
    std::memcpy(&vel_ki, &raw_data, sizeof(float));
    float vel_kd;
    raw_data = mdco.getValueFromOpenRegister(0x2001, 0x03);
    std::memcpy(&vel_kd, &raw_data, sizeof(float));
    float vel_windup;
    raw_data = mdco.getValueFromOpenRegister(0x2001, 0x04);
    std::memcpy(&vel_windup, &raw_data, sizeof(float));

    /*────────────────────────────
      section [impedance pd]
    ────────────────────────────*/
    float imp_kp;
    raw_data = mdco.getValueFromOpenRegister(0x200C, 0x01);
    std::memcpy(&imp_kp, &raw_data, sizeof(float));
    float imp_kd;
    raw_data = mdco.getValueFromOpenRegister(0x200C, 0x02);
    std::memcpy(&imp_kd, &raw_data, sizeof(float));

    /*────────────────────────────
      section [homing]
    ────────────────────────────*/
    std::string homing_mode       = "OFF";
    float       homing_max_travel = 10.0;
    float       homing_max_torque = 1.0;
    float       homing_max_vel    = 10.0;
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
    cfg << "max current = " << motor_max_current << '\n';
    cfg << "torque bandwidth = " << motor_torque_bandwidth << '\n';
    cfg << "shutdown temp = " << motor_shutdown_temp << "\n\n";

    // ----- [limits] -----
    cfg << "[limits]\n";
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

    // ----- [homing] -----
    cfg << "[homing]\n";
    cfg << "mode = " << homing_mode << '\n';
    cfg << "max travel = " << homing_max_travel << '\n';
    cfg << "max torque = " << homing_max_torque << '\n';
    cfg << "max velocity = " << homing_max_vel << '\n';

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

void CandleToolCO::testMove(u16 id, f32 targetPosition, moveParameter param)
{
    MDCO mdco            = MDCO(id, m_candle);
    long DesiredPosition = (long)(targetPosition);
    mdco.setProfileParameters(param);
    mdco.enableDriver(CyclicSyncPosition);
    mdco.movePosition(DesiredPosition);
    mdco.disableDriver();
}

void CandleToolCO::testMoveAbsolute(u16 id, i32 targetPos, moveParameter param)
{
    MDCO mdco       = MDCO(id, m_candle);
    long desiredPos = ((long)targetPos);
    mdco.setProfileParameters(param);
    mdco.enableDriver(ProfilePosition);
    mdco.movePosition(desiredPos);
    mdco.disableDriver();
}

void CandleToolCO::testMoveSpeed(u16 id, moveParameter param, i32 DesiredSpeed)
{
    mab::MDCO mdco(id, m_candle);
    mdco.setProfileParameters(param);
    mdco.enableDriver(CyclicSyncVelocity);
    mdco.moveSpeed(DesiredSpeed);
    mdco.disableDriver();
}

void CandleToolCO::testMoveImpedance(u16 id, i32 desiredSpeed, f32 targetPos, moveParameter param)
{
    mab::MDCO mdco(id, m_candle);
    mdco.setProfileParameters(param);
    mdco.enableDriver(Impedance);
    mdco.moveImpedance(desiredSpeed, targetPos, param);
    mdco.disableDriver();
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
    MDCO mdco(id, m_candle);
    mdco.testEncoder(false, true);
}

void CandleToolCO::testEncoderMain(u16 id)
{
    MDCO mdco(id, m_candle);
    mdco.testEncoder(true, false);
}

void CandleToolCO::SDOsegmentedRead(u16 id, u16 reg, u8 subIndex)
{
    MDCO            mdco(id, m_candle);
    std::vector<u8> data;
    mdco.readLongOpenRegisters(reg, subIndex, data);
}

void CandleToolCO::SDOsegmentedWrite(u16 id, u16 reg, u8 subIndex, std::string& data)
{
    MDCO mdco(id, m_candle);
    mdco.writeLongOpenRegisters(reg, subIndex, data);
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
    MDCO mdco(id, m_candle);
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
        // we convert the string value to an unsigned long
        unsigned long data = strtoul((value.c_str()), nullptr, 16);
        // if the data size is 1, 2 or 4 bytes, we write the value
        // to the object dictionary
        mdco.writeOpenRegisters(regAdress, subIndex, data, dataSize, force);
        return;
    }
    else if (dataSize == 8)
    {
        // if data size is over bytes, we need a segmented transfer
        mdco.writeLongOpenRegisters(regAdress, subIndex, value, force);
        return;
    }
    else if (dataSize == 0)
    {
        // if data size is 0, we assume it is a string
        mdco.writeLongOpenRegisters(regAdress, subIndex, value, force);
        return;
    }
    else
    {
        // if the data size is -1, there is an error (e.g. wrong register or subindex)
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

    std::vector<u8> frame;
    frame.reserve(6);
    frame.push_back((u8)millis_since_midnight);
    frame.push_back((u8)(millis_since_midnight >> 8));
    frame.push_back((u8)(millis_since_midnight >> 16));
    frame.push_back((u8)(millis_since_midnight >> 24));
    frame.push_back((u8)(days_since));
    frame.push_back((u8)(days_since >> 8));

    mdco.writeOpenPDORegisters(TimeMessageId, frame);
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
    MDCO mdco(id, m_candle);
    mdco.readOpenRegisters(0x6064, 0);
}

void CandleToolCO::clearErrors(u16 id, const std::string& level)
{
    MDCO mdco(id, m_candle);

    log.info("envoie en canOpen clear error \n");
    if (level == "error")
        mdco.clearOpenErrors(1);
    else if (level == "warning")
        mdco.clearOpenErrors(2);
    else if (level == "all")
        mdco.clearOpenErrors(2);
    else
        log.error("Unknown command");
}

void CandleToolCO::reset(u16 id)
{
    MDCO mdco(id, m_candle);
    mdco.openReset();
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

void CandleToolCO::edsGenerateMarkdown()
{
    edsParser MyEdsParser;
    MyEdsParser.generateMarkdown();
}

void CandleToolCO::edsGenerateHtml()
{
    edsParser MyEdsParser;
    MyEdsParser.generateHtml();
}

void CandleToolCO::edsGenerateCpp()
{
    edsParser MyEdsParser;
    MyEdsParser.generateCpp();
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
    std::vector<u8> data;
    data.reserve(2);
    data.push_back(command);
    data.push_back(id);
    mdco.writeOpenPDORegisters(0x000, data);
}

void CandleToolCO::ReadHeartbeat(u16 id)
{
    MDCO mdco(id, m_candle);
    mdco.testHeartbeat();
}
