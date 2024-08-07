#include "candletool.hpp"

#include <numeric>
#include <unistd.h>

#include "ui.hpp"
#include "configHelpers.hpp"

#include "uploader.hpp"
#include "mabFileParser.hpp"

f32 lerp(f32 start, f32 end, f32 t)
{
    return (start * (1.f - t)) + (end * t);
}
std::string floatToString(f32 value, bool noDecimals = false)
{
    std::stringstream ss;
    ss << std::fixed;

    if (noDecimals)
    {
        ss << std::setprecision(0);
        ss << value;
        return ss.str();
    }
    else
    {
        if (static_cast<int>(value) == value)
        {
            ss << std::setprecision(1);
            ss << value;
            return ss.str();
        }
        else
        {
            ss << std::setprecision(7);
            ss << value;
            std::string str = ss.str();
            return str.substr(0, str.find_last_not_of('0') + 1);
        }
    }
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
    std::cerr << "[CANDLESDK] Version: " << mab::Candle::getVersion() << std::endl;
    log.tag = "candletool";

    mab::BusType_E        busType = mab::BusType_E::USB;
    mab::CANdleBaudrate_E baud    = mab::CANdleBaudrate_E::CAN_BAUD_1M;

    mINI::INIFile      file(getCandletoolConfigPath());
    mINI::INIStructure ini;
    file.read(ini);

    busString = ini["communication"]["bus"];
    if (busString == "SPI")
        busType = mab::BusType_E::SPI;
    else if (busString == "UART")
        busType = mab::BusType_E::UART;
    else if (busString == "USB")
        busType = mab::BusType_E::USB;
    std::string& device = ini["communication"]["device"];
    try
    {
        if (device != "" && busType != mab::BusType_E::USB)
            candle = std::make_unique<mab::Candle>(baud, printVerbose, busType, device);
        else
            candle = std::make_unique<mab::Candle>(baud, printVerbose, busType);
    }

    catch (const char* e)
    {
        return;
    }

    return;
}

void CandleTool::ping(const std::string& variant)
{
    if (variant == "all")
    {
        candle->ping(mab::CANdleBaudrate_E::CAN_BAUD_1M);
        candle->ping(mab::CANdleBaudrate_E::CAN_BAUD_2M);
        candle->ping(mab::CANdleBaudrate_E::CAN_BAUD_5M);
        candle->ping(mab::CANdleBaudrate_E::CAN_BAUD_8M);
        return;
    }
    candle->ping(str2baud(variant));
}

void CandleTool::configCan(
    u16 id, u16 newId, const std::string& baud, u16 timeout, bool termination)
{
    checkSpeedForId(id);
    candle->configMd80Can(id, newId, str2baud(baud), timeout, termination);
}
void CandleTool::configSave(u16 id)
{
    candle->configMd80Save(id);
}

void CandleTool::configZero(u16 id)
{
    candle->controlMd80SetEncoderZero(id);
}

void CandleTool::configCurrent(u16 id, f32 current)
{
    candle->configMd80SetCurrentLimit(id, current);
}

void CandleTool::configBandwidth(u16 id, f32 bandwidth)
{
    candle->configMd80TorqueBandwidth(id, bandwidth);
}

void CandleTool::configClear(u16 id)
{
    if (!tryAddMD80(id))
        return;
    if (candle->writeMd80Register(id, mab::Md80Reg_E::runRestoreFactoryConfig, true))
        log.success("Config reverted to factory state!");
    else
        log.error("Error reverting config to factory state!");
}

void CandleTool::setupCalibration(u16 id)
{
    if (!ui::getCalibrationConfirmation() || checkSetupError(id))
        return;

    candle->setupMd80Calibration(id);
}

