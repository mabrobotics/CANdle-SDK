#include "candletoolCO.hpp"

#include <algorithm>
#include <any>
#include <cstdint>
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
#include "mab_types.hpp"
#include "md_types.hpp"
#include "MDStatus.hpp"
#include "canLoader.hpp"
#include "mab_types.hpp"
#include "md_types.hpp"
#include "configHelpers.hpp"
#include "utilities.hpp"
#include "utilities.hpp"

#include "mabFileParser.hpp"
#include "candle_bootloader.hpp"
#include "candle.hpp"
#include "I_communication_interface.hpp"
#include "mab_crc.hpp"

#include "pds.hpp"

using namespace mab;

uint64_t get_time_us()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000000ULL + tv.tv_usec;
}

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

// void CandleToolCO::configCurrent(u16 id, f32 current)
// {
//     MD   md        = MD(id, m_candle);
//     auto connected = md.init();
//     if (connected != MD::Error_t::OK)
//     {
//         log.error("Could not connect MD with id %d", id);
//         return;
//     }
//     auto result = md.setCurrentLimit(current);

//     if (result != MD::Error_t::OK)
//     {
//         log.error("Failed to set max current for the driver with id %d!", id);
//     }
//     else
//     {
//         log.info("Max current set successfully!");
//     }
//     return;
// }

void CandleToolCO::configBandwidth(u16 id, f32 bandwidth)
{
    MDCO mdco = MDCO(id, m_candle);

    u16 newBandwidth = ((u16)bandwidth);
    mdco.CanOpenBandwidth(newBandwidth);
}

void CandleToolCO::sendPdoSpeed(u16 id, i32 desiredSpeed)
{
    MDCO md = MDCO(id, m_candle);
    log.info("Sending SDO for motor setup!");
    md.WriteOpenRegisters("Max Current", 500, 2);
    md.WriteOpenRegisters("Motor Rated Current", 1000, 4);
    md.WriteOpenRegisters("Max Motor Speed", 200, 4);
    md.WriteOpenRegisters("Motor Max Torque", 500, 2);
    md.WriteOpenRegisters("Motor Rated Torque", 1000, 4);
    md.WriteOpenRegisters("Modes Of Operation", 9, 1);
    md.WriteOpenRegisters("Controlword", 0x80, 2);
    md.WriteOpenRegisters("Controlword", 0x06, 2);
    md.WriteOpenRegisters("Controlword", 15, 2);
    log.info("Sending PDO for speed loop control");
    std::vector<u8> frameSetup;
    frameSetup.push_back(0x0F);
    frameSetup.push_back(0x00);
    frameSetup.push_back(0x09);
    md.WriteOpenPDORegisters(0x300 + id, frameSetup);
    std::vector<u8> frameSpeed;
    frameSpeed.push_back(0x0F);
    frameSpeed.push_back(0x00);
    frameSpeed.push_back((u8)(desiredSpeed));
    frameSpeed.push_back((u8)(desiredSpeed >> 8));
    frameSpeed.push_back((u8)(desiredSpeed >> 16));
    frameSpeed.push_back((u8)(desiredSpeed >> 24));
    md.WriteOpenPDORegisters(0x500 + id, frameSpeed);
    usleep(5000000);
    if ((int)md.GetValueFromOpenRegister(0x606C, 0x00) <= desiredSpeed + 5 &&
        (int)md.GetValueFromOpenRegister(0x606C, 0x00) >= desiredSpeed - 5)
    {
        log.success("Velocity Target reached with +/- 5RPM");
    }
    else
    {
        log.error("Velocity Target not reached");
    }
    md.WriteOpenRegisters("Motor Target Velocity", 0, 4);
    md.WriteOpenRegisters("Controlword", 6, 2);
    md.WriteOpenRegisters("Modes Of Operation", 0, 1);
}

