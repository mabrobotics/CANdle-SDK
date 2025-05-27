#include "candletool.hpp"

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
#include "ui.hpp"
#include "configHelpers.hpp"
#include "utilities.hpp"
#include "utilities.hpp"

#include "mabFileParser.hpp"
#include "candle_bootloader.hpp"
#include "candle.hpp"
#include "I_communication_interface.hpp"
#include "mab_crc.hpp"

#include "pds.hpp"
#include "candle_bootloader.hpp"
#include "candle.hpp"
#include "I_communication_interface.hpp"
#include "mab_crc.hpp"

using namespace mab;

f32 lerp(f32 start, f32 end, f32 t)
{
    return (start * (1.f - t)) + (end * t);
}

mab::CANdleBaudrate_E str2baud(const std::string& baud)
{
    if (baud == "1M")
        return mab::CANdleBaudrate_E::CAN_BAUD_1M;
    if (baud == "2M")
        return mab::CANdleBaudrate_E::CAN_BAUD_2M;
    if (baud == "5M")
        return mab::CANdleBaudrate_E::CAN_BAUD_5M;
    if (baud == "8M")
        return mab::CANdleBaudrate_E::CAN_BAUD_8M;
    return mab::CANdleBaudrate_E::CAN_BAUD_1M;
}