void CandleTool::setupCalibrationOutput(u16 id)
{
    if (!ui::getCalibrationOutputConfirmation() || checkSetupError(id))
        return;
    u16 outputEncoder = 0;
    candle->readMd80Register(id, mab::Md80Reg_E::outputEncoder, outputEncoder);
    if (!outputEncoder)
    {
        log.error("No output encoder is configured!");
        return;
    }
    candle->setupMd80CalibrationOutput(id);
}

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
        logger::LogLevel_E prePromptLevel = log.level;
        log.level                         = logger::LogLevel_E::INFO;
        log.error("\"%s\" is incomplete.", finalConfigPath.c_str());
        log.info("Generate updated file with all required fileds? [y/n]");
        if (!getConfirmation())
            exit(0);
        finalConfigPath = generateUpdatedConfigFile(finalConfigPath);
        log.info("Generated updated file \"%s\"", finalConfigPath.c_str());
        log.info("Setup MD with newly generated config? [y/n]");
        if (!getConfirmation())
            exit(0);
        log.level = prePromptLevel;
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
        log.warn("Ommiting config validation on user request!");
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

    if (!tryAddMD80(id))
        return;
    mab::regWrite_st& regW = candle->getMd80FromList(id).getWriteReg();
    mab::regRead_st&  regR = candle->getMd80FromList(id).getReadReg();

    /* add a field here only if you want to test it against limits form the candletool.ini file */
    memcpy(
        regW.RW.motorName, (cfg["motor"]["name"]).c_str(), strlen((cfg["motor"]["name"]).c_str()));
    if (!getField(cfg, ini, "motor", "pole pairs", regW.RW.polePairs))
        return;
    if (!getField(cfg, ini, "motor", "torque constant", regW.RW.motorKt))
        return;
    if (!getField(cfg, ini, "motor", "KV", regW.RW.motorKV))
        return;
    if (!getField(cfg, ini, "motor", "gear ratio", regW.RW.gearRatio))
        return;
    if (!getField(cfg, ini, "motor", "max current", regW.RW.iMax))
        return;
    if (!getField(cfg, ini, "motor", "torque constant a", regW.RW.motorKt_a))
        return;
    if (!getField(cfg, ini, "motor", "torque constant b", regW.RW.motorKt_b))
        return;
    if (!getField(cfg, ini, "motor", "torque constant c", regW.RW.motorKt_c))
        return;
    if (!getField(cfg, ini, "motor", "torque bandwidth", regW.RW.torqueBandwidth))
        return;
    if (!getField(cfg, ini, "motor", "dynamic friction", regW.RW.friction))
        return;
    if (!getField(cfg, ini, "motor", "static friction", regW.RW.stiction))
        return;
    if (!getField(cfg, ini, "motor", "shutdown temp", regW.RW.motorShutdownTemp))
        return;
    if (!getField(cfg, ini, "limits", "max velocity", regW.RW.maxVelocity))
        return;
    if (!getField(cfg, ini, "limits", "max acceleration", regW.RW.maxAcceleration))
        return;
    if (!getField(cfg, ini, "limits", "max deceleration", regW.RW.maxDeceleration))
        return;

    auto checkFieldWriteIfPopulated =
        [&](const char* category, const char* field, auto& fieldVar, mab::Md80Reg_E reg) -> bool
    {
        if (cfg[category][field] == "")
            return true;

        if (!getField(cfg, ini, category, field, fieldVar))
            return false;

        if (!candle->writeMd80Register(id, reg, fieldVar))
        {
            log.error("Failed to setup motor! Error writing: 0x%x register.", (u16)reg);
            return false;
        }
        return true;
    };

    if (!checkFieldWriteIfPopulated("hardware",
                                    "shunt resistance",
                                    regR.RO.shuntResistance,
                                    mab::Md80Reg_E::shuntResistance))
        return;

    if (!checkFieldWriteIfPopulated("motor",
                                    "reverse direction",
                                    regR.RW.reverseDirection,
                                    mab::Md80Reg_E::reverseDirection))
        return;

    regW.RW.motorCalibrationMode =
        getNumericParamFromList(cfg["motor"]["calibration mode"], ui::motorCalibrationModes);

    regW.RW.outputEncoderDefaultBaud =
        atoi(cfg["output encoder"]["output encoder default baud"].c_str());
    regW.RW.outputEncoder =
        getNumericParamFromList(cfg["output encoder"]["output encoder"], ui::encoderTypes);
    regW.RW.outputEncoderMode =
        getNumericParamFromList(cfg["output encoder"]["output encoder mode"], ui::encoderModes);
    regW.RW.outputEncoderCalibrationMode = getNumericParamFromList(
        cfg["output encoder"]["output encoder calibration mode"], ui::encoderCalibrationModes);
    regW.RW.homingMode = getNumericParamFromList(cfg["homing"]["mode"], ui::homingModes);
    regW.RW.brakeMode  = getNumericParamFromList(cfg["brake"]["mode"], ui::brakeModes);

    auto f32FromField = [&](const char* category, const char* field) -> f32
    { return atof(cfg[category][field].c_str()); };

    /* motor base config */
    if (!candle->writeMd80Register(id,
                                   mab::Md80Reg_E::motorName,
                                   regW.RW.motorName,
                                   mab::Md80Reg_E::motorPolePairs,
                                   regW.RW.polePairs,
                                   mab::Md80Reg_E::motorKt,
                                   regW.RW.motorKt,
                                   mab::Md80Reg_E::motorKV,
                                   regW.RW.motorKV,
                                   mab::Md80Reg_E::motorGearRatio,
                                   regW.RW.gearRatio,
                                   mab::Md80Reg_E::motorIMax,
                                   regW.RW.iMax,
                                   mab::Md80Reg_E::motorTorgueBandwidth,
                                   regW.RW.torqueBandwidth))
        log.error("Failed to setup motor!");

    /* motor advanced config */
    if (!candle->writeMd80Register(id,
                                   mab::Md80Reg_E::motorFriction,
                                   regW.RW.friction,
                                   mab::Md80Reg_E::motorStiction,
                                   regW.RW.stiction,
                                   mab::Md80Reg_E::motorKt_a,
                                   regW.RW.motorKt_a,
                                   mab::Md80Reg_E::motorKt_b,
                                   regW.RW.motorKt_b,
                                   mab::Md80Reg_E::motorKt_c,
                                   regW.RW.motorKt_c,
                                   mab::Md80Reg_E::outputEncoder,
                                   regW.RW.outputEncoder,
                                   mab::Md80Reg_E::outputEncoderMode,
                                   regW.RW.outputEncoderMode,
                                   mab::Md80Reg_E::outputEncoderDefaultBaud,
                                   regW.RW.outputEncoderDefaultBaud))
        log.error("Failed to setup motor!");

    /* motor motion config - Position and velocity PID*/
    if (!candle->writeMd80Register(id,
                                   mab::Md80Reg_E::motorPosPidKp,
                                   f32FromField("position PID", "kp"),
                                   mab::Md80Reg_E::motorPosPidKi,
                                   f32FromField("position PID", "ki"),
                                   mab::Md80Reg_E::motorPosPidKd,
                                   f32FromField("position PID", "kd"),
                                   mab::Md80Reg_E::motorPosPidWindup,
                                   f32FromField("position PID", "windup"),
                                   mab::Md80Reg_E::motorVelPidKp,
                                   f32FromField("velocity PID", "kp"),
                                   mab::Md80Reg_E::motorVelPidKi,
                                   f32FromField("velocity PID", "ki"),
                                   mab::Md80Reg_E::motorVelPidKd,
                                   f32FromField("velocity PID", "kd"),
                                   mab::Md80Reg_E::motorVelPidWindup,
                                   f32FromField("velocity PID", "windup")))
        log.error("Failed to setup motor!");

    /* motor motion config - Impedance PD*/
    if (!candle->writeMd80Register(id,
                                   mab::Md80Reg_E::motorImpPidKp,
                                   f32FromField("impedance PD", "kp"),
                                   mab::Md80Reg_E::motorImpPidKd,
                                   f32FromField("impedance PD", "kd"),
                                   mab::Md80Reg_E::motorShutdownTemp,
                                   regW.RW.motorShutdownTemp))
        log.error("Failed to setup motor!");

    if (!candle->writeMd80Register(
            id, mab::Md80Reg_E::outputEncoderCalibrationMode, regW.RW.outputEncoderCalibrationMode))
        log.error("Failed to setup motor!");

    if (!candle->writeMd80Register(
            id, mab::Md80Reg_E::motorCalibrationMode, regW.RW.motorCalibrationMode))
        log.error("Failed to setup motor!");

    if (!candle->writeMd80Register(id,
                                   mab::Md80Reg_E::homingMode,
                                   regW.RW.homingMode,
                                   mab::Md80Reg_E::homingMaxTravel,
                                   f32FromField("homing", "max travel"),
                                   mab::Md80Reg_E::homingTorque,
                                   f32FromField("homing", "max torque"),
                                   mab::Md80Reg_E::homingVelocity,
                                   f32FromField("homing", "max velocity")))
        log.error("Failed to setup motor!");

    if (!candle->writeMd80Register(id,
                                   mab::Md80Reg_E::maxTorque,
                                   f32FromField("limits", "max torque"),
                                   mab::Md80Reg_E::maxAcceleration,
                                   regW.RW.maxAcceleration,
                                   mab::Md80Reg_E::maxDeceleration,
                                   regW.RW.maxDeceleration,
                                   mab::Md80Reg_E::maxVelocity,
                                   regW.RW.maxVelocity,
                                   mab::Md80Reg_E::positionLimitMin,
                                   f32FromField("limits", "min position"),
                                   mab::Md80Reg_E::positionLimitMax,
                                   f32FromField("limits", "max position")))
        log.error("Failed to setup motor!");

    if (!candle->writeMd80Register(id,
                                   mab::Md80Reg_E::profileAcceleration,
                                   f32FromField("profile", "acceleration"),
                                   mab::Md80Reg_E::profileDeceleration,
                                   f32FromField("profile", "deceleration"),
                                   mab::Md80Reg_E::quickStopDeceleration,
                                   f32FromField("profile", "quick stop deceleration"),
                                   mab::Md80Reg_E::profileVelocity,
                                   f32FromField("profile", "velocity")))
        log.error("Failed to setup motor!");

    if (!candle->writeMd80Register(id, mab::Md80Reg_E::brakeMode, regW.RW.brakeMode))
        log.error("Failed to setup motor!");

    if (candle->configMd80Save(id))
    {
        log.success("Config save sucessfully!");
        log.info("Rebooting md80...");
    }
    /* wait for a full reboot */
    sleep(3);
    if (candle->controlMd80Enable(id, false))
        log.success("Ready!");
    else
        log.warn("Failed to reboot (ID: %d)!", id);
}
void CandleTool::setupReadConfig(u16 id, const std::string& cfgName)
{
    if (!tryAddMD80(id))
        return;

    mINI::INIStructure readIni; /**< mINI structure for read data */
    mab::regRead_st&   regR = candle->getMd80FromList(id).getReadReg(); /**< read register */
    char               motorNameChar[24];

    std::string configName = cfgName;
    if (cfgName == "")
    {
        if (!candle->readMd80Register(id, mab::Md80Reg_E::motorName, motorNameChar))
        {
            log.error("Failed to read motor conifg %d!", id);
            snprintf(motorNameChar, 24, "UNKNOWN_MD");
        }
        configName = std::string(motorNameChar) + "_" + std::to_string(id) + "_read.cfg";
    }
    else if (std::filesystem::path(configName).extension() == "")
        configName += ".cfg";

    /* Ask user if the motor config should be saved */
    bool saveConfig = ui::getSaveMotorConfigConfirmation(configName);

    /* Motor config - motor section */
    if (!candle->readMd80Register(id,
                                  mab::Md80Reg_E::motorPolePairs,
                                  regR.RW.polePairs,
                                  mab::Md80Reg_E::motorKt,
                                  regR.RW.motorKt,
                                  mab::Md80Reg_E::motorIMax,
                                  regR.RW.iMax,
                                  mab::Md80Reg_E::motorGearRatio,
                                  regR.RW.gearRatio,
                                  mab::Md80Reg_E::motorTorgueBandwidth,
                                  regR.RW.torqueBandwidth,
                                  mab::Md80Reg_E::motorKV,
                                  regR.RW.motorKV))
        log.warn("Failed to read motor config!");

    readIni["motor"]["name"]             = std::string(motorNameChar);
    readIni["motor"]["pole pairs"]       = floatToString(regR.RW.polePairs);
    readIni["motor"]["KV"]               = floatToString(regR.RW.motorKV);
    readIni["motor"]["torque constant"]  = floatToString(regR.RW.motorKt);
    readIni["motor"]["gear ratio"]       = floatToString(regR.RW.gearRatio);
    readIni["motor"]["max current"]      = floatToString(regR.RW.iMax);
    readIni["motor"]["torque bandwidth"] = floatToString(regR.RW.torqueBandwidth);

    if (!candle->readMd80Register(id, mab::Md80Reg_E::motorShutdownTemp, regR.RW.motorShutdownTemp))
        log.warn("Failed to read motor config!");

    readIni["motor"]["shutdown temp"] = floatToString(regR.RW.motorShutdownTemp);

    /* Motor config - limits section */
    if (!candle->readMd80Register(id,
                                  mab::Md80Reg_E::positionLimitMax,
                                  regR.RW.positionLimitMax,
                                  mab::Md80Reg_E::positionLimitMin,
                                  regR.RW.positionLimitMin,
                                  mab::Md80Reg_E::maxTorque,
                                  regR.RW.maxTorque,
                                  mab::Md80Reg_E::maxVelocity,
                                  regR.RW.maxVelocity,
                                  mab::Md80Reg_E::maxAcceleration,
                                  regR.RW.maxAcceleration,
                                  mab::Md80Reg_E::maxDeceleration,
                                  regR.RW.maxDeceleration))
        log.warn("Failed to read motor config!");

    readIni["limits"]["max torque"]       = floatToString(regR.RW.maxTorque);
    readIni["limits"]["max velocity"]     = floatToString(regR.RW.maxVelocity);
    readIni["limits"]["max position"]     = floatToString(regR.RW.positionLimitMax);
    readIni["limits"]["min position"]     = floatToString(regR.RW.positionLimitMin);
    readIni["limits"]["max acceleration"] = floatToString(regR.RW.maxAcceleration);
    readIni["limits"]["max deceleration"] = floatToString(regR.RW.maxDeceleration);

    /* Motor config - profile section */
    if (!candle->readMd80Register(id,
                                  mab::Md80Reg_E::profileVelocity,
                                  regR.RW.profileVelocity,
                                  mab::Md80Reg_E::profileAcceleration,
                                  regR.RW.profileAcceleration,
                                  mab::Md80Reg_E::profileDeceleration,
                                  regR.RW.profileDeceleration))
        log.warn("Failed to read motor config!");

    readIni["profile"]["acceleration"] = floatToString(regR.RW.profileAcceleration);
    readIni["profile"]["deceleration"] = floatToString(regR.RW.profileDeceleration);
    readIni["profile"]["velocity"]     = floatToString(regR.RW.profileVelocity);

    /* Motor config - output encoder section */
    if (!candle->readMd80Register(id,
                                  mab::Md80Reg_E::outputEncoder,
                                  regR.RW.outputEncoder,
                                  mab::Md80Reg_E::outputEncoderMode,
                                  regR.RW.outputEncoderMode))
        log.warn("Failed to read motor config!");

    if (regR.RW.outputEncoder == 0.f)
        readIni["output encoder"]["output encoder"] = floatToString(0.f, true);
    else
        readIni["output encoder"]["output encoder"] = ui::encoderTypes[regR.RW.outputEncoder];

    if (regR.RW.outputEncoderMode == 0.f)
        readIni["output encoder"]["output encoder mode"] = floatToString(0.f, true);
    else
        readIni["output encoder"]["output encoder mode"] =
            ui::encoderModes[regR.RW.outputEncoderMode];

    /* Motor config - position PID section */
    if (!candle->readMd80Register(id,
                                  mab::Md80Reg_E::motorPosPidKp,
                                  regR.RW.positionPidGains.kp,
                                  mab::Md80Reg_E::motorPosPidKi,
                                  regR.RW.positionPidGains.ki,
                                  mab::Md80Reg_E::motorPosPidKd,
                                  regR.RW.positionPidGains.kd,
                                  mab::Md80Reg_E::motorPosPidWindup,
                                  regR.RW.positionPidGains.intWindup))
        log.warn("Failed to read motor config!");

    readIni["position PID"]["kp"]     = floatToString(regR.RW.positionPidGains.kp);
    readIni["position PID"]["ki"]     = floatToString(regR.RW.positionPidGains.ki);
    readIni["position PID"]["kd"]     = floatToString(regR.RW.positionPidGains.kd);
    readIni["position PID"]["windup"] = floatToString(regR.RW.positionPidGains.intWindup);

    /* Motor config - velocity PID section */
    if (!candle->readMd80Register(id,
                                  mab::Md80Reg_E::motorVelPidKp,
                                  regR.RW.velocityPidGains.kp,
                                  mab::Md80Reg_E::motorVelPidKi,
                                  regR.RW.velocityPidGains.ki,
                                  mab::Md80Reg_E::motorVelPidKd,
                                  regR.RW.velocityPidGains.kd,
                                  mab::Md80Reg_E::motorVelPidWindup,
                                  regR.RW.velocityPidGains.intWindup))
        log.warn("Failed to read motor config!");

    readIni["velocity PID"]["kp"]     = floatToString(regR.RW.velocityPidGains.kp);
    readIni["velocity PID"]["ki"]     = floatToString(regR.RW.velocityPidGains.ki);
    readIni["velocity PID"]["kd"]     = floatToString(regR.RW.velocityPidGains.kd);
    readIni["velocity PID"]["windup"] = floatToString(regR.RW.velocityPidGains.intWindup);

    /* Motor config - impedance PD section */
    if (!candle->readMd80Register(id,
                                  mab::Md80Reg_E::motorImpPidKp,
                                  regR.RW.impedancePdGains.kp,
                                  mab::Md80Reg_E::motorImpPidKd,
                                  regR.RW.impedancePdGains.kd))
        log.warn("Failed to read motor config!");

    readIni["impedance PD"]["kp"] = floatToString(regR.RW.impedancePdGains.kp);
    readIni["impedance PD"]["kd"] = floatToString(regR.RW.impedancePdGains.kd);

    /* Motor config - homing section */
    if (!candle->readMd80Register(id,
                                  mab::Md80Reg_E::homingMode,
                                  regR.RW.homingMode,
                                  mab::Md80Reg_E::homingMaxTravel,
                                  regR.RW.homingMaxTravel,
                                  mab::Md80Reg_E::homingVelocity,
                                  regR.RW.homingVelocity,
                                  mab::Md80Reg_E::homingTorque,
                                  regR.RW.homingTorque))
        log.warn("Failed to read motor config!");

    readIni["homing"]["mode"]         = ui::homingModes[regR.RW.homingMode];
    readIni["homing"]["max travel"]   = floatToString(regR.RW.homingMaxTravel);
    readIni["homing"]["max torque"]   = floatToString(regR.RW.homingTorque);
    readIni["homing"]["max velocity"] = floatToString(regR.RW.homingVelocity);

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
    checkSpeedForId(id);
    if (!candle->addMd80(id))
        return;
    candle->setupMd80DiagnosticExtended(id);
    ui::printDriveInfoExtended(candle->getMd80FromList(id), printAll);
}