void CandleToolCO::sendPdoPosition(u16 id, i32 DesiredPos)
{
    MDCO md = MDCO(id, m_candle);
    log.info("Sending SDO for motor setup!");
    md.WriteOpenRegisters("Max Current", 500, 2);
    md.WriteOpenRegisters("Motor Rated Current", 1000, 4);
    md.WriteOpenRegisters("Max Motor Speed", 200, 4);
    md.WriteOpenRegisters("Motor Max Torque", 500, 2);
    md.WriteOpenRegisters("Motor Rated Torque", 1000, 4);
    md.WriteOpenRegisters("Modes Of Operation", 8, 1);
    md.WriteOpenRegisters("Controlword", 0x80, 2);
    md.WriteOpenRegisters("Controlword", 0x06, 2);
    md.WriteOpenRegisters("Controlword", 15, 2);

    usleep(1000);

    log.info("Sending PDO for speed loop control");

    std::vector<u8> frameSetup;
    frameSetup.push_back(0x0F);
    frameSetup.push_back(0x00);
    frameSetup.push_back(0x08);
    md.WriteOpenPDORegisters(0x300 + id, frameSetup);
    std::vector<u8> framePosition;
    framePosition.push_back(0x0F);
    framePosition.push_back(0x00);
    framePosition.push_back((u8)(DesiredPos));
    framePosition.push_back((u8)(DesiredPos >> 8));
    framePosition.push_back((u8)(DesiredPos >> 16));
    framePosition.push_back((u8)(DesiredPos >> 24));

    log.debug("position ask : %d\n", DesiredPos);

    time_t start = time(nullptr);

    while (time(nullptr) - start < 5 &&
           !((int)md.GetValueFromOpenRegister(0x6064, 0) > (DesiredPos - 100) &&
             (int)md.GetValueFromOpenRegister(0x6064, 0) < (DesiredPos + 100)))
    {
        md.WriteOpenPDORegisters(0x400 + id, framePosition);
        usleep(1000);
    }

    log.debug("position actual : %d\n", (int)md.GetValueFromOpenRegister(0x6064, 0));

    if (((int)md.GetValueFromOpenRegister(0x6064, 0) > (DesiredPos - 200) &&
         (int)md.GetValueFromOpenRegister(0x6064, 0) < (DesiredPos + 200)))
    {
        log.success("Position reached in less than 5s");
    }
    else
    {
        log.error("Position not reached in less than 5s");
    }

    md.WriteOpenRegisters("Controlword", 6, 2);
    md.WriteOpenRegisters("Modes Of Operation", 0, 1);
}

