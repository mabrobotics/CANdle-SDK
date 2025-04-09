#include "candletool.hpp"

#include <any>
#include <numeric>
#include <unistd.h>
#include <string>
#include <array>
#include <vector>

#include "MDStatus.hpp"
#include "mab_types.hpp"
#include "md_types.hpp"
#include "register.hpp"
#include "ui.hpp"
#include "configHelpers.hpp"

#include "mabFileParser.hpp"
#include "candle_bootloader.hpp"
#include "candle_v2.hpp"
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

CandleTool::CandleTool()
{
    log.m_tag   = "CANDLETOOL";
    log.m_layer = Logger::ProgramLayer_E::TOP;
    log << "CandleSDK Version: " << mab::Candle::getVersion() << std::endl;

    std::unique_ptr<I_CommunicationInterface> bus;
    mab::CANdleBaudrate_E                     baud =
        mab::CANdleBaudrate_E::CAN_BAUD_1M;  // TODO: this must be parsed as a flag

    mINI::INIFile      file(getCandletoolConfigPath());
    mINI::INIStructure ini;
    file.read(ini);

    std::string& device = ini["communication"]["device"];
    busString           = ini["communication"]["bus"];

    if (busString == "SPI")
    {
        bus = nullptr;  // TODO: placeholder
    }
    else if (busString == "USB")
        bus = std::make_unique<USBv2>(CandleV2::CANDLE_VID, CandleV2::CANDLE_PID, device);

    m_candle = new CandleV2(baud, std::move(bus));
    m_candle->init();
    // TODO: move this to be more stateless and be able to start w/o candle attached

    return;
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
    checkSpeedForId(id);
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
    checkSpeedForId(id);
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
        return;
    }
    mdRegisters.runRestoreFactoryConfig = 1;
    auto result                         = md.writeRegisters(mdRegisters.runRestoreFactoryConfig);

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
    if (resultRead.second != MD::Error_t::OK)
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
        if (!getConfirmation())
            exit(0);
        finalConfigPath = generateUpdatedConfigFile(finalConfigPath);
        log.info("Generated updated file \"%s\"", finalConfigPath.c_str());
        log.info("Setup MD with newly generated config? [y/n]");
        if (!getConfirmation())
            exit(0);
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

    // if (!tryAddMD80(id))
    //     return;
    // mab::regWrite_st& regW = candle->getMd80FromList(id).getWriteReg();
    // mab::regRead_st&  regR = candle->getMd80FromList(id).getReadReg();

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
        return;
    if (!getField(cfg, ini, "motor", "torque constant", regs.motorKt.value))
        return;
    if (!getField(cfg, ini, "motor", "KV", regs.motorKV.value))
        return;
    if (!getField(cfg, ini, "motor", "gear ratio", regs.motorGearRatio.value))
        return;
    if (!getField(cfg, ini, "motor", "max current", regs.motorIMax.value))
        return;
    if (!getField(cfg, ini, "motor", "torque constant a", regs.motorKtPhaseA.value))
        return;
    if (!getField(cfg, ini, "motor", "torque constant b", regs.motorKtPhaseB.value))
        return;
    if (!getField(cfg, ini, "motor", "torque constant c", regs.motorKtPhaseC.value))
        return;
    if (!getField(cfg, ini, "motor", "torque bandwidth", regs.motorTorqueBandwidth.value))
        return;
    if (!getField(cfg, ini, "motor", "dynamic friction", regs.motorFriction.value))
        return;
    if (!getField(cfg, ini, "motor", "static friction", regs.motorStiction.value))
        return;
    if (!getField(cfg, ini, "motor", "shutdown temp", regs.motorShutdownTemp.value))
        return;
    if (!getField(cfg, ini, "limits", "max velocity", regs.maxVelocity.value))
        return;
    if (!getField(cfg, ini, "limits", "max acceleration", regs.maxAcceleration.value))
        return;
    if (!getField(cfg, ini, "limits", "max deceleration", regs.maxDeceleration.value))
        return;

    auto checkFieldWriteIfPopulated = [&]<class T>(const char*               category,
                                                   const char*               field,
                                                   mab::MDRegisterEntry_S<T> regVal) -> bool
    {
        if (cfg[category][field] == "")
            return true;

        if (!getField(cfg, ini, category, field, regVal.value))
            return false;

        if (md.writeRegisters(regVal))
        {
            log.error("Failed to setup motor! Error writing: 0x%x register.", regVal.m_regAddress);
            return false;
        }
        return true;
    };

    if (!checkFieldWriteIfPopulated("hardware", "shunt resistance", regs.shuntResistance))
        return;

    if (!checkFieldWriteIfPopulated("motor", "reverse direction", regs.reverseDirection))
        return;

    regs.motorCalibrationMode =
        getNumericParamFromList(cfg["motor"]["calibration mode"], ui::motorCalibrationModes);

    regs.auxEncoderDefaultBaud =
        std::atoi(cfg["output encoder"]["output encoder default baud"].c_str());
    regs.auxEncoder =
        getNumericParamFromList(cfg["output encoder"]["output encoder"], ui::encoderTypes);
    regs.auxEncoderMode =
        getNumericParamFromList(cfg["output encoder"]["output encoder mode"], ui::encoderModes);
    regs.auxEncoderCalibrationMode = getNumericParamFromList(
        cfg["output encoder"]["output encoder calibration mode"], ui::encoderCalibrationModes);
    regs.userGpioConfiguration = getNumericParamFromList(cfg["GPIO"]["mode"], ui::GPIOmodes);

    auto f32FromField = [&](const char* category, const char* field) -> f32
    { return std::atof(cfg[category][field].c_str()); };

    /* motor base config */

    if (md.writeRegisters(regs.motorName,
                          regs.motorPolePairs,
                          regs.motorKt,
                          regs.motorKV,
                          regs.motorGearRatio,
                          regs.motorIMax,
                          regs.motorTorqueBandwidth) != MD::Error_t::OK)
    {
        log.error("Failed to setup motor!");
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
        log.error("Failed to setup motor!");
    }

    /* motor motion config - Position and velocity PID*/
    if (md.writeRegisters(regs.motorPosPidKp,
                          regs.motorPosPidKi,
                          regs.motorPosPidKd,
                          regs.motorPosPidWindup,
                          regs.motorVelPidKp,
                          regs.motorVelPidKi,
                          regs.motorVelPidKd,
                          regs.motorVelPidWindup) != MD::Error_t::OK)
    {
        log.error("Failed to setup PIDs!");
    }

    /* motor motion config - Impedance PD*/
    regs.motorImpPidKp = f32FromField("impedance PD", "kp");
    regs.motorImpPidKd = f32FromField("impedance PD", "kd");
    if (md.writeRegisters(regs.motorImpPidKp, regs.motorImpPidKd, regs.motorShutdownTemp) !=
        MD::Error_t::OK)
    {
        log.error("Failed to setup PIDs!");
    }

    if (md.writeRegisters(regs.auxEncoderCalibrationMode, regs.auxEncoderCalibrationMode) !=
        MD::Error_t::OK)
    {
        log.error("Failed to setup encoder calibration mode!");
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
    }

    if (md.writeRegisters(regs.userGpioConfiguration) != MD::Error_t::OK)
    {
        log.error("Failed to setup gpio configuration!");
    }

    if (md.save() != MD::Error_t::OK)
    {
        log.error("Save failed!");
    }

    /* wait for a full reboot */
    sleep(3);
    if (md.enable() != MD::Error_t::OK)
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

    if (md.init() != MD::Error_t::OK)
    {
        log.error("Could not communicate with MD device with ID %d", id);
        return;
    }

    if (cfgName == "")
    {
        if (md.readRegisters(regs.motorName).second != MD::Error_t::OK)
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
                         regs.motorKV)
            .second != MD::Error_t::OK)
    {
        log.warn("Failed to read motor config!");
    }

    readIni["motor"]["name"]             = std::string(regs.motorName.value);
    readIni["motor"]["pole pairs"]       = prettyFloatToString(regs.motorPolePairs.value);
    readIni["motor"]["KV"]               = prettyFloatToString(regs.motorKV.value);
    readIni["motor"]["torque constant"]  = prettyFloatToString(regs.motorKt.value);
    readIni["motor"]["gear ratio"]       = prettyFloatToString(regs.motorGearRatio.value);
    readIni["motor"]["max current"]      = prettyFloatToString(regs.motorIMax.value);
    readIni["motor"]["torque bandwidth"] = prettyFloatToString(regs.motorTorqueBandwidth.value);

    if (md.readRegisters(regs.motorShutdownTemp).second != MD::Error_t::OK)
    {
        log.warn("Failed to read motor config!");
    }

    readIni["motor"]["shutdown temp"] = prettyFloatToString(regs.motorShutdownTemp.value);

    /* Motor config - limits section */
    if (md.readRegisters(regs.maxTorque,
                         regs.maxVelocity,
                         regs.maxAcceleration,
                         regs.maxDeceleration,
                         regs.positionLimitMax,
                         regs.positionLimitMin)
            .second != MD::Error_t::OK)
    {
        log.warn("Failed to read motor config!");
    }
    readIni["limits"]["max velocity"]     = prettyFloatToString(regs.maxVelocity.value);
    readIni["limits"]["max position"]     = prettyFloatToString(regs.positionLimitMax.value);
    readIni["limits"]["min position"]     = prettyFloatToString(regs.positionLimitMin.value);
    readIni["limits"]["max acceleration"] = prettyFloatToString(regs.maxAcceleration.value);
    readIni["limits"]["max deceleration"] = prettyFloatToString(regs.maxDeceleration.value);
    readIni["limits"]["max torque"]       = prettyFloatToString(regs.maxTorque.value);

    /* Motor config - profile section */
    if (md.readRegisters(regs.profileVelocity, regs.profileAcceleration, regs.profileDeceleration)
            .second != MD::Error_t::OK)
    {
        log.warn("Failed to read motor config!");
    }

    readIni["profile"]["quick stop deceleration"] =
        prettyFloatToString(regs.quickStopDeceleration.value);
    readIni["profile"]["max acceleration"] = prettyFloatToString(regs.maxAcceleration.value);
    readIni["profile"]["max deceleration"] = prettyFloatToString(regs.maxDeceleration.value);

    /* Motor config - output encoder section */
    // if (!candle->readMd80Register(id,
    //                               mab::Md80Reg_E::outputEncoder,
    //                               regR.RW.outputEncoder,
    //                               mab::Md80Reg_E::outputEncoderMode,
    //                               regR.RW.outputEncoderMode))
    //     log.warn("Failed to read motor config!");
    if (md.readRegisters(regs.auxEncoder, regs.auxEncoderMode).second != MD::Error_t::OK)
    {
        log.warn("Failed to read motor config!");
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
              regs.motorPosPidKp, regs.motorPosPidKi, regs.motorPosPidKd, regs.motorPosPidWindup)
            .second != MD::Error_t::OK)
    {
        log.warn("Failed to read motor config!");
    }
    readIni["position PID"]["kp"]     = prettyFloatToString(regs.motorPosPidKp.value);
    readIni["position PID"]["ki"]     = prettyFloatToString(regs.motorPosPidKi.value);
    readIni["position PID"]["kd"]     = prettyFloatToString(regs.motorPosPidKd.value);
    readIni["position PID"]["windup"] = prettyFloatToString(regs.motorPosPidWindup.value);

    /* Motor config - velocity PID section */
    if (md.readRegisters(
              regs.motorVelPidKp, regs.motorVelPidKi, regs.motorVelPidKd, regs.motorVelPidWindup)
            .second != MD::Error_t::OK)
    {
        log.warn("Failed to read motor config!");
    }
    readIni["velocity PID"]["kp"]     = prettyFloatToString(regs.motorVelPidKp.value);
    readIni["velocity PID"]["ki"]     = prettyFloatToString(regs.motorVelPidKi.value);
    readIni["velocity PID"]["kd"]     = prettyFloatToString(regs.motorVelPidKd.value);
    readIni["velocity PID"]["windup"] = prettyFloatToString(regs.motorVelPidWindup.value);

    /* Motor config - impedance PD section */
    if (md.readRegisters(regs.motorImpPidKp, regs.motorImpPidKd).second != MD::Error_t::OK)
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
        return;
    }

    auto readReadableRegs = [&]<typename T>(MDRegisterEntry_S<T> reg)
    {
        auto fault = md.readRegisters(reg).second;
        if (fault != MD::Error_t::OK)
            log.error("Error while reading register %s", reg.m_name);
    };

    readableRegisters.forEachRegister(readableRegisters, readReadableRegs);

    ui::printDriveInfoExtended(md, readableRegisters, printAll);
}