void CandleTool::setupHoming(u16 id)
{
    candle->setupMd80PerformHoming(id);
}

void CandleTool::testMove(u16 id, f32 targetPosition)
{
    if (targetPosition > 10.0f)
        targetPosition = 10.0f;
    if (targetPosition < -10.0f)
        targetPosition = -10.0f;
    if (!tryAddMD80(id))
        return;
    if (hasError(id))
        return;

    mab::Md80& md = candle->md80s[0];
    candle->controlMd80Mode(id, mab::Md80Mode_E::IMPEDANCE);
    f32 pos = md.getPosition();
    md.setTargetPosition(pos);
    targetPosition += pos;

    candle->controlMd80Enable(id, true);
    candle->begin();
    for (f32 t = 0.f; t < 1.f; t += 0.01f)
    {
        f32 target = lerp(pos, targetPosition, t);
        log.level  = logger::LogLevel_E::DEBUG;
        md.setTargetPosition(target);
        log.info("[%4d] Position: %4.2f, Velocity: %4.1f", id, md.getPosition(), md.getVelocity());
        usleep(30000);
    }
    candle->end();
}

void CandleTool::testMoveAbsolute(u16 id, f32 targetPos, f32 velLimit, f32 accLimit, f32 dccLimit)
{
    if (!tryAddMD80(id))
        return;

    if (hasError(id))
        return;
    if (velLimit > 0)
        candle->writeMd80Register(id, mab::Md80Reg_E::profileVelocity, velLimit);
    if (accLimit > 0)
        candle->writeMd80Register(id, mab::Md80Reg_E::profileAcceleration, accLimit);
    if (dccLimit > 0)
        candle->writeMd80Register(id, mab::Md80Reg_E::profileDeceleration, dccLimit);

    candle->controlMd80Mode(id, mab::Md80Mode_E::POSITION_PROFILE);
    candle->controlMd80Enable(id, true);
    candle->begin();
    candle->md80s[0].setTargetPosition(targetPos);
    while (!candle->md80s[0].isTargetPositionReached())
        sleep(1);
    log.info("TARGET REACHED!");
    candle->end();
}