void CandleToolCO::SendCustomPdo(u16 id, const edsObject& Desiregister, u64 data)
{
    // TO DO: finish this
    MDCO mdco = MDCO(id, m_candle);
    // if the the index start with 0x1A00
    if (Desiregister.index == (0x1600))
    {
        std::vector<u8> frame;
        frame.push_back((u8)(data >> 8));
        frame.push_back(data);
        mdco.WriteOpenPDORegisters(0x200 + id, frame);
    }
    else if (Desiregister.index == (0x1601))
    {
        std::vector<u8> frame;
        frame.push_back((u8)(data >> 16));
        frame.push_back((u8)(data >> 8));
        frame.push_back((u8)data);
        mdco.WriteOpenPDORegisters(0x300 + id, frame);
    }
    else if (Desiregister.index == (0x1602))
    {
        std::vector<u8> frame;
        frame.push_back((u8)(data >> 40));
        frame.push_back((u8)(data >> 32));
        frame.push_back((u8)(data >> 24));
        frame.push_back((u8)(data >> 16));
        frame.push_back((u8)(data >> 8));
        frame.push_back((u8)data);
        mdco.WriteOpenPDORegisters(0x400 + id, frame);
    }
    else if (Desiregister.index == (0x1603))
    {
        std::vector<u8> frame;
        frame.push_back((u8)(data >> 40));
        frame.push_back((u8)(data >> 32));
        frame.push_back((u8)(data >> 24));
        frame.push_back((u8)(data >> 16));
        frame.push_back((u8)(data >> 8));
        frame.push_back((u8)data);
        mdco.WriteOpenPDORegisters(0x500 + id, frame);
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

    int limits_maxtorque       = 0;  // not usefull because it's not save after motor shutdown
    int limits_maxvelocity     = 0;  // not usefull because it's not save after motor shutdown
    int limits_maxposition     = 0;  // not usefull because it's not save after motor shutdown
    int limits_minposition     = 0;  // not usefull because it's not save after motor shutdown
    int limits_maxacceleration = 0;  // not usefull because it's not save after motor shutdown
    int limits_maxdeceleration = 0;  // not usefull because it's not save after motor shutdown

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
            section = clean(line.substr(1, line.size() - 2));
            continue;
        }
        std::istringstream iss(line);
        std::string        left, right;
        if (!std::getline(iss, left, '='))
            continue;
        if (!std::getline(iss, right))
            continue;
        left  = clean(left);
        right = clean(right);

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
            limits_maxtorque = std::stoi(right);
        else if (fullkey == "limits_maxvelocity")
            limits_maxvelocity = std::stoi(right);
        else if (fullkey == "limits_maxposition")
            limits_maxposition = std::stoi(right);
        else if (fullkey == "limits_minposition")
            limits_minposition = std::stoi(right);
        else if (fullkey == "limits_maxacceleration")
            limits_maxacceleration = std::stoi(right);
        else if (fullkey == "limits_maxdeceleration")
            limits_maxdeceleration = std::stoi(right);

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
            positionpid_kp = std::stoi(right);
        else if (fullkey == "positionpid_ki")
            positionpid_ki = std::stod(right);
        else if (fullkey == "positionpid_kd")
            positionpid_kd = std::stoi(right);
        else if (fullkey == "positionpid_windup")
            positionpid_windup = std::stoi(right);

        else if (fullkey == "velocitypid_kp")
            velocitypid_kp = std::stoi(right);
        else if (fullkey == "velocitypid_ki")
            velocitypid_ki = std::stod(right);
        else if (fullkey == "velocitypid_kd")
            velocitypid_kd = std::stoi(right);
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

    std::cout << " ---------- value read from config file ---------- " << std::endl;
    std::cout << "motor_name = " << motor_name << '\n'
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
    mdco.WriteLongOpenRegisters(0x2000, 0x06, motor_name);
    mdco.WriteOpenRegisters(0x2000, 0x01, motor_polepairs, 4);
    uint32_t motor_torqueconstant_as_long;
    std::memcpy(&motor_torqueconstant_as_long, &motor_torqueconstant, sizeof(float));
    mdco.WriteOpenRegisters(0x2000, 0x02, motor_torqueconstant_as_long, 4);
    uint32_t motor_gearratio_as_long;
    std::memcpy(&motor_gearratio_as_long, &motor_gearratio, sizeof(float));
    mdco.WriteOpenRegisters(0x2000, 0x08, motor_gearratio_as_long, 4);
    mdco.WriteOpenRegisters(0x2000, 0x05, motor_torquebandwidth, 2);
    mdco.WriteOpenRegisters(0x2000, 0x07, motor_shutdowntemp, 1);
    mdco.WriteOpenRegisters(0x2005, 0x03, outputencoder_outputencodermode, 1);
    uint32_t positionpid_kp_as_long;
    std::memcpy(&positionpid_kp_as_long, &positionpid_kp, sizeof(float));
    mdco.WriteOpenRegisters(0x2002, 0x01, positionpid_kp_as_long, 4);
    uint32_t positionpid_ki_as_long;
    std::memcpy(&positionpid_ki_as_long, &positionpid_ki, sizeof(float));
    mdco.WriteOpenRegisters(0x2002, 0x02, positionpid_ki_as_long, 4);
    uint32_t positionpid_kd_as_long;
    std::memcpy(&positionpid_kd_as_long, &positionpid_kd, sizeof(float));
    mdco.WriteOpenRegisters(0x2002, 0x03, positionpid_kd_as_long, 4);
    uint32_t positionpid_windup_as_long;
    std::memcpy(&positionpid_windup_as_long, &positionpid_windup, sizeof(float));
    mdco.WriteOpenRegisters(0x2002, 0x04, positionpid_windup_as_long, 4);
    uint32_t velocitypid_kp_as_long;
    std::memcpy(&velocitypid_kp_as_long, &velocitypid_kp, sizeof(float));
    mdco.WriteOpenRegisters(0x2001, 0x01, velocitypid_kp_as_long, 4);
    uint32_t velocitypid_ki_as_long;
    std::memcpy(&velocitypid_ki_as_long, &velocitypid_ki, sizeof(float));
    mdco.WriteOpenRegisters(0x2001, 0x02, velocitypid_ki_as_long, 4);
    uint32_t velocitypid_kd_as_long;
    std::memcpy(&velocitypid_kd_as_long, &velocitypid_kd, sizeof(float));
    mdco.WriteOpenRegisters(0x2001, 0x03, velocitypid_kd_as_long, 4);
    uint32_t velocitypid_windup_as_long;
    std::memcpy(&velocitypid_windup_as_long, &velocitypid_windup, sizeof(float));
    mdco.WriteOpenRegisters(0x2001, 0x04, velocitypid_windup_as_long, 4);
    uint32_t impedancepd_kp_as_long;
    std::memcpy(&impedancepd_kp_as_long, &impedancepd_kp, sizeof(float));
    mdco.WriteOpenRegisters(0x200C, 0x01, impedancepd_kp, 4);
    uint32_t impedancepd_kd_as_long;
    std::memcpy(&impedancepd_kd_as_long, &impedancepd_kd, sizeof(float));
    mdco.WriteOpenRegisters(0x200C, 0x02, impedancepd_kd, 4);
}