void CandleTool::testMove(u16 id, f32 targetPosition)
{
    if (targetPosition > 10.0f)
        targetPosition = 10.0f;
    if (targetPosition < -10.0f)
        targetPosition = -10.0f;

    mab::MD md(id, m_candle);
    auto    mdInitResult = md.init();
    if (mdInitResult != MD::Error_t::OK)
        log.error("Error while initializing MD80");

    md.setMotionMode(mab::Md80Mode_E::IMPEDANCE);
    f32 pos = md.getPosition().first;
    md.setTargetPosition(pos);
    targetPosition += pos;

    md.enable();

    for (f32 t = 0.f; t < 1.f; t += 0.01f)
    {
        f32 target  = std::lerp(pos, targetPosition, t);
        log.m_layer = Logger::ProgramLayer_E::TOP;
        md.setTargetPosition(target);
        log.info("[%4d] Position: %4.2f, Velocity: %4.1f", id, md.getPosition(), md.getVelocity());
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
    if (accLimit > 0)
        md.writeRegisters(registers.profileAcceleration);
    if (dccLimit > 0)
        md.writeRegisters(registers.profileDeceleration);

    md.setMotionMode(mab::Md80Mode_E::POSITION_PROFILE);
    md.enable();
    md.setTargetPosition(targetPos);
    while (
        !(md.getQuickStatus().first.at(MDStatus::QuickStatusBits::TargetPositionReached).isSet()))
        sleep(1);
    log.info("TARGET REACHED!");
}

void CandleTool::testLatency(const std::string& canBaudrate, std::string busString)
{
    // #ifdef UNIX
    //     struct sched_param sp;
    //     memset(&sp, 0, sizeof(sp));
    //     sp.sched_priority = 99;
    //     sched_setscheduler(0, SCHED_FIFO, &sp);
    // #endif
    // #ifdef WIN32
    //     SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);
    // #endif
    //     auto ids = m_candle->ping(str2baud(canBaudrate));
    //     if (ids.size() == 0)
    //         return;
    //     checkSpeedForId(ids[0]);

    //     for (auto& id : ids)
    //     {
    //         m_candle->addMd80(id);
    //         m_candle->controlMd80Mode(id, mab::Md80Mode_E::IMPEDANCE);
    //     }

    //     m_candle->begin();
    //     std::vector<u32> samples;
    //     const u32        timelen = 10;
    //     sleep(1);
    //     for (u32 i = 0; i < timelen; i++)
    //     {
    //         sleep(1);
    //         samples.push_back(m_candle->getActualCommunicationFrequency());
    //         log.info("Current average communication speed: %d Hz", samples[i]);
    //     }

    //     /* calculate mean and stdev */
    //     f32 sum = std::accumulate(std::begin(samples), std::end(samples), 0.0);
    //     f32 m   = sum / samples.size();

    //     f32 accum = 0.0;
    //     std::for_each(
    //         std::begin(samples), std::end(samples), [&](const f32 d) { accum += (d - m) * (d -
    //         m); });
    //     f32 stdev = sqrt(accum / (samples.size() - 1));

    //     ui::printLatencyTestResult(ids.size(), m, stdev, busString);

    //     m_candle->end();
    //
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

    MDRegisters_S            regs;
    std::variant<s32, float> regValue;
    bool                     foundRegister      = false;
    bool                     registerCompatible = false;

    /// Check if the value is a float or an integer
    if (value.find('.') != std::string::npos)
        regValue = std::stof(value);
    else
        regValue = std::stoi(value);

    // TODO: make it work for integers smaller than s32
    auto getRegValueByAdress = [&]<typename T>(MDRegisterEntry_S<T> reg)
    {
        if (reg.m_regAddress == regAdress)
        {
            foundRegister      = true;
            registerCompatible = std::holds_alternative<T>(regValue);
            if (registerCompatible)
            {
                reg.value   = std::get<T>(regValue);
                auto result = md.writeRegisters(reg);
                if (result != MD::Error_t::OK)
                    log.error("Failed to write register %d", reg.m_regAddress);
            }
        }
    };
    regs.forEachRegister(regs, getRegValueByAdress);
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

void CandleTool::registerRead(u16 id, u16 reg)
{
    if (!tryAddMD80(id))
        return;
    mab::Md80Reg_E regId = (mab::Md80Reg_E)reg;
    std::string    value = "";

    switch (mab::Register::getType(regId))
    {
        case mab::Register::type::U8:
            readRegisterToString<u8>(id, regId, value);
            break;
        case mab::Register::type::I8:
            readRegisterToString<int8_t>(id, regId, value);
            break;
        case mab::Register::type::U16:
            readRegisterToString<u16>(id, regId, value);
            break;
        case mab::Register::type::I16:
            readRegisterToString<int16_t>(id, regId, value);
            break;
        case mab::Register::type::U32:
            readRegisterToString<u32>(id, regId, value);
            break;
        case mab::Register::type::I32:
            readRegisterToString<int32_t>(id, regId, value);
            break;
        case mab::Register::type::F32:
            readRegisterToString<f32>(id, regId, value);
            break;
        case mab::Register::type::STR:
        {
            char str[24]{};
            m_candle->readMd80Register(id, regId, str);
            value = std::string(str);
            break;
        }
        case mab::Register::type::UNKNOWN:
            log.error("Unknown register! Please check the ID and try again");
            break;
    }
    log.success("Register value: %s", value.c_str());
}

void CandleTool::updateCandle(const std::string& mabFilePath, bool noReset)
{
    log.info("Performing Candle firmware update.");
    MabFileParser         mabFile(mabFilePath, MabFileParser::TargetDevice_E::CANDLE);
    mab::FirmwareUploader firmwareUploader(m_candle, mabFile);
    firmwareUploader.flashDevice(noReset);

    void CandleTool::pdsSetupInfo(u16 id)
    {
        mab::Pds pds(id, *candle);

        mab::Pds::modulesSet_S pdsModules = pds.getModules();

        u32                       shutdownTime  = 0;
        u32                       batteryLvl1   = 0;
        u32                       batteryLvl2   = 0;
        u32                       pdsBusVoltage = 0;
        mab::controlBoardStatus_S pdsStatus     = {0};

        pds.getStatus(pdsStatus);
        pds.getBusVoltage(pdsBusVoltage);
        pds.getShutdownTime(shutdownTime);
        pds.getBatteryVoltageLevels(batteryLvl1, batteryLvl2);

        log.info("Submodules:");
        log.info("\t1 :: %s", mab::Pds::moduleTypeToString(pdsModules.moduleTypeSocket1));
        log.info("\t2 :: %s", mab::Pds::moduleTypeToString(pdsModules.moduleTypeSocket2));
        log.info("\t3 :: %s", mab::Pds::moduleTypeToString(pdsModules.moduleTypeSocket3));
        log.info("\t4 :: %s", mab::Pds::moduleTypeToString(pdsModules.moduleTypeSocket4));
        log.info("\t5 :: %s", mab::Pds::moduleTypeToString(pdsModules.moduleTypeSocket5));
        log.info("\t6 :: %s", mab::Pds::moduleTypeToString(pdsModules.moduleTypeSocket6));

        log.info("PDS Status:");

        log.info("\t* ENABLED           [ %s ]", pdsStatus.ENABLED ? "YES" : "NO");
        log.info("\t* OVER_TEMPERATURE  [ %s ]", pdsStatus.OVER_TEMPERATURE ? "YES" : "NO");
        log.info("\t* OVER_CURRENT      [ %s ]", pdsStatus.OVER_CURRENT ? "YES" : "NO");
        log.info("\t* STO_1             [ %s ]", pdsStatus.STO_1 ? "YES" : "NO");
        log.info("\t* STO_2             [ %s ]", pdsStatus.STO_2 ? "YES" : "NO");
        log.info("\t* FDCAN_TIMEOUT     [ %s ]", pdsStatus.FDCAN_TIMEOUT ? "YES" : "NO");
        log.info("\t* SUBMODULE_1_ERROR [ %s ]", pdsStatus.SUBMODULE_1_ERROR ? "YES" : "NO");
        log.info("\t* SUBMODULE_2_ERROR [ %s ]", pdsStatus.SUBMODULE_2_ERROR ? "YES" : "NO");
        log.info("\t* SUBMODULE_3_ERROR [ %s ]", pdsStatus.SUBMODULE_3_ERROR ? "YES" : "NO");
        log.info("\t* SUBMODULE_4_ERROR [ %s ]", pdsStatus.SUBMODULE_4_ERROR ? "YES" : "NO");
        log.info("\t* SUBMODULE_5_ERROR [ %s ]", pdsStatus.SUBMODULE_5_ERROR ? "YES" : "NO");
        log.info("\t* SUBMODULE_6_ERROR [ %s ]", pdsStatus.SUBMODULE_6_ERROR ? "YES" : "NO");
        log.info("\t* CHARGER_DETECTED  [ %s ]", pdsStatus.CHARGER_DETECTED ? "YES" : "NO");

        log.info("---------------------------------");

        log.info("Config data:");
        log.info("shutdown time: [ %u mS ] ", shutdownTime);
        log.info("Battery level 1: %0.2f", batteryLvl1 / 1000.0f);
        log.info("Battery level 2: %0.2f", batteryLvl2 / 1000.0f);

        log.info("---------------------------------");

        log.info("Metrology data:");
        log.info("Bus voltage: %0.2f", pdsBusVoltage / 1000.0f);
    }

    void CandleTool::pdsSetupConfig(u16 id, const std::string& cfgPath)
    {
        using err_E = mab::PdsModule::error_E;

        mab::Pds pds(id, *candle);

        mINI::INIFile      pdsCfgFile(cfgPath);
        mINI::INIStructure pdsCfg;
        pdsCfgFile.read(pdsCfg);

        u32 shutdownTime = atoi(pdsCfg["Control board"]["shutdown time"].c_str());
        u32 battLvl1     = atoi(pdsCfg["Control board"]["battery level 1"].c_str());
        u32 battLvl2     = atoi(pdsCfg["Control board"]["battery level 2"].c_str());

        err_E result = pds.setShutdownTime(shutdownTime);
        if (result != err_E::OK)
            log.error("PDS Config error [ %u ] [ %s:%u ]", result, __FILE__, __LINE__);

        result = pds.setBatteryVoltageLevels(battLvl1, battLvl2);
        if (result != err_E::OK)
            log.error("PDS Config error [ %u ] [ %s:%u ]", result, __FILE__, __LINE__);
    }

    // Fill Power stage Ini structure
    static void fillPsIni(PowerStage & ps, mINI::INIStructure & rIni, std::string sectionName)
    {
        socketIndex_E brSocket         = socketIndex_E::UNASSIGNED;
        u32           brTriggerVoltage = 0;
        u32           ocdLevel         = 0;
        u32           ocdDelay         = 0;
        f32           temperatureLimit = 0.0f;

        ps.getBindBrakeResistor(brSocket);
        ps.getBrakeResistorTriggerVoltage(brTriggerVoltage);
        ps.getOcdLevel(ocdLevel);
        ps.getOcdDelay(ocdDelay);
        ps.getTemperatureLimit(temperatureLimit);

        rIni[sectionName]["type"]      = PdsModule::moduleType2String(moduleType_E::POWER_STAGE);
        rIni[sectionName]["BR Socket"] = floatToString((uint8_t)brSocket);
        rIni[sectionName]["BR Trigger voltage"] = floatToString(brTriggerVoltage);
        rIni[sectionName]["OCD level"]          = floatToString(ocdLevel);
        rIni[sectionName]["OCD delay"]          = floatToString(ocdDelay);
    }

    // Fill Brake resistor Ini structure
    static void fillBrIni(BrakeResistor & br, mINI::INIStructure & rIni, std::string sectionName)
    {
        f32 temperatureLimit = 0.0f;

        br.getTemperatureLimit(temperatureLimit);

        rIni[sectionName]["type"] = PdsModule::moduleType2String(moduleType_E::BRAKE_RESISTOR);
    }

    // Fill Brake resistor Ini structure
    static void fillIcIni(IsolatedConv & ic, mINI::INIStructure & rIni, std::string sectionName)
    {
        f32 temperatureLimit = 0.0f;

        ic.getTemperatureLimit(temperatureLimit);

        rIni[sectionName]["type"] = PdsModule::moduleType2String(moduleType_E::ISOLATED_CONVERTER);
    }

    static void fullModuleIni(
        Pds & pds, moduleType_E moduleType, mINI::INIStructure & rIni, socketIndex_E socketIndex)
    {
        std::string sectionName = "Submodule " + std::to_string((int)socketIndex);

        switch (moduleType)
        {
            case moduleType_E::UNDEFINED:
                rIni[sectionName]["type"] = "NO MODULE";
                break;

            case moduleType_E::CONTROL_BOARD:
                break;

            case moduleType_E::BRAKE_RESISTOR:
            {
                auto br = pds.attachBrakeResistor(socketIndex);
                fillBrIni(*br, rIni, sectionName);
                break;
            }

            case moduleType_E::ISOLATED_CONVERTER:
            {
                auto ic = pds.attachIsolatedConverter(socketIndex);
                fillIcIni(*ic, rIni, sectionName);
                break;
            }

            case moduleType_E::POWER_STAGE:
            {
                auto ps = pds.attachPowerStage(socketIndex);
                fillPsIni(*ps, rIni, sectionName);
                break;
            }

                /* NEW MODULE TYPES HERE */

            default:
                break;
        }
    }

    void CandleTool::pdsReadConfig(u16 id, const std::string& cfgPath)
    {
        mINI::INIStructure readIni; /**< mINI structure for read data */
        Pds                pds(id, *candle);
        u32                shutDownTime = 0;
        u32                batLvl1      = 0;
        u32                batLvl2      = 0;
        Pds::modulesSet_S  pdsModules   = pds.getModules();

        std::string configName = cfgPath;
        if (std::filesystem::path(configName).extension() == "")
            configName += ".cfg";

        pds.getShutdownTime(shutDownTime);
        pds.getBatteryVoltageLevels(batLvl1, batLvl2);

        readIni["Control board"]["CAN ID"]          = floatToString(id);
        readIni["Control board"]["CAN BAUD"]        = "";
        readIni["Control board"]["shutdown time"]   = floatToString(shutDownTime);
        readIni["Control board"]["battery level 1"] = floatToString(batLvl1);
        readIni["Control board"]["battery level 2"] = floatToString(batLvl2);
        fullModuleIni(pds, pdsModules.moduleTypeSocket1, readIni, socketIndex_E::SOCKET_1);
        fullModuleIni(pds, pdsModules.moduleTypeSocket2, readIni, socketIndex_E::SOCKET_2);
        fullModuleIni(pds, pdsModules.moduleTypeSocket3, readIni, socketIndex_E::SOCKET_3);
        fullModuleIni(pds, pdsModules.moduleTypeSocket4, readIni, socketIndex_E::SOCKET_4);
        fullModuleIni(pds, pdsModules.moduleTypeSocket5, readIni, socketIndex_E::SOCKET_5);
        fullModuleIni(pds, pdsModules.moduleTypeSocket6, readIni, socketIndex_E::SOCKET_6);

        mINI::INIFile configFile(configName);
        configFile.write(readIni);
    }

    void CandleTool::pdsStoreConfig(u16 id)
    {
        using err_E = mab::PdsModule::error_E;

        mab::Pds pds(id, *candle);

        err_E result = pds.saveConfig();

        if (result != err_E::OK)
            log.error("PDS Configuration save error [ %u ] [ %s:%u ]", result, __FILE__, __LINE__);
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
            std::memcpy(page.data(), &candleFirmware.m_fwEntry.data[i], page.size());
            u32 crc = crc32(page.data(), page.size());
            if (candle_bootloader->writePage(page, crc) != candleTypes::Error_t::OK)
            {
                log.error("Candle flashing failed!");
                break;
            }
        }
        // mab::FirmwareUploader firmwareUploader(*candle, mabFile);
        // firmwareUploader.flashDevice(noReset);
    }

    void CandleTool::updateMd(const std::string& mabFilePath, uint16_t canId, bool noReset)
    {
        MabFileParser mabFile(mabFilePath, MabFileParser::TargetDevice_E::MD);
        // mab::FirmwareUploader firmwareUploader(*candle, mabFile, canId);
        // if (firmwareUploader.flashDevice(noReset))
        //     log.success("Update complete for MD @ %d", canId);
        // TODO: implement this
    }

    void CandleTool::updatePds(const std::string& mabFilePath, uint16_t canId, bool noReset)
    {
        MabFileParser mabFile(mabFilePath, MabFileParser::TargetDevice_E::PDS);
        // mab::FirmwareUploader firmwareUploader(*candle, mabFile, canId);
        // if (firmwareUploader.flashDevice(noReset))
        //     log.success("Update complete for PDS @ %d", canId);
    }

    void CandleTool::blink(u16 id)
    {
        MD md(id, candle);
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
        if (!tryAddMD80(id))
            return;
        mab::Md80& md = candle->md80s[0];
        candle->controlMd80Mode(id, mab::Md80Mode_E::IDLE);
        candle->controlMd80Enable(id, true);
        candle->begin();
        while (1)
        {
            log.info(
                "[%4d] Position: %6.2f, Velocity: %6.1f", id, md.getPosition(), md.getVelocity());
            usleep(100000);
        }
        candle->end();
    }
    void CandleTool::bus(const std::string& bus, const std::string& device)
    {
#ifdef WIN32
        log.error("bus - option not available on Windows OS.");
        return;
#endif
        if (bus != "USB" && bus != "SPI" && bus != "UART")
            return;
        if ((bus == "SPI" || bus == "UART") && device == "")
        {
            log.error("Bus: %s, requires specifying device!", bus.c_str());
            return;
        }
        mINI::INIFile      file(getCandletoolConfigPath());
        mINI::INIStructure ini;
        file.read(ini);
        ini["communication"]["bus"]    = bus;
        ini["communication"]["device"] = device;
        if (!file.write(ini))
            log.error("failed to write ini file");
    }

    void CandleTool::clearErrors(u16 id, const std::string& level)
    {
        if (level == "error")
            m_candle->setupMd80ClearErrors(id);
        if (level == "warning")
            m_candle->setupMd80ClearWarnings(id);
        else
        {
            m_candle->setupMd80ClearErrors(id);
            m_candle->setupMd80ClearWarnings(id);
        }
    }

    void CandleTool::reset(u16 id)
    {
        m_candle->setupMd80PerformReset(id);
    }

    mab::CANdleBaudrate_E CandleTool::checkSpeedForId(u16 id)
    {
        std::initializer_list<mab::CANdleBaudrate_E> bauds = {mab::CANdleBaudrate_E::CAN_BAUD_1M,
                                                              mab::CANdleBaudrate_E::CAN_BAUD_2M,
                                                              mab::CANdleBaudrate_E::CAN_BAUD_5M,
                                                              mab::CANdleBaudrate_E::CAN_BAUD_8M};

        for (auto& baud : bauds)
        {
            m_candle->configCandleBaudrate(baud);
            if (m_candle->checkMd80ForBaudrate(id))
                return baud;
        }

        return mab::CANdleBaudrate_E::CAN_BAUD_1M;
    }

    u8 CandleTool::getNumericParamFromList(std::string & param,
                                           const std::vector<std::string>& list)
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
    bool CandleTool::getField(mINI::INIStructure & cfg,
                              mINI::INIStructure & ini,
                              std::string category,
                              std::string field,
                              T & value)
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
        u32 calibrationStatus = 0;
        m_candle->readMd80Register(id, mab::Md80Reg_E::calibrationErrors, calibrationStatus);

        if (calibrationStatus & (1 << ui::calibrationErrorList.at(std::string("ERROR_SETUP"))))
        {
            log.error(
                "Could not proceed due to %s. Please call candletool setup motor <ID> <cfg> "
                "first.",
                RED__("ERROR_SETUP"));
            return true;
        }

        return false;
    }