CandleTool::CandleTool(const mab::CANdleBaudrate_E baud)
{
    log.m_tag   = "CANDLETOOL";
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

CandleTool::~CandleTool()
{
    detachCandle(m_candle);
}

void CandleTool::ping(const std::string& variant)
{
    if (variant == "all")
    {
        // TODO: implement all variant later, change string variant to enum to avoid undefined
        log.error("Not implemented");
        // TODO: implement all variant later, change string variant to enum to avoid undefined
        log.error("Not implemented");
        return;
    }
    auto mdIds = MD::discoverMDs(m_candle);
    log.info("Discovered MDs: ");
    for (const auto& id : mdIds)
    {
        log.info("- %d", id);
    }
}

void CandleTool::configCan(
    u16 id, u16 newId, const std::string& baud, u16 timeout, bool termination)
{
    MDRegisters_S mdRegisters;
    MD            md        = MD(id, m_candle);
    auto          connected = md.init();
    if (connected != MD::Error_t::OK)
    {
        log.error("Could not connect MD with id %d", id);
        return;
    }
    mdRegisters.canID        = newId;
    mdRegisters.canBaudrate  = str2baud(baud);
    mdRegisters.canWatchdog  = timeout;
    mdRegisters.runCanReinit = 1;
    auto result              = md.writeRegisters(mdRegisters.canID,
                                    mdRegisters.canBaudrate,
                                    mdRegisters.canWatchdog,
                                    mdRegisters.runCanReinit);

    if (result != MD::Error_t::OK)
    {
        log.error("Failed to setup can parameters for driver with id %d!", id);
    }
    else
    {
        log.info("Can parameter set successful!");
    }
    return;
}
void CandleTool::configSave(u16 id)
{
    MD   md        = MD(id, m_candle);
    auto connected = md.init();
    if (connected != MD::Error_t::OK)
    {
        log.error("Could not connect MD with id %d", id);
        return;
    }
    auto result = md.save();

    if (result != MD::Error_t::OK)
    {
        log.error("Failed to save config for driver with id %d!", id);
    }
    else
    {
        log.info("Config saved successfully!");
    }
    return;
}

void CandleTool::configZero(u16 id)
{
    MD   md        = MD(id, m_candle);
    auto connected = md.init();
    if (connected != MD::Error_t::OK)
    {
        log.error("Could not connect MD with id %d", id);
        return;
    }
    auto result = md.zero();

    if (result != MD::Error_t::OK)
    {
        log.error("Failed to zero the driver with id %d!", id);
    }
    else
    {
        log.info("Drive %d zeroed", id);
    }
    return;
}

void CandleTool::configCurrent(u16 id, f32 current)
{
    MD   md        = MD(id, m_candle);
    auto connected = md.init();
    if (connected != MD::Error_t::OK)
    {
        log.error("Could not connect MD with id %d", id);
        return;
    }
    auto result = md.setCurrentLimit(current);

    if (result != MD::Error_t::OK)
    {
        log.error("Failed to set max current for the driver with id %d!", id);
    }
    else
    {
        log.info("Max current set successfully!");
    }
    return;
}

void CandleTool::configBandwidth(u16 id, f32 bandwidth)
{
    MDRegisters_S mdRegisters;
    MD            md        = MD(id, m_candle);
    auto          connected = md.init();
    if (connected != MD::Error_t::OK)
    {
        log.error("Could not connect MD with id %d", id);
        return;
    }
    mdRegisters.motorTorqueBandwidth = bandwidth;
    mdRegisters.runCalibratePiGains  = 1;
    auto result =
        md.writeRegisters(mdRegisters.motorTorqueBandwidth, mdRegisters.runCalibratePiGains);

    if (result != MD::Error_t::OK)
    {
        log.error("Failed to setup bandwidth for the driver with id %d!", id);
    }
    else
    {
        log.info("Bandwidth set successful!");
    }
    return;
}

void CandleTool::configClear(u16 id)
{
    MDRegisters_S mdRegisters;
    MD            md        = MD(id, m_candle);
    auto          connected = md.init();
    if (connected != MD::Error_t::OK)
    {
        log.error("Could not connect MD with id %d", id);
        MDRegisters_S mdRegisters;
        MD            md        = MD(id, m_candle);
        auto          connected = md.init();
        if (connected != MD::Error_t::OK)
        {
            log.error("Could not connect MD with id %d", id);
            return;
        }
        mdRegisters.runRestoreFactoryConfig = 1;
        auto result = md.writeRegisters(mdRegisters.runRestoreFactoryConfig);

        if (result != MD::Error_t::OK)
        {
            log.error("Failed to clear config in the driver with id %d!", id);
        }
        else
        {
            log.info("Config cleared successful!");
        }
        return;
    }
    mdRegisters.runRestoreFactoryConfig = 1;
    auto result                         = md.writeRegisters(mdRegisters.runRestoreFactoryConfig);

    if (result != MD::Error_t::OK)
    {
        log.error("Failed to clear config in the driver with id %d!", id);
    }
    else
    {
        log.info("Config cleared successful!");
    }
    return;
}

void CandleTool::setupCalibration(u16 id)
{
    if (!ui::getCalibrationConfirmation() || checkSetupError(id))
        return;

    MDRegisters_S mdRegisters;
    MD            md        = MD(id, m_candle);
    auto          connected = md.init();
    if (connected != MD::Error_t::OK)
    {
        log.error("Could not connect MD with id %d", id);
        return;
    }
    mdRegisters.runCalibrateCmd = 1;
    auto result                 = md.writeRegisters(mdRegisters.runCalibrateCmd);

    if (result != MD::Error_t::OK)
    {
        log.error("Failed to calibrate for the driver with id %d!", id);
    }
    else
    {
        log.info("Calibration started!");
    }
    return;
}

void CandleTool::setupCalibrationOutput(u16 id)
{
    if (!ui::getCalibrationOutputConfirmation() || checkSetupError(id))
        return;

    MDRegisters_S mdRegisters;
    MD            md        = MD(id, m_candle);
    auto          connected = md.init();
    if (connected != MD::Error_t::OK)
    {
        log.error("Could not connect MD with id %d", id);
        return;
    }
    mdRegisters.auxEncoder = 0;
    auto resultRead        = md.readRegister(mdRegisters.auxEncoder);
    if (mdRegisters.auxEncoder.value != 0)
    {
        log.error("No output encoder is configured!");
        return;
    }
    if (resultRead != MD::Error_t::OK)
    {
        log.error("Failed to read output encoder type for the driver with id %d!", id);
    }

    mdRegisters.runCalibrateAuxEncoderCmd = 1;
    auto resultWrite = md.writeRegisters(mdRegisters.runCalibrateAuxEncoderCmd);
    if (resultWrite != MD::Error_t::OK)
    {
        log.error("Failed to calibrate for the driver with id %d!", id);
    }
    else
    {
        log.info("Calibration started!");
    }
}

// TODO: Variant of this method for PDS device
std::string CandleTool::validateAndGetFinalConfigPath(const std::string& cfgPath)
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

void CandleTool::setupMotor(u16 id, const std::string& cfgPath, bool force)
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

    mab::MDRegisters_S regs;
    mab::MD            md = MD(id, m_candle);
    if (md.init() != MD::Error_t::OK)
    {
        log.error("Failed to connect to MD!");
    }

    /* add a field here only if you want to test it against limits form the candletool.ini file
     */
    memcpy(regs.motorName.value,
           (cfg["motor"]["name"]).c_str(),
           strlen((cfg["motor"]["name"]).c_str()));
    if (!getField(cfg, ini, "motor", "pole pairs", regs.motorPolePairs.value))
        /* add a field here only if you want to test it against limits form the candletool.ini file
         */
        memcpy(regs.motorName.value,
               (cfg["motor"]["name"]).c_str(),
               strlen((cfg["motor"]["name"]).c_str()));
    if (!getField(cfg, ini, "motor", "pole pairs", regs.motorPolePairs.value))
        return;
    if (!getField(cfg, ini, "motor", "torque constant", regs.motorKt.value))
        if (!getField(cfg, ini, "motor", "torque constant", regs.motorKt.value))
            return;
    if (!getField(cfg, ini, "motor", "KV", regs.motorKV.value))
        if (!getField(cfg, ini, "motor", "KV", regs.motorKV.value))
            return;
    if (!getField(cfg, ini, "motor", "gear ratio", regs.motorGearRatio.value))
        if (!getField(cfg, ini, "motor", "gear ratio", regs.motorGearRatio.value))
            return;
    if (!getField(cfg, ini, "motor", "max current", regs.motorIMax.value))
        if (!getField(cfg, ini, "motor", "max current", regs.motorIMax.value))
            return;
    if (!getField(cfg, ini, "motor", "torque constant a", regs.motorKtPhaseA.value))
        if (!getField(cfg, ini, "motor", "torque constant a", regs.motorKtPhaseA.value))
            return;
    if (!getField(cfg, ini, "motor", "torque constant b", regs.motorKtPhaseB.value))
        if (!getField(cfg, ini, "motor", "torque constant b", regs.motorKtPhaseB.value))
            return;
    if (!getField(cfg, ini, "motor", "torque constant c", regs.motorKtPhaseC.value))
        if (!getField(cfg, ini, "motor", "torque constant c", regs.motorKtPhaseC.value))
            return;
    if (!getField(cfg, ini, "motor", "torque bandwidth", regs.motorTorqueBandwidth.value))
        if (!getField(cfg, ini, "motor", "torque bandwidth", regs.motorTorqueBandwidth.value))
            return;
    if (!getField(cfg, ini, "motor", "dynamic friction", regs.motorFriction.value))
        if (!getField(cfg, ini, "motor", "dynamic friction", regs.motorFriction.value))
            return;
    if (!getField(cfg, ini, "motor", "static friction", regs.motorStiction.value))
        if (!getField(cfg, ini, "motor", "static friction", regs.motorStiction.value))
            return;
    if (!getField(cfg, ini, "motor", "shutdown temp", regs.motorShutdownTemp.value))
        if (!getField(cfg, ini, "motor", "shutdown temp", regs.motorShutdownTemp.value))
            return;
    if (!getField(cfg, ini, "limits", "max velocity", regs.maxVelocity.value))
        if (!getField(cfg, ini, "limits", "max velocity", regs.maxVelocity.value))
            return;
    if (!getField(cfg, ini, "limits", "max acceleration", regs.maxAcceleration.value))
        if (!getField(cfg, ini, "limits", "max acceleration", regs.maxAcceleration.value))
            return;
    if (!getField(cfg, ini, "limits", "max deceleration", regs.maxDeceleration.value))
        if (!getField(cfg, ini, "limits", "max deceleration", regs.maxDeceleration.value))
            return;

    auto checkFieldWriteIfPopulated = [&]<class T>(const char*               category,
                                                   const char*               field,
                                                   mab::MDRegisterEntry_S<T> regVal) -> bool
    {
        if (cfg[category][field] == "")
            return true;

        if (!getField(cfg, ini, category, field, regVal.value))
            if (!getField(cfg, ini, category, field, regVal.value))
                return false;

        if (md.writeRegisters(regVal))
            if (md.writeRegisters(regVal))
            {
                log.error("Failed to setup motor! Error writing: 0x%x register.",
                          regVal.m_regAddress);
                log.error("Failed to setup motor! Error writing: 0x%x register.",
                          regVal.m_regAddress);
                return false;
            }
        return true;
    };

    if (!checkFieldWriteIfPopulated("hardware", "shunt resistance", regs.shuntResistance))
        return;

    if (!checkFieldWriteIfPopulated("motor", "reverse direction", regs.reverseDirection))
        return;

    regs.motorCalibrationMode = regs.motorCalibrationMode =
        getNumericParamFromList(cfg["motor"]["calibration mode"], ui::motorCalibrationModes);

    regs.auxEncoderDefaultBaud =
        std::atoi(cfg["output encoder"]["output encoder default baud"].c_str());
    regs.auxEncoder =
        getNumericParamFromList(cfg["output encoder"]["output encoder"], ui::encoderTypes);
    regs.auxEncoderMode = regs.auxEncoderMode =
        getNumericParamFromList(cfg["output encoder"]["output encoder mode"], ui::encoderModes);
    regs.auxEncoderCalibrationMode = getNumericParamFromList(
        cfg["output encoder"]["output encoder calibration mode"], ui::encoderCalibrationModes);
    regs.userGpioConfiguration = getNumericParamFromList(cfg["GPIO"]["mode"], ui::GPIOmodes);

    auto f32FromField = [&](const char* category, const char* field) -> f32
    { return std::atof(cfg[category][field].c_str()); };

    /* motor base config */

    md.m_timeout = 10;

    if (md.writeRegisters(regs.motorName,
                          regs.motorPolePairs,
                          regs.motorKt,
                          regs.motorKV,
                          regs.motorGearRatio,
                          regs.motorIMax,
                          regs.motorTorqueBandwidth) != MD::Error_t::OK)
    {
        md.m_timeout = 10;

        if (md.writeRegisters(regs.motorName,
                              regs.motorPolePairs,
                              regs.motorKt,
                              regs.motorKV,
                              regs.motorGearRatio,
                              regs.motorIMax,
                              regs.motorTorqueBandwidth) != MD::Error_t::OK)
        {
            log.error("Failed to setup motor!");
            return;
        }
        return;
    }
    /* motor advanced config */
    if (md.writeRegisters(regs.motorFriction,
                          regs.motorStiction,
                          regs.motorKtPhaseA,
                          regs.motorKtPhaseB,
                          regs.motorKtPhaseC,
                          regs.auxEncoder,
                          regs.auxEncoderMode,
                          regs.auxEncoderDefaultBaud) != MD::Error_t::OK)
    {
        if (md.writeRegisters(regs.motorFriction,
                              regs.motorStiction,
                              regs.motorKtPhaseA,
                              regs.motorKtPhaseB,
                              regs.motorKtPhaseC,
                              regs.auxEncoder,
                              regs.auxEncoderMode,
                              regs.auxEncoderDefaultBaud) != MD::Error_t::OK)
        {
            log.error("Failed to setup motor!");
            return;
        }
        return;
    }

    /* motor motion config - Position and velocity PID*/
    regs.motorPosPidKp     = f32FromField("position PID", "kp");
    regs.motorPosPidKi     = f32FromField("position PID", "ki");
    regs.motorPosPidKd     = f32FromField("position PID", "kd");
    regs.motorPosPidWindup = f32FromField("position PID", "windup");
    if (md.writeRegisters(
            regs.motorPosPidKp, regs.motorPosPidKi, regs.motorPosPidKd, regs.motorPosPidWindup) !=
        MD::Error_t::OK)
    {
        log.error("Failed to setup pos PIDs!");
        return;
    }

    regs.motorVelPidKp     = f32FromField("velocity PID", "kp");
    regs.motorVelPidKi     = f32FromField("velocity PID", "ki");
    regs.motorVelPidKd     = f32FromField("velocity PID", "kd");
    regs.motorVelPidWindup = f32FromField("velocity PID", "windup");
    if (md.writeRegisters(
            regs.motorVelPidKp, regs.motorVelPidKi, regs.motorVelPidKd, regs.motorVelPidWindup) !=
        MD::Error_t::OK)
    {
        log.error("Failed to setup vel PIDs!");
        return;
    }
    regs.motorPosPidKp     = f32FromField("position PID", "kp");
    regs.motorPosPidKi     = f32FromField("position PID", "ki");
    regs.motorPosPidKd     = f32FromField("position PID", "kd");
    regs.motorPosPidWindup = f32FromField("position PID", "windup");
    if (md.writeRegisters(
            regs.motorPosPidKp, regs.motorPosPidKi, regs.motorPosPidKd, regs.motorPosPidWindup) !=
        MD::Error_t::OK)
    {
        log.error("Failed to setup pos PIDs!");
        return;
    }

    regs.motorVelPidKp     = f32FromField("velocity PID", "kp");
    regs.motorVelPidKi     = f32FromField("velocity PID", "ki");
    regs.motorVelPidKd     = f32FromField("velocity PID", "kd");
    regs.motorVelPidWindup = f32FromField("velocity PID", "windup");
    if (md.writeRegisters(
            regs.motorVelPidKp, regs.motorVelPidKi, regs.motorVelPidKd, regs.motorVelPidWindup) !=
        MD::Error_t::OK)
    {
        log.error("Failed to setup vel PIDs!");
        return;
    }

    /* motor motion config - Impedance PD*/
    regs.motorImpPidKp = f32FromField("impedance PD", "kp");
    regs.motorImpPidKd = f32FromField("impedance PD", "kd");
    if (md.writeRegisters(regs.motorImpPidKp, regs.motorImpPidKd, regs.motorShutdownTemp) !=
        MD::Error_t::OK)
    {
        log.error("Failed to setup impedance PIDs!");
        return;
    }
    regs.motorImpPidKp = f32FromField("impedance PD", "kp");
    regs.motorImpPidKd = f32FromField("impedance PD", "kd");
    if (md.writeRegisters(regs.motorImpPidKp, regs.motorImpPidKd, regs.motorShutdownTemp) !=
        MD::Error_t::OK)
    {
        log.error("Failed to setup impedance PIDs!");
        return;
    }

    if (md.writeRegisters(regs.auxEncoderCalibrationMode, regs.auxEncoderCalibrationMode) !=
        MD::Error_t::OK)
    {
        log.error("Failed to setup encoder calibration mode!");
        return;
    }

    // TODO: homing was here, just a remainder that adding it will be needed at some point

    /*additional movement parameters*/
    regs.maxTorque        = f32FromField("limits", "max torque");
    regs.positionLimitMin = f32FromField("limits", "min position");
    regs.positionLimitMax = f32FromField("limits", "max position");
    if (md.writeRegisters(regs.maxTorque,
                          regs.maxAcceleration,
                          regs.maxDeceleration,
                          regs.maxVelocity,
                          regs.positionLimitMax,
                          regs.positionLimitMin) != MD::Error_t::OK)
    {
        log.error("Failed to setup position limits!");
        return;
    }

    regs.profileAcceleration   = f32FromField("profile", "acceleration");
    regs.profileDeceleration   = f32FromField("profile", "deceleration");
    regs.quickStopDeceleration = f32FromField("profile", "quick stop deceleration");
    regs.profileVelocity       = f32FromField("profile", "velocity");
    if (md.writeRegisters(regs.profileAcceleration,
                          regs.profileDeceleration,
                          regs.quickStopDeceleration,
                          regs.profileVelocity) != MD::Error_t::OK)
    {
        log.error("Failed to setup accelerations!");
        return;
    }

    regs.userGpioConfiguration = getNumericParamFromList(cfg["GPIO"]["mode"], ui::GPIOmodes);
    if (md.writeRegisters(regs.userGpioConfiguration) != MD::Error_t::OK)
    {
        log.error("Failed to setup gpio configuration!");
        return;
    }
    if (md.writeRegisters(regs.auxEncoderCalibrationMode, regs.auxEncoderCalibrationMode) !=
        MD::Error_t::OK)
    {
        log.error("Failed to setup encoder calibration mode!");
        return;
    }

    // TODO: homing was here, just a remainder that adding it will be needed at some point

    /*additional movement parameters*/
    regs.maxTorque        = f32FromField("limits", "max torque");
    regs.positionLimitMin = f32FromField("limits", "min position");
    regs.positionLimitMax = f32FromField("limits", "max position");
    if (md.writeRegisters(regs.maxTorque,
                          regs.maxAcceleration,
                          regs.maxDeceleration,
                          regs.maxVelocity,
                          regs.positionLimitMax,
                          regs.positionLimitMin) != MD::Error_t::OK)
    {
        log.error("Failed to setup position limits!");
        return;
    }

    regs.profileAcceleration   = f32FromField("profile", "acceleration");
    regs.profileDeceleration   = f32FromField("profile", "deceleration");
    regs.quickStopDeceleration = f32FromField("profile", "quick stop deceleration");
    regs.profileVelocity       = f32FromField("profile", "velocity");
    if (md.writeRegisters(regs.profileAcceleration,
                          regs.profileDeceleration,
                          regs.quickStopDeceleration,
                          regs.profileVelocity) != MD::Error_t::OK)
    {
        log.error("Failed to setup accelerations!");
        return;
    }

    regs.userGpioConfiguration = getNumericParamFromList(cfg["GPIO"]["mode"], ui::GPIOmodes);
    if (md.writeRegisters(regs.userGpioConfiguration) != MD::Error_t::OK)
    {
        log.error("Failed to setup gpio configuration!");
        return;
    }

    if (md.save() != MD::Error_t::OK)
        if (md.save() != MD::Error_t::OK)
        {
            log.error("Save failed!");
            return;
            log.error("Save failed!");
            return;
        }

    /* wait for a full reboot */
    sleep(3);
    if (md.disable() == MD::Error_t::OK)
        log.success("Ready!");
    else
        log.warn("Failed to reboot (ID: %d)!", id);
}
void CandleTool::setupReadConfig(u16 id, const std::string& cfgName)
{
    mINI::INIStructure readIni; /**< mINI structure for read data */
    MD                 md = MD(id, m_candle);
    MDRegisters_S      regs; /**< read register */
    char               motorNameChar[24];
    std::string        configName = cfgName;
    md.m_timeout                  = 10;

    if (md.init() != MD::Error_t::OK)
    {
        log.error("Could not communicate with MD device with ID %d", id);
        return;
    }

    if (cfgName == "")
    {
        if (md.readRegisters(regs.motorName) != MD::Error_t::OK)
            if (md.readRegisters(regs.motorName) != MD::Error_t::OK)
            {
                log.error("Failed to read motor config %d!", id);
                snprintf(motorNameChar, 24, "UNKNOWN_MD");
            }
        configName = std::string(motorNameChar) + "_" + std::to_string(id) + "_read.cfg";
    }
    else if (std::filesystem::path(configName).extension() == "")
        configName += ".cfg";

    /* Ask user if the motor config should be saved */
    bool saveConfig = ui::getSaveConfigConfirmation(configName);

    /* Motor config - motor section */

    if (md.readRegisters(regs.motorPolePairs,
                         regs.motorKt,
                         regs.motorIMax,
                         regs.motorGearRatio,
                         regs.motorTorqueBandwidth,
                         regs.motorKV) != MD::Error_t::OK)
    {
        if (md.readRegisters(regs.motorPolePairs,
                             regs.motorKt,
                             regs.motorIMax,
                             regs.motorGearRatio,
                             regs.motorTorqueBandwidth,
                             regs.motorKV) != MD::Error_t::OK)
        {
            log.warn("Failed to read motor config!");
        }
    }

    readIni["motor"]["name"]             = std::string(regs.motorName.value);
    readIni["motor"]["pole pairs"]       = prettyFloatToString(regs.motorPolePairs.value);
    readIni["motor"]["KV"]               = prettyFloatToString(regs.motorKV.value);
    readIni["motor"]["torque constant"]  = prettyFloatToString(regs.motorKt.value);
    readIni["motor"]["gear ratio"]       = prettyFloatToString(regs.motorGearRatio.value);
    readIni["motor"]["max current"]      = prettyFloatToString(regs.motorIMax.value);
    readIni["motor"]["torque bandwidth"] = prettyFloatToString(regs.motorTorqueBandwidth.value);
    readIni["motor"]["name"]             = std::string(regs.motorName.value);
    readIni["motor"]["pole pairs"]       = prettyFloatToString(regs.motorPolePairs.value);
    readIni["motor"]["KV"]               = prettyFloatToString(regs.motorKV.value);
    readIni["motor"]["torque constant"]  = prettyFloatToString(regs.motorKt.value);
    readIni["motor"]["gear ratio"]       = prettyFloatToString(regs.motorGearRatio.value);
    readIni["motor"]["max current"]      = prettyFloatToString(regs.motorIMax.value);
    readIni["motor"]["torque bandwidth"] = prettyFloatToString(regs.motorTorqueBandwidth.value);

    if (md.readRegisters(regs.motorShutdownTemp) != MD::Error_t::OK)
    {
        if (md.readRegisters(regs.motorShutdownTemp) != MD::Error_t::OK)
        {
            log.warn("Failed to read motor config!");
        }
    }

    readIni["motor"]["shutdown temp"] = prettyFloatToString(regs.motorShutdownTemp.value);
    readIni["motor"]["shutdown temp"] = prettyFloatToString(regs.motorShutdownTemp.value);

    /* Motor config - limits section */
    if (md.readRegisters(regs.maxTorque,
                         regs.maxVelocity,
                         regs.maxAcceleration,
                         regs.maxDeceleration,
                         regs.positionLimitMax,
                         regs.positionLimitMin) != MD::Error_t::OK)
    {
        if (md.readRegisters(regs.maxTorque,
                             regs.maxVelocity,
                             regs.maxAcceleration,
                             regs.maxDeceleration,
                             regs.positionLimitMax,
                             regs.positionLimitMin) != MD::Error_t::OK)
        {
            log.warn("Failed to read motor config!");
        }
        readIni["limits"]["max velocity"]     = prettyFloatToString(regs.maxVelocity.value);
        readIni["limits"]["max position"]     = prettyFloatToString(regs.positionLimitMax.value);
        readIni["limits"]["min position"]     = prettyFloatToString(regs.positionLimitMin.value);
        readIni["limits"]["max acceleration"] = prettyFloatToString(regs.maxAcceleration.value);
        readIni["limits"]["max deceleration"] = prettyFloatToString(regs.maxDeceleration.value);
        readIni["limits"]["max torque"]       = prettyFloatToString(regs.maxTorque.value);
    }
    readIni["limits"]["max velocity"]     = prettyFloatToString(regs.maxVelocity.value);
    readIni["limits"]["max position"]     = prettyFloatToString(regs.positionLimitMax.value);
    readIni["limits"]["min position"]     = prettyFloatToString(regs.positionLimitMin.value);
    readIni["limits"]["max acceleration"] = prettyFloatToString(regs.maxAcceleration.value);
    readIni["limits"]["max deceleration"] = prettyFloatToString(regs.maxDeceleration.value);
    readIni["limits"]["max torque"]       = prettyFloatToString(regs.maxTorque.value);

    /* Motor config - profile section */
    if (md.readRegisters(regs.profileVelocity,
                         regs.profileAcceleration,
                         regs.profileDeceleration) != MD::Error_t::OK)
    {
        if (md.readRegisters(regs.profileVelocity,
                             regs.profileAcceleration,
                             regs.profileDeceleration) != MD::Error_t::OK)
        {
            log.warn("Failed to read motor config!");
        }
    }

    readIni["profile"]["quick stop deceleration"] =
        prettyFloatToString(regs.quickStopDeceleration.value);
    readIni["profile"]["max acceleration"] = prettyFloatToString(regs.maxAcceleration.value);
    readIni["profile"]["max deceleration"] = prettyFloatToString(regs.maxDeceleration.value);
    readIni["profile"]["quick stop deceleration"] =
        prettyFloatToString(regs.quickStopDeceleration.value);
    readIni["profile"]["max acceleration"] = prettyFloatToString(regs.maxAcceleration.value);
    readIni["profile"]["max deceleration"] = prettyFloatToString(regs.maxDeceleration.value);

    /* Motor config - output encoder section */

    if (md.readRegisters(regs.auxEncoder, regs.auxEncoderMode) != MD::Error_t::OK)
    {
        if (md.readRegisters(regs.auxEncoder, regs.auxEncoderMode) != MD::Error_t::OK)
        {
            log.warn("Failed to read motor config!");
        }
    }

    if (regs.auxEncoder.value == 0)
        readIni["output encoder"]["output encoder"] = prettyFloatToString(0.f, true);
    else
        readIni["output encoder"]["output encoder"] = ui::encoderTypes[regs.auxEncoder.value];

    if (regs.auxEncoderMode.value == 0)
        readIni["output encoder"]["output encoder mode"] = prettyFloatToString(0.f, true);
    else
        readIni["output encoder"]["output encoder mode"] =
            ui::encoderModes[regs.auxEncoderMode.value];

    /* Motor config - position PID section */
    if (md.readRegisters(
            regs.motorPosPidKp, regs.motorPosPidKi, regs.motorPosPidKd, regs.motorPosPidWindup) !=
        MD::Error_t::OK)

    {
        log.warn("Failed to read motor config!");
    }
    readIni["position PID"]["kp"]     = prettyFloatToString(regs.motorPosPidKp.value);
    readIni["position PID"]["ki"]     = prettyFloatToString(regs.motorPosPidKi.value);
    readIni["position PID"]["kd"]     = prettyFloatToString(regs.motorPosPidKd.value);
    readIni["position PID"]["windup"] = prettyFloatToString(regs.motorPosPidWindup.value);

    /* Motor config - velocity PID section */
    if (md.readRegisters(
            regs.motorVelPidKp, regs.motorVelPidKi, regs.motorVelPidKd, regs.motorVelPidWindup) !=
        MD::Error_t::OK)
    {
        log.warn("Failed to read motor config!");
    }
    readIni["velocity PID"]["kp"]     = prettyFloatToString(regs.motorVelPidKp.value);
    readIni["velocity PID"]["ki"]     = prettyFloatToString(regs.motorVelPidKi.value);
    readIni["velocity PID"]["kd"]     = prettyFloatToString(regs.motorVelPidKd.value);
    readIni["velocity PID"]["windup"] = prettyFloatToString(regs.motorVelPidWindup.value);

    /* Motor config - impedance PD section */
    if (md.readRegisters(regs.motorImpPidKp, regs.motorImpPidKd) != MD::Error_t::OK)
    {
        log.warn("Failed to read motor config!");
    }
    readIni["impedance PD"]["kp"] = prettyFloatToString(regs.motorImpPidKp.value);
    readIni["impedance PD"]["kd"] = prettyFloatToString(regs.motorImpPidKd.value);

    /* Motor config - homing section */

    // TODO: implement homing section

    /* Saving motor config to file */
    if (saveConfig)
    {
        std::string saveConfigPath = std::filesystem::absolute(configName).string();

        bool checkFile = true;
        while (checkFile)
        {
            struct stat st;
            if (stat(saveConfigPath.c_str(), &st) == 0)
            {
                if (!ui::getOverwriteMotorConfigConfirmation(configName))
                {
                    configName = ui::getNewMotorConfigName(configName);
                    saveConfigPath =
                        saveConfigPath.substr(0, saveConfigPath.find_last_of("/") + 1) + configName;

                    if (std::filesystem::path(saveConfigPath).extension() == "")
                        saveConfigPath += ".cfg";
                }
                else
                    checkFile = false;
            }
            else
                checkFile = false;
        }

        mINI::INIFile configFile(saveConfigPath);
        configFile.write(readIni);
    }

    /* Printing motor config */
    ui::printMotorConfig(readIni);
}