void CandleTool::testLatency(const std::string& canBaudrate)
{
#ifdef UNIX
    struct sched_param sp;
    memset(&sp, 0, sizeof(sp));
    sp.sched_priority = 99;
    sched_setscheduler(0, SCHED_FIFO, &sp);
#endif
#ifdef WIN32
    SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);
#endif
    auto ids = candle->ping(str2baud(canBaudrate));
    if (ids.size() == 0)
        return;
    checkSpeedForId(ids[0]);

    for (auto& id : ids)
    {
        candle->addMd80(id);
        candle->controlMd80Mode(id, mab::Md80Mode_E::IMPEDANCE);
    }

    candle->begin();
    std::vector<u32> samples;
    const u32        timelen = 10;
    sleep(1);
    for (u32 i = 0; i < timelen; i++)
    {
        sleep(1);
        samples.push_back(candle->getActualCommunicationFrequency());
        log.info("Current average communication speed: %d Hz", samples[i]);
    }

    /* calculate mean and stdev */
    f32 sum = std::accumulate(std::begin(samples), std::end(samples), 0.0);
    f32 m   = sum / samples.size();

    f32 accum = 0.0;
    std::for_each(
        std::begin(samples), std::end(samples), [&](const f32 d) { accum += (d - m) * (d - m); });
    f32 stdev = sqrt(accum / (samples.size() - 1));

    ui::printLatencyTestResult(ids.size(), m, stdev, busString);

    candle->end();
}