std::string CandleToolCO::clean(std::string s)
{
    auto notSpace = [](unsigned char c) { return !std::isspace(c); };
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), notSpace));
    s.erase(std::find_if(s.rbegin(), s.rend(), notSpace).base(), s.end());
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return std::tolower(c); });
    s.erase(std::remove_if(s.begin(), s.end(), [](unsigned char c) { return std::isspace(c); }),
            s.end());
    return s;
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
    mdproducer.SendCustomData(0x700 + MasterId, frame);
    if (md.GetValueFromOpenRegister(0x1003, 0x00) != 00)
    {
        log.error("The Driver is in fault state before testing the hearbeat");
        return;
    }

    // md.WriteOpenRegisters(0x1016, 0x00, 0x1, 1);
    md.WriteOpenRegisters(0x1016, 0x01, DataSlave, 4);
    mdproducer.SendCustomData(0x700 + MasterId, frame);
    usleep(HeartbeatTimeout);
    if (md.GetValueFromOpenRegister(0x1001, 0) != 0)
    {
        log.error("The driver enter fault mode before the Heartbeat timeout");
        return;
    }
    usleep(2000 * HeartbeatTimeout);

    if (md.GetValueFromOpenRegister(0x1001, 0) != 0)
    {
        log.success("The driver enter fault mode after the Heartbeat timeout");
        return;
    }
    mdproducer.SendCustomData(0x700 + MasterId, frame);
    if (md.GetValueFromOpenRegister(0x1001, 0) != 0)
    {
        log.error("The driver still in error mode");
    }
}