void CandleTool::setupInfo(u16 id, bool printAll)
{
    MD            md = MD(id, m_candle);
    MDRegisters_S readableRegisters;
    if (md.init() != MD::Error_t::OK)
    {
        log.error("Could not communicate with MD device with ID %d", id);
        MD            md = MD(id, m_candle);
        MDRegisters_S readableRegisters;
        if (md.init() != MD::Error_t::OK)
        {
            log.error("Could not communicate with MD device with ID %d", id);
            return;
        }

        auto readReadableRegs = [&]<typename T>(MDRegisterEntry_S<T>& reg)
        {
            // TODO: skipping new registers for now
            if ((reg.m_regAddress < 0x800 && reg.m_regAddress > 0x700) || reg.m_regAddress > 0x810)
                return;
            if (reg.m_accessLevel != RegisterAccessLevel_E::WO)
            {
                auto fault = md.readRegisters(reg);
                if (fault != MD::Error_t::OK)
                    log.error("Error while reading register %s", reg.m_name.data());
            }
        };

        readableRegisters.forEachRegister(readReadableRegs);

        ui::printDriveInfoExtended(md, readableRegisters, printAll);
    }

    auto readReadableRegs = [&]<typename T>(MDRegisterEntry_S<T>& reg)
    {
        // TODO: skipping new registers for now
        if ((reg.m_regAddress < 0x800 && reg.m_regAddress > 0x700) || reg.m_regAddress > 0x810)
            return;
        if (reg.m_accessLevel != RegisterAccessLevel_E::WO)
        {
            auto fault = md.readRegisters(reg);
            if (fault != MD::Error_t::OK)
                log.error("Error while reading register %s", reg.m_name.data());
        }
    };

    readableRegisters.forEachRegister(readReadableRegs);

    ui::printDriveInfoExtended(md, readableRegisters, printAll);
}