void CandleTool::testEncoderOutput(u16 id)
{
    if (!tryAddMD80(id) && hasError(id))
        return;

    u16 outputEncoder = 0;
    candle->readMd80Register(id, mab::Md80Reg_E::outputEncoder, outputEncoder);
    if (!outputEncoder)
    {
        log.warn("No output encoder on ID: %d! Not testing.", id);
        return;
    }
    candle->setupMd80TestOutputEncoder(id);
}

void CandleTool::testEncoderMain(u16 id)
{
    if (!tryAddMD80(id) && hasError(id))
        return;
    candle->setupMd80TestMainEncoder(id);
}

void CandleTool::registerWrite(u16 id, u16 reg, const std::string& value)
{
    if (!tryAddMD80(id))
        return;

    mab::Md80Reg_E regId    = (mab::Md80Reg_E)reg;
    u32            regValue = atoi(value.c_str());

    bool success = false;

    switch (mab::Register::getType(regId))
    {
        case mab::Register::type::U8:
            success = candle->writeMd80Register(id, regId, (u8)regValue);
            break;
        case mab::Register::type::I8:
            success = candle->writeMd80Register(id, regId, (s8)regValue);
            break;
        case mab::Register::type::U16:
            success = candle->writeMd80Register(id, regId, (u16)regValue);
            break;
        case mab::Register::type::I16:
            success = candle->writeMd80Register(id, regId, (s16)regValue);
            break;
        case mab::Register::type::U32:
            success = candle->writeMd80Register(id, regId, (u32)regValue);
            break;
        case mab::Register::type::I32:
            success = candle->writeMd80Register(id, regId, (s32)regValue);
            break;
        case mab::Register::type::F32:
            success = candle->writeMd80Register(id, regId, (f32)std::atof(value.c_str()));
            break;
        case mab::Register::type::STR:
        {
            char str[24] = {};
            strncpy(str, value.c_str(), sizeof(str));
            success = candle->writeMd80Register(id, regId, str);
            break;
        }
        case mab::Register::type::UNKNOWN:
            log.error("Unknown register! Please check the ID and try again");
    }
    if (success)
        log.success("Writing register successful!");
    else
        log.error("Writing register failed!");
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
            candle->readMd80Register(id, regId, str);
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
    MabFileParser         mabFile(mabFilePath);
    mab::FirmwareUploader firmwareUploader(*candle, mabFile);
    firmwareUploader.flashDevice(noReset);
}