void CandleToolCO::SendSync(u16 id)
{
    MDCO            mdco(id, m_candle);
    long            SyncMessageValue = mdco.GetValueFromOpenRegister(0x1005, 0x0);
    std::vector<u8> data;
    if (SyncMessageValue != -1)
    {
        mdco.WriteOpenPDORegisters((int)SyncMessageValue, data);
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
    mdco.ReadLongOpenRegisters(0x2000, 0x06, motor_name);
    int   motor_kv         = 100;
    u32   motor_pole_pairs = mdco.GetValueFromOpenRegister(0x2000, 0x01);
    float motor_torque_constant;
    raw_data = (float)mdco.GetValueFromOpenRegister(0x2000, 0x02);
    std::memcpy(&motor_torque_constant, &raw_data, sizeof(float));
    raw_data = (float)mdco.GetValueFromOpenRegister(0x2000, 0x08);
    float motor_gear_ratio;
    std::memcpy(&motor_gear_ratio, &raw_data, sizeof(float));
    float motor_max_current = 0;
    if (mdco.GetValueFromOpenRegister(0x6075, 0x00))
    {
        motor_max_current = mdco.GetValueFromOpenRegister(0x6073, 0x00) /
                            mdco.GetValueFromOpenRegister(0x6075, 0x00);
    }
    u16 motor_torque_bandwidth = mdco.GetValueFromOpenRegister(0x2000, 0x05);
    u32 motor_shutdown_temp    = mdco.GetValueFromOpenRegister(0x2000, 0x07);

    /*────────────────────────────
      section [limits]
    ────────────────────────────*/
    float limits_max_torque;
    raw_data = mdco.GetValueFromOpenRegister(0x2007, 0x01);
    std::memcpy(&limits_max_torque, &raw_data, sizeof(float));
    float limits_max_velocity;
    raw_data = mdco.GetValueFromOpenRegister(0x2007, 0x04);
    std::memcpy(&limits_max_velocity, &raw_data, sizeof(float));
    float limits_max_position;
    raw_data = mdco.GetValueFromOpenRegister(0x2007, 0x05);
    std::memcpy(&limits_max_position, &raw_data, sizeof(float));
    float limits_min_position;
    raw_data = mdco.GetValueFromOpenRegister(0x2007, 0x06);
    std::memcpy(&limits_min_position, &raw_data, sizeof(float));
    float limits_max_acceleration;
    raw_data = mdco.GetValueFromOpenRegister(0x2007, 0x02);
    std::memcpy(&limits_max_acceleration, &raw_data, sizeof(float));
    float limits_max_deceleration;
    raw_data = mdco.GetValueFromOpenRegister(0x2007, 0x03);
    std::memcpy(&limits_max_deceleration, &raw_data, sizeof(float));

    /*────────────────────────────
      section [profile]
    ────────────────────────────*/
    float profile_acceleration;
    raw_data = mdco.GetValueFromOpenRegister(0x2008, 0x04);
    std::memcpy(&profile_acceleration, &raw_data, sizeof(float));
    float profile_deceleration;
    raw_data = mdco.GetValueFromOpenRegister(0x2008, 0x05);
    std::memcpy(&profile_deceleration, &raw_data, sizeof(float));
    float profile_velocity;
    raw_data = mdco.GetValueFromOpenRegister(0x2008, 0x03);
    std::memcpy(&profile_velocity, &raw_data, sizeof(float));

    /*────────────────────────────
      section [output encoder]
    ────────────────────────────*/
    short output_encoder;
    raw_data = mdco.GetValueFromOpenRegister(0x2005, 0x01);
    std::memcpy(&output_encoder, &raw_data, sizeof(short));
    short output_encoder_mode;
    raw_data = mdco.GetValueFromOpenRegister(0x2005, 0x03);
    std::memcpy(&output_encoder_mode, &raw_data, sizeof(short));

    /*────────────────────────────
      section [position pid]
    ────────────────────────────*/
    float pos_kp;
    raw_data = mdco.GetValueFromOpenRegister(0x2002, 0x01);
    std::memcpy(&pos_kp, &raw_data, sizeof(float));
    float pos_ki;
    raw_data = mdco.GetValueFromOpenRegister(0x2002, 0x02);
    std::memcpy(&pos_ki, &raw_data, sizeof(float));
    float pos_kd;
    raw_data = mdco.GetValueFromOpenRegister(0x2002, 0x03);
    std::memcpy(&pos_kd, &raw_data, sizeof(float));
    float pos_windup;
    raw_data = mdco.GetValueFromOpenRegister(0x2002, 0x04);
    std::memcpy(&pos_windup, &raw_data, sizeof(float));

    /*────────────────────────────
      section [velocity pid]
    ────────────────────────────*/
    float vel_kp;
    raw_data = mdco.GetValueFromOpenRegister(0x2001, 0x01);
    std::memcpy(&vel_kp, &raw_data, sizeof(float));
    float vel_ki;
    raw_data = mdco.GetValueFromOpenRegister(0x2001, 0x02);
    std::memcpy(&vel_ki, &raw_data, sizeof(float));
    float vel_kd;
    raw_data = mdco.GetValueFromOpenRegister(0x2001, 0x03);
    std::memcpy(&vel_kd, &raw_data, sizeof(float));
    float vel_windup;
    raw_data = mdco.GetValueFromOpenRegister(0x2001, 0x04);
    std::memcpy(&vel_windup, &raw_data, sizeof(float));

    /*────────────────────────────
      section [impedance pd]
    ────────────────────────────*/
    float imp_kp;
    raw_data = mdco.GetValueFromOpenRegister(0x200C, 0x01);
    std::memcpy(&imp_kp, &raw_data, sizeof(float));
    float imp_kd;
    raw_data = mdco.GetValueFromOpenRegister(0x200C, 0x02);
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
        long devicetype = mdco.GetValueFromOpenRegister(0x1000, 0);
        log.info("Device type:", devicetype);
        long int    hexValue = mdco.GetValueFromOpenRegister(0x1008, 0);
        std::string motorName;
        for (int i = 0; i <= 3; i++)
        {
            char c = (hexValue >> (8 * i)) & 0xFF;
            motorName += c;
        }
        log.info("Manufacturer Device Name: %s", motorName.c_str());
        long Firmware = mdco.GetValueFromOpenRegister(0x200A, 3);
        log.info("Firmware version: %li", Firmware);
        long Bootloder = mdco.GetValueFromOpenRegister(0x200B, 4);
        log.info("Bootloder version: %li", Bootloder);
    }
    else
    {
        mdco.printAllInfo();
    }
}