void CandleTool::testMove(u16 id, f32 targetPosition)
{
    if (targetPosition > 10.0f)
        targetPosition = 10.0f;

    mab::MD md(id, m_candle);
    auto    mdInitResult = md.init();
    if (mdInitResult != MD::Error_t::OK)
        log.error("Error while initializing MD");

    md.setMotionMode(mab::MdMode_E::IMPEDANCE);
    f32 pos = md.getPosition().first;
    md.setTargetPosition(pos);
    targetPosition += pos;
    md.enable();

    for (f32 t = 0.f; t < 1.f; t += 0.01f)
    {
        f32 target  = std::lerp(pos, targetPosition, t);
        log.m_layer = Logger::ProgramLayer_E::TOP;
        md.setTargetPosition(target);
        log.info("[%4d] Position: %4.2f, Velocity: %4.1f",
                 id,
                 md.getPosition().first,
                 md.getVelocity().first);
        usleep(30000);
    }
}

void CandleTool::testMoveAbsolute(u16 id, f32 targetPos, f32 velLimit, f32 accLimit, f32 dccLimit)
{
    MD md(id, m_candle);
    if (md.init() != MD::Error_t::OK)
    {
        log.error("Failed to initialize MD");
        return;
    }

    MDRegisters_S registers;
    registers.profileVelocity     = velLimit;
    registers.profileAcceleration = accLimit;
    registers.profileDeceleration = dccLimit;

    if (velLimit > 0)
        md.writeRegisters(registers.profileVelocity);
    md.writeRegisters(registers.profileVelocity);
    if (accLimit > 0)
        md.writeRegisters(registers.profileAcceleration);
    md.writeRegisters(registers.profileAcceleration);
    if (dccLimit > 0)
        md.writeRegisters(registers.profileDeceleration);
    md.writeRegisters(registers.profileDeceleration);

    md.setMotionMode(mab::MdMode_E::POSITION_PROFILE);
    md.enable();
    md.setTargetPosition(targetPos);
    while (
        !(md.getQuickStatus().first.at(MDStatus::QuickStatusBits::TargetPositionReached).isSet()))
        usleep(10'000);
    md.disable();
    log.info("TARGET REACHED!");
}

void CandleTool::testLatency(const std::string& canBaudrate, std::string busString)
{
    log.warn("Not implemented");
    // TODO: think of a better way to test communication speed
}

void CandleTool::testEncoderOutput(u16 id)
{
    MD md(id, m_candle);
    if (md.init() != MD::Error_t::OK)
    {
        log.error("Failed to initialize MD with ID: %d", id);
        return;
    }
    MDRegisters_S regs;
    md.readRegister(regs.auxEncoder);
    if (regs.auxEncoder.value == 0)
    {
        log.warn("No output encoder on ID: %d! Not testing.", id);
        return;
    }
    regs.runTestAuxEncoderCmd = 1;
    md.writeRegisters(regs.runTestAuxEncoderCmd);
    log.info("Please wait for the test to finish...");
}

void CandleTool::testEncoderMain(u16 id)
{
    MD md(id, m_candle);
    if (md.init() != MD::Error_t::OK)
    {
        log.error("Failed to initialize MD with ID: %d", id);
        return;
    }
    MDRegisters_S regs;
    regs.runTestMainEncoderCmd = 1;
    md.writeRegisters(regs.runTestMainEncoderCmd);
    log.info("Please wait for the test to finish...");
}

void CandleTool::registerWrite(u16 id, u16 regAdress, const std::string& value)
{
    MD md(id, m_candle);
    if (md.init() != MD::Error_t::OK)
    {
        log.error("Failed to initialize MD with ID: %d", id);
        return;
    }

    std::string trimmedValue = trim(value);

    MDRegisters_S                             regs;
    std::variant<int64_t, float, std::string> regValue;
    bool                                      foundRegister      = false;
    bool                                      registerCompatible = false;

    // Check if the value is a string or a number
    if (trimmedValue.find_first_not_of("0123456789.f") == std::string::npos)
    {
        /// Check if the value is a float or an integer
        if (trimmedValue.find('.') != std::string::npos)
            regValue = std::stof(value);
        else
            regValue = std::stoll(value);
    }
    else
    {
        regValue = trimmedValue;
    }

    auto setRegValueByAdress = [&]<typename T>(MDRegisterEntry_S<T> reg)
    {
        if (reg.m_regAddress == regAdress)
        {
            foundRegister = true;
            if constexpr (std::is_arithmetic<T>::value)
            {
                registerCompatible = true;
                if (std::holds_alternative<int64_t>(regValue))
                    reg.value = std::get<int64_t>(regValue);
                else if (std::holds_alternative<float>(regValue))
                    reg.value = std::get<float>(regValue);

                auto result = md.writeRegisters(reg);
                if (result != MD::Error_t::OK)
                    log.error("Failed to write register %d", reg.m_regAddress);
            }
            else if constexpr (std::is_same<std::decay_t<T>, char*>::value)
            {
                registerCompatible = true;
                std::string_view strV;
                if (std::holds_alternative<std::string>(regValue))
                    strV = std::get<std::string>(regValue).c_str();
                else
                {
                    log.error("Invalid value type for register %d", reg.m_regAddress);
                    return;
                }

                if (strV.length() > sizeof(reg.value) + 1)
                {
                    log.error("Value too long for register %d", reg.m_regAddress);
                    return;
                }

                std::copy(strV.data(), strV.data() + strV.length(), reg.value);

                auto result = md.writeRegisters(reg);
                if (result != MD::Error_t::OK)
                    log.error("Failed to write register %d", reg.m_regAddress);
            }
        }
    };
    regs.forEachRegister(setRegValueByAdress);
    if (!foundRegister)
    {
        log.error("Register %d not found", regAdress);
        return;
    }
    if (!registerCompatible)
    {
        log.error("Register %d not compatible with value %s", regAdress, value.c_str());
        return;
    }

    log.success("Writing register successful!");
}

void CandleTool::registerRead(u16 id, u16 regAdress)
{
    MD md(id, m_candle);
    if (md.init() != MD::Error_t::OK)
    {
        log.error("Failed to initialize MD with ID: %d", id);
    }
    MDRegisters_S regs;
    auto          getValueByAdress = [&]<typename T>(MDRegisterEntry_S<T> reg)
    {
        if (reg.m_regAddress == regAdress)
        {
            if constexpr (std::is_arithmetic_v<T>)
            {
                auto result = md.readRegister(reg);
                if (result != MD::Error_t::OK)
                {
                    log.error("Failed to read register %d", regAdress);
                    return false;
                }
                std::string value = std::to_string(reg.value);
                log.success("Register %d value: %s", regAdress, value.c_str());
                return true;
            }
            else if constexpr (std::is_same<std::decay_t<T>, char*>::value)
            {
                auto result = md.readRegisters(reg);
                if (result != MD::Error_t::OK)
                {
                    log.error("Failed to read register %d", regAdress);
                    return false;
                }
                const char* value = reg.value;
                log.success("Register %d value: %s", regAdress, value);
                return true;
            }
        }
        return false;
    };
    regs.forEachRegister(getValueByAdress);
}

void CandleTool::updateCandle(const std::string& mabFilePath)
{
    log.info("Performing Candle firmware update.");

    MabFileParser candleFirmware(mabFilePath, MabFileParser::TargetDevice_E::CANDLE);

    auto candle_bootloader = attachCandleBootloader();
    for (size_t i = 0; i < candleFirmware.m_fwEntry.size;
         i += CandleBootloader::PAGE_SIZE_STM32G474)
    {
        std::array<u8, CandleBootloader::PAGE_SIZE_STM32G474> page;
        std::memcpy(page.data(), &candleFirmware.m_fwEntry.data->data()[i], page.size());
        u32 crc = crc32(page.data(), page.size());
        if (candle_bootloader->writePage(page, crc) != candleTypes::Error_t::OK)
        {
            log.error("Candle flashing failed!");
            break;
        }
    }
}

void CandleTool::updateMd(const std::string& mabFilePath, mab::canId_t canId, bool noReset)
{
    MabFileParser mabFile(mabFilePath, MabFileParser::TargetDevice_E::MD);

    if (!noReset)
    {
        MD md(canId, m_candle);
        if (md.init() != MD::Error_t::OK)
        {
            log.error("Could not communicate with MD device with ID %d", canId);
            return;
        }
        md.reset();
    }
    usleep(300'000);
    CanLoader canLoader(m_candle, &mabFile, canId);
    if (canLoader.flashAndBoot())
    {
        log.success("Update complete for MD @ %d", canId);
    }
    else
    {
        log.error("MD flashing failed!");
    }
}

void CandleTool::updatePds(Pds& pds, const std::string& mabFilePath, uint16_t canId, bool noReset)
{
    MabFileParser      mabFile(mabFilePath, MabFileParser::TargetDevice_E::PDS);
    PdsModule::error_E result = PdsModule::error_E::OK;

    log.warn("PDS Firmware update is under development and not yet available.");

    if (!noReset)
    {
        log.debug("Resetting PDS...");
        result = pds.reboot();
        if (result != PdsModule::error_E::OK)
        {
            log.error("PDS Reset failed! [ %s ]", PdsModule::error2String(result));
            return;
        }
        else
        {
            log.success("PDS Reset successful!");
        }
    }
    log.debug("Waiting for PDS to boot...");
    usleep(400000);

    CanLoader canLoader(m_candle, &mabFile, canId);
    if (canLoader.flashAndBoot())
    {
        log.success("Update complete for PDS @ %d", canId);
    }
    else
    {
        log.error("PDS flashing failed!");
    }
}

void CandleTool::blink(u16 id)
{
    MD md(id, m_candle);
    if (md.init() != MD::Error_t::OK)
    {
        log.error("Could not communicate with MD device with ID %d", id);
        return;
    }
    if (md.blink() != MD::Error_t::OK)
    {
        log.error("Failed to blink MD device with ID %d", id);
        return;
    }
    log.success("Blinking MD device with ID %d", id);
}
void CandleTool::encoder(u16 id)
{
    // TODO: remove this as it is useless
    log.error("Not implemented!");
}
void CandleTool::bus(const std::string& bus, const std::string& device)
{
    // TODO: not implemented yet, will be implemented with the SPI
    log.error("Not implemented!");
}

void CandleTool::clearErrors(u16 id, const std::string& level)
{
    MD md(id, m_candle);
    if (md.init() != MD::Error_t::OK)
    {
        log.error("Could not communicate with MD device with ID %d", id);
        return;
    }
    if (level == "error")
        md.clearErrors();
    else if (level == "warning")
        md.clearErrors();  // TODO: implement clear warnings if nessesary
    else
    {
        md.clearErrors();
    }
}

void CandleTool::reset(u16 id)
{
    MD md(id, m_candle);
    if (md.init() != MD::Error_t::OK)
    {
        log.error("Could not communicate with MD device with ID %d", id);
        return;
    }
    md.reset();
}

u8 CandleTool::getNumericParamFromList(std::string& param, const std::vector<std::string>& list)
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
bool CandleTool::getField(mINI::INIStructure& cfg,
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

    if (ui::checkParamLimit(value, min, max))
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

bool CandleTool::checkSetupError(u16 id)
{
    MD md(id, m_candle);
    if (md.init() != MD::Error_t::OK)
    {
        return false;
    }

    bool setupError =
        md.getCalibrationStatus().first.at(MDStatus::CalibrationStatusBits::ErrorSetup).m_set;

    if (setupError)
    {
        log.error(
            "Could not proceed due to %s. Please call candletool setup motor <ID> "
            "<cfg> first.",
            RED__("ERROR_SETUP"));
        return true;
    }

    return false;
}