void CandleTool::updateMd(const std::string& mabFilePath, uint16_t canId, bool noReset)
{
    MabFileParser         mabFile(mabFilePath);
    mab::FirmwareUploader firmwareUploader(*candle, mabFile, canId);
    firmwareUploader.flashDevice(noReset);
}

void CandleTool::updatePds(const std::string& mabFilePath, uint16_t canId, bool noReset)
{
    MabFileParser         mabFile(mabFilePath);
    mab::FirmwareUploader firmwareUploader(*candle, mabFile, canId);
    firmwareUploader.flashDevice(noReset);
}

void CandleTool::blink(u16 id)
{
    candle->configMd80Blink(id);
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
        log.info("[%4d] Position: %6.2f, Velocity: %6.1f", id, md.getPosition(), md.getVelocity());
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
        candle->setupMd80ClearErrors(id);
    if (level == "warning")
        candle->setupMd80ClearWarnings(id);
    else
    {
        candle->setupMd80ClearErrors(id);
        candle->setupMd80ClearWarnings(id);
    }
}

void CandleTool::reset(u16 id)
{
    candle->setupMd80PerformReset(id);
}

mab::CANdleBaudrate_E CandleTool::checkSpeedForId(u16 id)
{
    std::initializer_list<mab::CANdleBaudrate_E> bauds = {mab::CANdleBaudrate_E::CAN_BAUD_1M,
                                                          mab::CANdleBaudrate_E::CAN_BAUD_2M,
                                                          mab::CANdleBaudrate_E::CAN_BAUD_5M,
                                                          mab::CANdleBaudrate_E::CAN_BAUD_8M};

    for (auto& baud : bauds)
    {
        candle->configCandleBaudrate(baud);
        if (candle->checkMd80ForBaudrate(id))
            return baud;
    }

    return mab::CANdleBaudrate_E::CAN_BAUD_1M;
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

bool CandleTool::hasError(u16 canId)
{
    candle->setupMd80DiagnosticExtended(canId);

    if (candle->getMd80FromList(canId).getReadReg().RO.mainEncoderErrors & 0x0000ffff ||
        candle->getMd80FromList(canId).getReadReg().RO.outputEncoderErrors & 0x0000ffff ||
        candle->getMd80FromList(canId).getReadReg().RO.calibrationErrors & 0x0000ffff ||
        candle->getMd80FromList(canId).getReadReg().RO.hardwareErrors & 0x0000ffff ||
        candle->getMd80FromList(canId).getReadReg().RO.bridgeErrors & 0x0000ffff ||
        candle->getMd80FromList(canId).getReadReg().RO.communicationErrors & 0x0000ffff)
    {
        log.error("Cannot execute command. MD has error:");
        ui::printAllErrors(candle->md80s[0]);
        return true;
    }

    return false;
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
bool CandleTool::tryAddMD80(u16 id)
{
    checkSpeedForId(id);
    if (!candle->addMd80(id))
    {
        log.error("MD with ID: %d, not found on the bus.", id);
        return false;
    }
    return true;
}
bool CandleTool::checkSetupError(u16 id)
{
    u32 calibrationStatus = 0;
    candle->readMd80Register(id, mab::Md80Reg_E::calibrationErrors, calibrationStatus);

    if (calibrationStatus & (1 << ui::calibrationErrorList.at(std::string("ERROR_SETUP"))))
    {
        log.error(
            "Could not proceed due to %s. Please call candletool setup motor <ID> <cfg> first.",
            RED__("ERROR_SETUP"));
        return true;
    }

    return false;
}