void CandleToolCO::testMove(u16 id,
                            f32 targetPosition,
                            u32 MaxSpeed,
                            u16 MaxCurrent,
                            u32 RatedCurrent,
                            u16 MaxTorque,
                            u32 RatedTorque)
{
    MDCO mdco = MDCO(id, m_candle);

    long DesiredPosition = (long)(targetPosition);
    mdco.movePosition(MaxCurrent, RatedCurrent, MaxTorque, RatedTorque, MaxSpeed, DesiredPosition);
}

void CandleToolCO::testMoveAbsolute(u16 id,
                                    f32 targetPos,
                                    f32 velLimit,
                                    f32 accLimit,
                                    f32 dccLimit,
                                    u16 MaxCurrent,
                                    u32 RatedCurrent,
                                    u16 MaxTorque,
                                    u32 RatedTorque)
{
    MDCO mdco = MDCO(id, m_candle);

    long desiredPos       = ((long)targetPos);
    long MaxSpeed         = ((long)velLimit);
    long MaxAcceleration  = ((long)accLimit);
    long MaxDecceleration = ((long)dccLimit);
    mdco.movePositionAcc(desiredPos,
                         MaxAcceleration,
                         MaxDecceleration,
                         MaxSpeed,
                         MaxCurrent,
                         RatedCurrent,
                         MaxTorque,
                         RatedTorque);
}

void CandleToolCO::testMoveSpeed(u16 id,
                                 u16 MaxCurrent,
                                 u32 RatedCurrent,
                                 u16 MaxTorque,
                                 u32 RatedTorque,
                                 u32 MaxSpeed,
                                 i32 DesiredSpeed)
{
    mab::MDCO md(id, m_candle);
    md.moveSpeed(MaxCurrent, RatedCurrent, MaxTorque, RatedTorque, MaxSpeed, DesiredSpeed);
}

void CandleToolCO::testMoveImpedance(u16 id,
                                     i32 desiredSpeed,
                                     f32 targetPos,
                                     f32 kp,
                                     f32 kd,
                                     i16 torque,
                                     u32 MaxSpeed,
                                     u16 MaxCurrent,
                                     u32 RatedCurrent,
                                     u16 MaxTorque,
                                     u32 RatedTorque)
{
    mab::MDCO md(id, m_candle);
    md.moveImpedance(desiredSpeed,
                     targetPos,
                     kp,
                     kd,
                     torque,
                     MaxSpeed,
                     MaxCurrent,
                     RatedCurrent,
                     MaxTorque,
                     RatedTorque);
}

void CandleToolCO::testLatency(u16 id)
{
    MDCO     md(id, m_candle);
    uint64_t latence_totale = 0;
    bool     testOk         = true;

    for (int i = 0; i < 100; ++i)
    {
        uint64_t start_time = get_time_us();

        if (md.ReadOpenRegisters(0x1000, 0) != MDCO::Error_t::OK)
        {
            testOk = false;
        }
        uint64_t end_time = get_time_us();
        latence_totale += (end_time - start_time);
    }
    if (testOk)
    {
        uint64_t moyenne = latence_totale / 100;
        log.info("---------------Latence---------------\n");
        log.info("Total: %lu µs\n", latence_totale);
        log.info("Result (average of 10000 attempts): %lu µs\n", moyenne);
    }
    else
    {
        log.error("MD driver not answering");
    }
}

void CandleToolCO::testEncoderOutput(u16 id)
{
    MDCO mdco(id, m_candle);
    mdco.testencoder(false, true);
}

void CandleToolCO::testEncoderMain(u16 id)
{
    MDCO mdco(id, m_candle);
    mdco.testencoder(true, false);
}

void CandleToolCO::SDOsegmentedRead(u16 id, u16 reg, u8 subIndex)
{
    MDCO            mdco(id, m_candle);
    std::vector<u8> data;
    mdco.ReadLongOpenRegisters(reg, subIndex, data);
}

void CandleToolCO::SDOsegmentedWrite(u16 id, u16 reg, u8 subIndex, std::string& data)
{
    MDCO mdco(id, m_candle);
    mdco.WriteLongOpenRegisters(reg, subIndex, data);
}

void CandleToolCO::registerRead(u16 id, u16 regAdress, u8 subIndex, bool force)
{
    MDCO            mdco(id, m_candle);
    std::vector<u8> data;
    int             dataSize = mdco.DataSizeOfEdsObject(regAdress, subIndex);
    if (!force)
    {
        if (dataSize == 1 || dataSize == 2 || dataSize == 4)
            mdco.ReadOpenRegisters(regAdress, subIndex, force);
        else if (dataSize == 8 || dataSize == 0)
            mdco.ReadLongOpenRegisters(regAdress, subIndex, data);
        else
        {
            log.error("Unknown register size for register 0x%04X subindex %d", regAdress, subIndex);
        }
    }
    else
        mdco.ReadOpenRegisters(regAdress, subIndex, force);
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
            dataSize = mdco.DataSizeOfEdsObject(regAdress, subIndex);
    }

    if (dataSize == 1 || dataSize == 2 || dataSize == 4)
    {
        // we convert the string value to an unsigned long
        unsigned long data = strtoul((value.c_str()), nullptr, 16);
        // if the data size is 1, 2 or 4 bytes, we write the value
        // to the object dictionary
        mdco.WriteOpenRegisters(regAdress, subIndex, data, dataSize, force);
        return;
    }
    else if (dataSize == 8)
    {
        // if data size is over bytes, we need a segmented transfer
        mdco.WriteLongOpenRegisters(regAdress, subIndex, value, force);
        return;
    }
    else if (dataSize == 0)
    {
        // if data size is 0, we assume it is a string
        mdco.WriteLongOpenRegisters(regAdress, subIndex, value, force);
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
    MDCO            mdco(id, m_candle);
    time_t          now = time(NULL);
    time_t          Begin;
    struct tm       datetime = *localtime(&now);
    int             diff;
    long            TimeMessageId;
    long            NumberOfMillis = 0;
    long            NumberOfDays   = 0;
    std::vector<u8> frame;
    datetime.tm_year  = 84;
    datetime.tm_mon   = 0;
    datetime.tm_mday  = 1;
    datetime.tm_hour  = 0;
    datetime.tm_min   = 0;
    datetime.tm_sec   = 0;
    datetime.tm_isdst = -1;
    Begin             = mktime(&datetime);
    diff              = difftime(now, Begin);
    NumberOfDays      = (diff / 86400);
    datetime          = *localtime(&now);
    NumberOfMillis =
        (datetime.tm_hour * 3600000 + datetime.tm_sec * 1000 + datetime.tm_min * 60000);
    log.info("The actual time according to your computer is: %s", asctime(&datetime));
    log.info("Number of days since 1st January 1984: %d", NumberOfDays);
    log.info("Number of millis since midnight: %d", NumberOfMillis);
    TimeMessageId = mdco.GetValueFromOpenRegister(0x1012, 0x00);
    frame.push_back((u8)NumberOfMillis);
    frame.push_back((u8)(NumberOfMillis >> 8));
    frame.push_back((u8)(NumberOfMillis >> 16));
    frame.push_back((u8)((NumberOfMillis >> 24)));
    frame.push_back((u8)(NumberOfDays));
    frame.push_back((u8)(NumberOfDays >> 8));
    mdco.WriteOpenPDORegisters(TimeMessageId, frame);
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
    mdco.ReadOpenRegisters(0x6064, 0);
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
    mdco.OpenReset();
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

/* gets field only if the value is within bounds form the ini file */
template <class T>
bool CandleToolCO::getField(mINI::INIStructure& cfg,
                            mINI::INIStructure& ini,
                            std::string         category,
                            std::string         field,
                            T&                  value)
{
    T min = 0;
    T max = 0;

    if (std::is_same<T, std::uint16_t>::value || std::is_same<T, std::uint8_t>::value ||
        std::is_same<T, std::int8_t>::value || std::is_same<T, std::uint32_t>::value)
    {
        value = atoi(cfg[category][field].c_str());
        min   = atoi(ini["limit min"][field].c_str());
        max   = atoi(ini["limit max"][field].c_str());
    }
    else if (std::is_same<T, std::float_t>::value)
    {
        value = strtof(cfg[category][field].c_str(), nullptr);
        min   = strtof(ini["limit min"][field].c_str(), nullptr);
        max   = strtof(ini["limit max"][field].c_str(), nullptr);
    }

    if (checkParamLimit(value, min, max))
        return true;
    else
    {
        log.error("Parameter [%s][%s] is out of bounds! Min: %.3f, max:%.3f, value: %.3f",
                  category.c_str(),
                  field.c_str(),
                  (f32)min,
                  (f32)max,
                  (f32)value);
        return false;
    }
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
    data.push_back(command);
    data.push_back(id);
    mdco.WriteOpenPDORegisters(0x000, data);
}

void CandleToolCO::ReadHeartbeat(u16 id)
{
    // TODO: find a better way to do this, it seems to work but it's clearly not the best way to do
    // it. A better way could be by implemented a listen mode on the USB.cpp file
    uint32_t heartbeat_id = 0x700 + id;
    log.info("Attente d'un heartbeat sur l'ID CAN 0x%03X...", heartbeat_id);
    uint64_t             firstHeartbeatReveiceved = 0;
    uint64_t             start_time               = get_time_us();
    const uint64_t       timeout_us               = 5 * 1000000;  // 5 secondes
    std::vector<uint8_t> frame;
    frame.push_back(0);
    std::vector<uint8_t>      response;
    mab::candleTypes::Error_t error;
    // if after 5 second no heartbeat receive then give the user an error(fail)
    while (get_time_us() - start_time < timeout_us)
    {
        // send Heartbeat Canopen frame with high frequencies
        auto result = m_candle->transferCANPDOFrame(heartbeat_id, frame, 1, /*timeoutMs=*/10);
        response    = result.first;
        error       = result.second;
        // if a correct message is received
        if (error == mab::candleTypes::Error_t::OK && response.size() >= 3)
        {
            // if the message is a Heartbeat
            if (response[0] == 0x04 && response[1] == 0x01 && response[2] == 0x05)
            {
                log.success("heartbeat reçu");
                // if it's the first time we received an heartbeat
                if (firstHeartbeatReveiceved == 0)
                {
                    firstHeartbeatReveiceved = get_time_us();
                    result =
                        m_candle->transferCANPDOFrame(heartbeat_id, frame, 1, /*timeoutMs=*/10);
                    response = result.first;
                    error    = result.second;
                    // keep sending message until the last heartbeat diseapear on the bus
                    while ((response.size() > 1 && response[1] == 0x01) &&
                           (get_time_us() - start_time < timeout_us))
                    {
                        result =
                            m_candle->transferCANPDOFrame(heartbeat_id, frame, 1, /*timeoutMs=*/10);
                        response = result.first;
                        error    = result.second;
                        // if we lost communication with the MD
                        if (error != mab::candleTypes::Error_t::OK || response.size() <= 1)
                        {
                            break;
                        }
                    }
                }
                else
                {
                    // if it's the second heartbeat we receive, then we calcul the delta time and we
                    // stop the function with a succes. the delta value is in [s] and should be
                    // around the stock in the register
                    log.success(
                        "heartbeat received with in between time of %lfs",
                        static_cast<double>(get_time_us() - firstHeartbeatReveiceved) / 1000000.0);
                    return;
                }
            }
        }
    }
    // if zero heartbeat message have been received then we log an error
    if (firstHeartbeatReveiceved == 0)
        log.error("Aucun heartbeat reçu après 5s.\n");
    // if only one heartbeat has been received in the last 5s we quit with a success
    else
        log.success("One heartbeat has been received in the last 5s");
    // log.info("Work in progress.\n");
}
