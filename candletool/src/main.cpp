#include "candle_types.hpp"
#include "candletool.hpp"
#include "candletoolCO.hpp"
#include "configHelpers.hpp"
#include "logger.hpp"
#include "ui.hpp"
#include "CLI/CLI.hpp"
#include "pds_cli.hpp"

int main(int argc, char** argv)
{
    CLI::App app{"candletool"};
    app.fallthrough();
    app.ignore_case();
    UserCommand cmd;

    auto* blink   = app.add_subcommand("blink", "Blink LEDs on MD drive.");
    auto* bus     = app.add_subcommand("bus", "Set CANdle bus parameters.");
    auto* clear   = app.add_subcommand("clear", "Clear Errors and Warnings.");
    auto* config  = app.add_subcommand("config", "Configure CAN parameters of MD drive.");
    auto* encoder = app.add_subcommand("encoder", "Display MD rotor position.");
    auto* mdco    = app.add_subcommand("mdco", "Send CANopen command instead of CAN FD");
    auto* ping    = app.add_subcommand("ping", "Discover MD drives on CAN bus.");
    auto* registr = app.add_subcommand("register", "Access MD drive via register read/write.");
    auto* reset   = app.add_subcommand("reset", "Reset MD drive.");
    auto* setup   = app.add_subcommand("setup", "Setup MD via config files, and calibrate.");
    auto* test    = app.add_subcommand("test", "Basic MD drive testing routines.");
    auto* update  = app.add_subcommand("update", "Firmware update.");

    // Verbosity
    uint32_t verbosityMode = 0;
    bool     silentMode{false};
    app.add_flag("-v{1},--verbosity{1}", verbosityMode, "Verbose modes (1,2,3)")
        ->default_val(0)
        ->expected(0, 1);

    app.add_flag("-s,--silent", silentMode, "Silent mode")->default_val(0);

    std::string logPath = "";
    app.add_flag("--log", logPath, "Redirect output to file")->default_val("")->expected(1);

    // baud
    app.add_option(
           "-b,--baud", cmd.baud, "Select FD CAN Baudrate CANdleTOOL will use for communication.")
        ->default_val("1M")
        ->check(CLI::IsMember({"1M", "2M", "5M", "8M"}))
        ->expected(1);

    // BLINK
    blink->add_option("<CAN_ID>", cmd.id, "CAN ID of the MD to interact with.")->required();

    // BUS
    bus->add_option("<BUS>", cmd.bus, "Can be USB/SPI/UART. Only for CANdleHAT.")->required();
    bus->add_option("<DEVICE>", cmd.variant, "SPI/UART device to use, ie: /dev/spidev1.1.");

    // CLEAR
    clear->add_option("<CAN_ID>", cmd.id, "CAN ID of the MD to interact with.")->required();
    clear->add_option("<LEVEL>", cmd.variant, "Can be: warning, error, all")->required();

    // CONFIG
    auto* configBand = config->add_subcommand(
        "bandwidth", "Set the torque bandwidth without recalibrating the actuator");
    auto* configCan     = config->add_subcommand("can", "Set CAN parameters of MD drive.");
    auto* configClear   = config->add_subcommand("clear");
    auto* configCurrent = config->add_subcommand(
        "current", "Set the maximum phase current that is allowed to flow through the motor.");
    auto* configSave = config->add_subcommand("save", "Save current config to MD flash memory.");
    auto* configZero = config->add_subcommand("zero", "Set MD zero position at current position.");

    configBand->add_option("<CAN_ID>", cmd.id, "CAN ID of the MD to interact with.")->required();
    configBand->add_option("<HZ>", cmd.bandwidth, "Desired torque bandwidth.")->required();
    configCan->add_option("<CAN_ID>", cmd.id, "Current CAN ID.")->required();
    configCan->add_option("<NEW_CAN_ID>", cmd.newId, "New CAN ID to set.")->required();
    configCan->add_option("<CAN_BAUD>", cmd.baud, "New CAN baudrate to set: 1M, 2M, 5M or 8M")
        ->required();
    configCan->add_option("<CAN_WDG>", cmd.canWatchdog, "New CAN watchdog timeout (in ms) to set.")
        ->required();
    configClear->add_option("<CAN_ID>", cmd.id, "CAN ID of the MD to interact with.")->required();
    configCurrent->add_option("<CAN_ID>", cmd.id, "CAN ID of the MD to interact with.")->required();
    configCurrent->add_option("<AMPS>", cmd.current, "Max current in Amps.")->required();
    configSave->add_option("<CAN_ID>", cmd.id, "CAN ID of the MD to interact with.")->required();
    configZero->add_option("<CAN_ID>", cmd.id, "CAN ID of the MD to interact with.")->required();

    // ENCODER
    encoder->add_option("<CAN_ID>", cmd.id, "CAN ID of the MD to interact with.")->required();

    // CANopen command
    auto* blinkco   = mdco->add_subcommand("blink", "Blink LEDs on MD drive.");
    auto* clearco   = mdco->add_subcommand("clear", "Clear Errors and Warnings.");
    auto* configco  = mdco->add_subcommand("config", "Configure CAN parameters of MD drive.");
    auto* encoderco = mdco->add_subcommand("encoder", "Display MD rotor position.");
    auto* pingco    = mdco->add_subcommand("ping", "Discover MD drives on CAN bus.");
    auto* registrco = mdco->add_subcommand("register", "Access MD drive via register read/write.");
    auto* resetco   = mdco->add_subcommand("reset", "Reset MD drive.");
    auto* pdoTestco =
        mdco->add_subcommand("pdo", "Try to send pdo can frame instead of sdo only for CANopen");
    auto* setupco     = mdco->add_subcommand("setup", "Setup MD via config files, and calibrate.");
    auto* testco      = mdco->add_subcommand("test", "Basic MD drive testing routines.");
    auto* heartbeatco = mdco->add_subcommand("heartbeat", "Test heartbeat only for CANopen");
    auto* SDOsegmentedco =
        mdco->add_subcommand("Segmented",
                             "Test a SDO segmented transfer. Available for "
                             "register containing data with size over 4 bit only for CANopen");
    // BLINK CANopen
    blinkco->add_option("<CAN_ID>", cmd.id, "CAN ID of the MD to interact with.")->required();
    // CLEAR
    clearco->add_option("<CAN_ID>", cmd.id, "CAN ID of the MD to interact with.")->required();
    clearco->add_option("<LEVEL>", cmd.variant, "Can be: warning, error, all")->required();

    // CONFIG CANopen
    auto* configBandco = configco->add_subcommand(
        "bandwidth", "Set the torque bandwidth without recalibrating the actuator");
    auto* configCanco = configco->add_subcommand("can", "Set CAN parameters of MD drive.");
    auto* configSaveco =
        configco->add_subcommand("save", "Save current config to MD flash memory.");
    auto* configZeroco =
        configco->add_subcommand("zero", "Set MD zero position at current position.");

    configBandco->add_option("<CAN_ID>", cmd.id, "CAN ID of the MD to interact with.")->required();
    configBandco->add_option("<HZ>", cmd.bandwidth, "Desired torque bandwidth.")->required();
    configCanco->add_option("<CAN_ID>", cmd.id, "Current CAN ID.")->required();
    configCanco->add_option("<NEW_CAN_ID>", cmd.newId, "New CAN ID to set.")->required();
    configCanco->add_option("<CAN_BAUD>", cmd.baud, "New CAN baudrate to set: 1M, 2M, 5M or 8M")
        ->required();
    configCanco
        ->add_option("<CAN_WDG>", cmd.canWatchdog, "New CAN watchdog timeout (in ms) to set.")
        ->required();
    configSaveco->add_option("<CAN_ID>", cmd.id, "CAN ID of the MD to interact with.")->required();
    configZeroco->add_option("<CAN_ID>", cmd.id, "CAN ID of the MD to interact with.")->required();

    // ENCODER CANopen
    encoderco->add_option("<CAN_ID>", cmd.id, "CAN ID of the MD to interact with.")->required();

    // PING CANopen
    pingco->add_option(
        "<CAN_BAUD>", cmd.variant, "Can be one of: 1M, 2M, 5M, 8M, all. Default: all");

    // REGISTER CANopen
    auto* regReadco  = registrco->add_subcommand("read", "Read MD register.");
    auto* regWriteco = registrco->add_subcommand("write", "Write MD register.");

    regReadco->add_option("<CAN_ID>", cmd.id, "CAN ID of the MD to interact with.")->required();
    regReadco->add_option("<REG_ID>", cmd.reg, "Register ID (offset) to read data from.")
        ->required();
    regReadco->add_option("<REG_SUB_ID>", cmd.subReg, "Register ID (offset) to read data from.")
        ->required();
    regWriteco->add_option("<CAN_ID>", cmd.id, "CAN ID of the MD to interact with.")->required();
    regWriteco->add_option("<REG_ID>", cmd.reg, "Register ID (offset) to write data to.")
        ->required();
    regWriteco
        ->add_option("<REG_SUB_ID>",
                     cmd.subReg,
                     "Register ID (offset) to read data from. Mandatory for CANopen.")
        ->required();
    regWriteco->add_option("<VALUE>", cmd.value, "Value to write")->required();

    regWriteco->add_option("<DATA_SIZE>",
                           cmd.dataSize,
                           "Size of data to be sent. {1,2,4}[bytes]. Mandatory for CANopen.");

    // RESET CANopen
    resetco->add_option("<CAN_ID>", cmd.id, "CAN ID of the MD to interact with.")->required();

    // PDOTEST CANopen
    auto* PdoSpeedco    = pdoTestco->add_subcommand("Speed", "Perform a speed loop.");
    auto* PdoPositionco = pdoTestco->add_subcommand("Position", "Perform a position loop");
    PdoSpeedco->add_option("<CAN_ID>", cmd.id, "CAN ID of the MD to interact with.")->required();
    PdoSpeedco->add_option("<SPEED>", cmd.desiredSpeed, "Desired motor speed [RPM]")->required();
    PdoPositionco->add_option("<CAN_ID>", cmd.id, "CAN ID of the MD to interact with.")->required();
    PdoPositionco->add_option("<POSITION>", cmd.desiredSpeed, "Desired motor position [inc]")
        ->required();

    // SETUP CANopen
    auto* setupCalibco    = setupco->add_subcommand("calibration", "Calibrate main MD encoder.");
    auto* setupCalibOutco = setupco->add_subcommand("calibration_out", "Calibrate output encoder.");
    auto* setupInfoco     = setupco->add_subcommand("info", "Display info about the MD drive.");
    auto* setupMotorco = setupco->add_subcommand("motor", "Upload actuator config from .cfg file.");
    auto* setupReadCfgco =
        setupco->add_subcommand("read_config", "Download actuator config from MD to .cfg file.");
    auto* setupInfoAllFlagco = setupInfoco->add_flag("-a", "Print ALL available info.");

    setupCalibco->add_option("<CAN_ID>", cmd.id, "CAN ID of the MD to interact with.")->required();
    setupCalibOutco->add_option("<CAN_ID>", cmd.id, "CAN ID of the MD to interact with.")
        ->required();
    setupInfoco->add_option("<CAN_ID>", cmd.id, "CAN ID of the MD to interact with.")->required();
    setupMotorco->add_option("<CAN_ID>", cmd.id, "CAN ID of the MD to interact with.")->required();
    setupMotorco->add_option("<.cfg_FILENAME>",
                             cmd.cfgPath,
                             "Filename of motor config. Default config files are "
                             "in:`/etc/candletool/config/motors/`.");
    setupMotorco->add_flag("-f", cmd.force, "Force uploading config file, without verification.");
    setupReadCfgco->add_option("<CAN_ID>", cmd.id, "CAN ID of the MD to interact with.")
        ->required();
    setupReadCfgco->add_option("<FILE>", cmd.value, "File to save config to.");

    // TEST CANopen
    auto* testEncoderco = testco->add_subcommand("encoder", "Perform accuracy test on encoder.");
    auto* testEncoderMainco =
        testEncoderco->add_subcommand("main", "Perfrom test routine on main encoder.");
    auto* testEncoderOutco =
        testEncoderco->add_subcommand("output", "Perfrom test routine on output encoder.");
    auto* testLatencyco = testco->add_subcommand(
        "latency",
        "Test max data exchange rate between your computer and all MD connected  drives.");
    auto* testMoveco = testco->add_subcommand("move", "Validate if motor can move.");
    auto* testMoveAbsco =
        testMoveco->add_subcommand("absolute", "Move motor to absolute position.");
    auto* testMoveRelco =
        testMoveco->add_subcommand("relative", "Move motor to relative position.");
    auto* testMoveSpeedco = testMoveco->add_subcommand("speed", "Move motor to desired speed.");
    auto* testImpedanceco = testMoveco->add_subcommand(
        "impedance",
        "Put the motor into Impedance PD mode "
        "cf:https://mabrobotics.github.io/MD80-x-CANdle-Documentation/"
        "md_x_candle_ecosystem_overview/Motion%20modes.html#impedance-pd");

    testEncoderMainco->add_option("<CAN_ID>", cmd.id, "CAN ID of the MD to interact with.")
        ->required();
    testEncoderOutco->add_option("<CAN_ID>", cmd.id, "CAN ID of the MD to interact with.")
        ->required();
    testLatencyco->add_option("<CAN_ID>", cmd.id, "CAN ID of the MD to interact with.")->required();
    testMoveAbsco->add_option("<CAN_ID>", cmd.id, "CAN ID of the MD to interact with.")->required();
    testMoveAbsco->add_option("<ABS_POS>", cmd.pos, "Absolute position to reach [rad].")
        ->required();
    testMoveAbsco->add_option("<MAX_VEL>", cmd.vel, "Profile max velocity [rad/s].");
    testMoveAbsco->add_option("<MAX_ACC>", cmd.acc, "Profile max acceleration [rad/s^2].");
    testMoveAbsco->add_option("<MAX_DCC>", cmd.dcc, "Profile max deceleration [rad/s^2].");
    testMoveAbsco->add_option(
        "<MAX_CURRENT>",
        cmd.maxCurrent,
        "Configures the maximum allowed phase current in the motor. The value is "
        "expressed in permille of rated current. Example: rated current of the motor "
        "is 15A, and the maximum is 30A. The Max Current object should equal to 2000.");
    testMoveAbsco->add_option(
        "<RATED_CURRENT>",
        cmd.ratedCurrent,
        "Configures the maximum allowed torque in the motor. The value is expressed "
        "in permille of rated torque. Example: rated torque of the motor is 1Nm, and "
        "the maximum is 2Nm. The Max Torque object should equal to 2000.");
    testMoveAbsco->add_option(
        "<MAX_TORQUE>",
        cmd.maxTorque,
        "Configures the maximum allowed phase current in the motor. The value is "
        "expressed in permille of rated current. Example: rated current of the motor "
        "is 15A, and the maximum is 30A. The Max Current object should equal to 2000.");
    testMoveAbsco->add_option(
        "<RATED_TORQUE>",
        cmd.ratedTorque,
        "Configures the motor rated torque expressed in mNm. This object is a "
        "reference for parameters such as 0x6072 Max Torque. The value should be "
        "taken from the motor's datasheet");

    testMoveRelco->add_option("<CAN_ID>", cmd.id, "CAN ID of the MD to interact with.")->required();
    testMoveRelco->add_option("<REL_POS>",
                              cmd.pos,
                              "Relative position to reach. <-10, 10>[rad] for canFD and <0x0, "
                              "0xFFFFFFFF>[inc] for CANopen");

    testMoveRelco->add_option(
        "<MAX_SPEED>",
        cmd.MaxSpeed,
        "Configures the target velocity for the profile position mode. If this value is "
        "greater than 0x6080 Max Motor Speed, it will be limited to Max Motor Speed.");
    testMoveRelco->add_option(
        "<MAX_CURRENT>",
        cmd.maxCurrent,
        "Configures the maximum allowed phase current in the motor. The value is "
        "expressed in permille of rated current. Example: rated current of the motor "
        "is 15A, and the maximum is 30A. The Max Current object should equal to 2000.");
    testMoveRelco->add_option(
        "<RATED_CURRENT>",
        cmd.ratedCurrent,
        "Configures the maximum allowed torque in the motor. The value is expressed "
        "in permille of rated torque. Example: rated torque of the motor is 1Nm, and "
        "the maximum is 2Nm. The Max Torque object should equal to 2000.");
    testMoveRelco->add_option(
        "<MAX_TORQUE>",
        cmd.maxTorque,
        "Configures the maximum allowed phase current in the motor. The value is "
        "expressed in permille of rated current. Example: rated current of the motor "
        "is 15A, and the maximum is 30A. The Max Current object should equal to 2000.");
    testMoveRelco->add_option(
        "<RATED_TORQUE>",
        cmd.ratedTorque,
        "Configures the motor rated torque expressed in mNm. This object is a "
        "reference for parameters such as 0x6072 Max Torque. The value should be "
        "taken from the motor’s datasheet");

    testMoveSpeedco->add_option("<CAN_ID>", cmd.id, "CAN ID of the MD to interact with.")
        ->required();
    testMoveSpeedco
        ->add_option(
            "<DESIRED_SPEED>", cmd.desiredSpeed, "Sets the target velocity for all motion modes.")
        ->required();
    testMoveSpeedco->add_option(
        "<MAX_CURRENT>",
        cmd.maxCurrent,
        "Configures the maximum allowed phase current in the motor. The value is "
        "expressed in permille of rated current. Example: rated current of the motor "
        "is 15A, and the maximum is 30A. The Max Current object should equal to 2000.");
    testMoveSpeedco->add_option(
        "<RATED_CURRENT>",
        cmd.ratedCurrent,
        "Configures the maximum allowed torque in the motor. The value is expressed "
        "in permille of rated torque. Example: rated torque of the motor is 1Nm, and "
        "the maximum is 2Nm. The Max Torque object should equal to 2000.");
    testMoveSpeedco->add_option(
        "<MAX_TORQUE>",
        cmd.maxTorque,
        "Configures the maximum allowed phase current in the motor. The value is "
        "expressed in permille of rated current. Example: rated current of the motor "
        "is 15A, and the maximum is 30A. The Max Current object should equal to 2000.");
    testMoveSpeedco->add_option(
        "<RATED_TORQUE>",
        cmd.ratedTorque,
        "Configures the motor rated torque expressed in mNm. This object is a "
        "reference for parameters such as 0x6072 Max Torque. The value should be "
        "taken from the motor's datasheet");
    testMoveSpeedco->add_option(
        "<MAX_SPEED>",
        cmd.MaxSpeed,
        "Configures the target velocity for the profile position mode. If this value is "
        "greater than 0x6080 Max Motor Speed, it will be limited to Max Motor Speed.");
    testImpedanceco->add_option("<CAN_ID>", cmd.id, "CAN ID of the MD to interact with.")
        ->required();
    testImpedanceco
        ->add_option(
            "<DESIRED_SPEED>", cmd.desiredSpeed, "Sets the target velocity for all motion modes.")
        ->required();
    testImpedanceco->add_option("<REL_POS>",
                                cmd.pos,
                                "Relative position to reach. <-10, 10>[rad] for canFD and <0x0, "
                                "0xFFFFFFFF>[inc] for CANopen");
    testImpedanceco->add_option("<KP>", cmd.kp, "position gain");
    testImpedanceco->add_option("<KD>", cmd.kd, "velocity gain");
    testImpedanceco->add_option("<TORQUE_FF>", cmd.torque, "Torque FF");
    testImpedanceco->add_option(
        "<MAX_SPEED>",
        cmd.MaxSpeed,
        "Configures the target velocity for the profile position mode. If this value is "
        "greater than 0x6080 Max Motor Speed, it will be limited to Max Motor Speed.");
    testImpedanceco->add_option(
        "<MAX_CURRENT>",
        cmd.maxCurrent,
        "Configures the maximum allowed phase current in the motor. The value is "
        "expressed in permille of rated current. Example: rated current of the motor "
        "is 15A, and the maximum is 30A. The Max Current object should equal to 2000.");
    testImpedanceco->add_option(
        "<RATED_CURRENT>",
        cmd.ratedCurrent,
        "Configures the maximum allowed torque in the motor. The value is expressed "
        "in permille of rated torque. Example: rated torque of the motor is 1Nm, and "
        "the maximum is 2Nm. The Max Torque object should equal to 2000.");
    testImpedanceco->add_option(
        "<MAX_TORQUE>",
        cmd.maxTorque,
        "Configures the maximum allowed phase current in the motor. The value is "
        "expressed in permille of rated current. Example: rated current of the motor "
        "is 15A, and the maximum is 30A. The Max Current object should equal to 2000.");
    testImpedanceco->add_option(
        "<RATED_TORQUE>",
        cmd.ratedTorque,
        "Configures the motor rated torque expressed in mNm. This object is a "
        "reference for parameters such as 0x6072 Max Torque. The value should be "
        "taken from the motor's datasheet");
    // Heartbeat
    heartbeatco
        ->add_option(
            "<CAN_MASTER_ID>", cmd.id, "CAN ID of the master who will produce the heartbeat")
        ->required();
    heartbeatco
        ->add_option("<CAN_SLAVE_ID>",
                     cmd.newId,
                     "CAN ID of the slave who will follow the heartbeat (A MD must be attach)")
        ->required();
    heartbeatco->add_option("-t,--timeout",
                            cmd.HeartbeatTimeout,
                            "duration after which the engine goes into preoperational state if it "
                            "has not received a heartbeat from the master node [ms] ");
    // SDOsegmented CANopen
    auto* SDOsegmentedReadco = SDOsegmentedco->add_subcommand(
        "read", "Read a value from a register with more than 4 bytes of data.");
    auto* SDOsegmentedWriteco = SDOsegmentedco->add_subcommand(
        "write", "write a value in a register with more than 4 bytes of data.");
    SDOsegmentedReadco->add_option("<CAN_ID>", cmd.id, "CAN ID of the MD to interact with.")
        ->required();
    SDOsegmentedReadco->add_option("<REG_ID>", cmd.reg, "Register ID (offset) to read data from.")
        ->required();
    SDOsegmentedReadco
        ->add_option("<REG_SUB_ID>", cmd.subReg, "Register ID (offset) to read data from.")
        ->required();
    SDOsegmentedWriteco->add_option("<CAN_ID>", cmd.id, "CAN ID of the MD to interact with.")
        ->required();
    SDOsegmentedWriteco->add_option("<REG_ID>", cmd.reg, "Register ID (offset) to read data from.")
        ->required();
    SDOsegmentedWriteco
        ->add_option("<REG_SUB_ID>", cmd.subReg, "Register ID (offset) to read data from.")
        ->required();
    SDOsegmentedWriteco->add_option("<DATA>", cmd.value, "value to write into the register")
        ->required();

    // PING
    ping->add_option("<CAN_BAUD>", cmd.variant, "Can be one of: 1M, 2M, 5M, 8M, all. Default: all");

    // REGISTER
    auto* regRead  = registr->add_subcommand("read", "Read MD register.");
    auto* regWrite = registr->add_subcommand("write", "Write MD register.");

    regRead->add_option("<CAN_ID>", cmd.id, "CAN ID of the MD to interact with.")->required();
    regRead->add_option("<REG_ID>", cmd.reg, "Register ID (offset) to read data from.")->required();
    regRead->add_option("<REG_SUB_ID>", cmd.subReg, "Register ID (offset) to read data from.")
        ->required();
    regWrite->add_option("<CAN_ID>", cmd.id, "CAN ID of the MD to interact with.")->required();
    regWrite->add_option("<REG_ID>", cmd.reg, "Register ID (offset) to write data to.")->required();
    regWrite
        ->add_option("<REG_SUB_ID>",
                     cmd.subReg,
                     "Register ID (offset) to read data from. Mandatory for CANopen.")
        ->required();
    regWrite->add_option("<VALUE>", cmd.value, "Value to write")->required();

    regWrite->add_option("<DATA_SIZE>",
                         cmd.dataSize,
                         "Size of data to be sent. {1,2,4}[bytes]. Mandatory for CANopen.");

    // RESET
    reset->add_option("<CAN_ID>", cmd.id, "CAN ID of the MD to interact with.")->required();

    // SETUP
    auto* setupCalib    = setup->add_subcommand("calibration", "Calibrate main MD encoder.");
    auto* setupCalibOut = setup->add_subcommand("calibration_out", "Calibrate output encoder.");
    auto* setupHoming   = setup->add_subcommand("homing", "Begin homing procedure.");
    auto* setupInfo     = setup->add_subcommand("info", "Display info about the MD drive.");
    auto* setupMotor    = setup->add_subcommand("motor", "Upload actuator config from .cfg file.");
    auto* setupReadCfg =
        setup->add_subcommand("read_config", "Download actuator config from MD to .cfg file.");
    auto* setupInfoAllFlag = setupInfo->add_flag("-a", "Print ALL available info.");

    setupCalib->add_option("<CAN_ID>", cmd.id, "CAN ID of the MD to interact with.")->required();
    setupCalibOut->add_option("<CAN_ID>", cmd.id, "CAN ID of the MD to interact with.")->required();
    setupHoming->add_option("<CAN_ID>", cmd.id, "CAN ID of the MD to interact with.")->required();
    setupInfo->add_option("<CAN_ID>", cmd.id, "CAN ID of the MD to interact with.")->required();
    setupMotor->add_option("<CAN_ID>", cmd.id, "CAN ID of the MD to interact with.")->required();
    setupMotor->add_option("<.cfg_FILENAME>",
                           cmd.cfgPath,
                           "Filename of motor config. Default config files are "
                           "in:`/etc/candletool/config/motors/`.");
    setupMotor->add_flag("-f", cmd.force, "Force uploading config file, without verification.");
    setupReadCfg->add_option("<CAN_ID>", cmd.id, "CAN ID of the MD to interact with.")->required();
    setupReadCfg->add_option("<FILE>", cmd.value, "File to save config to.");

    // TEST
    auto* testEncoder = test->add_subcommand("encoder", "Perform accuracy test on encoder.");
    auto* testEncoderMain =
        testEncoder->add_subcommand("main", "Perform test routine on main encoder.");
    auto* testEncoderOut =
        testEncoder->add_subcommand("output", "Perform test routine on output encoder.");
    auto* testLatency = test->add_subcommand(
        "latency",
        "Test max data exchange rate between your computer and all MD connected  drives.");
    auto* testMove    = test->add_subcommand("move", "Validate if motor can move.");
    auto* testMoveAbs = testMove->add_subcommand("absolute", "Move motor to absolute position.");
    auto* testMoveRel = testMove->add_subcommand("relative", "Move motor to relative position.");

    testEncoderMain->add_option("<CAN_ID>", cmd.id, "CAN ID of the MD to interact with.")
        ->required();
    testEncoderOut->add_option("<CAN_ID>", cmd.id, "CAN ID of the MD to interact with.")
        ->required();
    testLatency->add_option("<CAN_ID>", cmd.id, "CAN ID of the MD to interact with.")->required();
    testMoveAbs->add_option("<CAN_ID>", cmd.id, "CAN ID of the MD to interact with.")->required();
    testMoveAbs->add_option("<ABS_POS>", cmd.pos, "Absolute position to reach [rad].")->required();
    testMoveAbs->add_option("<MAX_VEL>", cmd.vel, "Profile max velocity [rad/s].");
    testMoveAbs->add_option("<MAX_ACC>", cmd.acc, "Profile max acceleration [rad/s^2].");
    testMoveAbs->add_option("<MAX_DCC>", cmd.dcc, "Profile max deceleration [rad/s^2].");
    testMoveRel->add_option("<CAN_ID>", cmd.id, "CAN ID of the MD to interact with.")->required();
    testMoveRel->add_option("<REL_POS>", cmd.pos, "Relative position to reach. <-10, 10>[rad]");

    // Update
    auto* candleUpdate = update->add_subcommand("candle", "Update firmware on Candle device.");
    auto* mdUpdate     = update->add_subcommand("md", "Update firmware on MD device.");
    auto* pdsUpdate    = update->add_subcommand("pds", "Update firmware on PDS device.");

    mdUpdate->add_option("<CAN_ID>", cmd.id, "CAN ID of the MD to interact with.")->required();
    pdsUpdate->add_option("<CAN_ID>", cmd.id, "CAN ID of the PDS to interact with.")->required();
    update->add_flag("-r", cmd.noReset, "Do not reset the device before updating firmware.");
    update->add_option("-f,--file", cmd.firmwareFileName, "Path to the .mab file");

    PdsCli pdsCli(app);

    CLI11_PARSE(app, argc, argv);

    std::optional<mab::CANdleBaudrate_E> baudOpt = stringToBaudrate(cmd.baud);
    if (!baudOpt.has_value())
    {
        std::cerr << "Invalid baudrate: " << cmd.baud << std::endl;
        return EXIT_FAILURE;
    }

    mab::CANdleBaudrate_E baud = baudOpt.value_or(CANdleBaudrate_E::CAN_BAUD_1M);

    mINI::INIFile      file(getCandletoolConfigPath());
    mINI::INIStructure ini;
    file.read(ini);

    std::string busString = ini["communication"]["bus"];

    // std::shared_ptr<mab::Candle> candle = nullptr;

    CLI11_PARSE(app, argc, argv);

    // TODO: make use of busType and baudrate options when creating Candle object within CandleTool
    // CandleTool candleTool(baud);
    // Pds        pds(cmd.id, candleTool.getCandle());

    // set global verbosity for loggers
    if (silentMode)
        Logger::g_m_verbosity = Logger::Verbosity_E::SILENT;
    else if (verbosityMode < static_cast<uint32_t>(Logger::Verbosity_E::SILENT))
        Logger::g_m_verbosity = static_cast<Logger::Verbosity_E>(verbosityMode);
    else
        throw std::runtime_error("Verbosity outside of range");

    // redirect logger if asked for
    if (logPath != "")
    {
        if (!Logger::setStream(logPath.c_str()))
            throw std::runtime_error("Could not create log file!");
    }

    if (mdco->parsed())
    {
        CandleToolCO candleToolCO(baud);
        Pds          pds(cmd.id, candleToolCO.getCandle());
        if (blinkco->parsed())
            candleToolCO.blink(cmd.id);
        if (clearco->parsed())
            candleToolCO.clearErrors(cmd.id, cmd.variant);
        if (configco->parsed())
        {
            if (configBandco->parsed())
                candleToolCO.configBandwidth(cmd.id, cmd.bandwidth);
            if (configCanco->parsed())
                candleToolCO.configCan(cmd.id, cmd.newId, cmd.baud, cmd.canWatchdog);
            if (configSaveco->parsed())
                candleToolCO.configSave(cmd.id);
            if (configZeroco->parsed())
                candleToolCO.configZero(cmd.id);
        }
        if (encoderco->parsed())
            candleToolCO.encoder(cmd.id);
        if (pingco->parsed())
            candleToolCO.ping(cmd.variant);
        if (registrco->parsed())
        {
            u16 reg    = strtoul(cmd.reg.c_str(), nullptr, 16);
            u8  subreg = (u8)cmd.subReg;

            if (regReadco->parsed())
                candleToolCO.registerRead(cmd.id, reg, subreg);
            if (regWriteco->parsed())
                candleToolCO.registerWrite(cmd.id, reg, cmd.value, subreg, cmd.dataSize);
        }
        if (resetco->parsed())
            candleToolCO.reset(cmd.id);
        if (pdoTestco->parsed())
        {
            if (PdoSpeedco->parsed())
                candleToolCO.sendPdoSpeed(cmd.id, cmd.desiredSpeed);
            if (PdoPositionco->parsed())
                candleToolCO.sendPdoPosition(cmd.id, cmd.desiredSpeed);
        }
        if (setupco->parsed())
        {
            if (setupCalibco->parsed())
                candleToolCO.setupCalibration(cmd.id);
            if (setupCalibOutco->parsed())
                candleToolCO.setupCalibrationOutput(cmd.id);
            if (setupInfoco->parsed())
                candleToolCO.setupInfo(cmd.id, (setupInfoAllFlagco->count() > 0 ? true : false));
            if (setupMotorco->parsed())
                candleToolCO.setupMotor(cmd.id, cmd.cfgPath, cmd.force);
            if (setupReadCfgco->parsed())
                candleToolCO.setupReadConfig(cmd.id, cmd.value);
        }
        if (testco->parsed())
        {
            if (testEncoderMainco->parsed())
                candleToolCO.testEncoderMain(cmd.id);
            if (testEncoderOutco->parsed())
                candleToolCO.testEncoderOutput(cmd.id);
            if (testLatencyco->parsed())
                candleToolCO.testLatency(cmd.id);
            if (testMoveAbsco->parsed())
            {
                // candleToolCO.testMoveAbsolute(cmd.id, cmd.pos, cmd.vel, cmd.acc, cmd.dcc);
            }
            if (testMoveRelco->parsed())
                candleToolCO.testMove(cmd.id,
                                      cmd.pos,
                                      cmd.MaxSpeed,
                                      cmd.maxCurrent,
                                      cmd.ratedCurrent,
                                      cmd.maxTorque,
                                      cmd.ratedTorque);

            if (testMoveSpeedco->parsed())
                candleToolCO.testMoveSpeed(cmd.id,
                                           cmd.maxCurrent,
                                           cmd.ratedCurrent,
                                           cmd.maxTorque,
                                           cmd.ratedTorque,
                                           cmd.MaxSpeed,
                                           cmd.desiredSpeed);
            if (testImpedanceco->parsed())
                candleToolCO.testMoveImpedance(cmd.id,
                                               cmd.desiredSpeed,
                                               cmd.pos,
                                               cmd.kp,
                                               cmd.kd,
                                               cmd.torque,
                                               cmd.MaxSpeed,
                                               cmd.maxCurrent,
                                               cmd.ratedCurrent,
                                               cmd.maxTorque,
                                               cmd.ratedTorque);
        }

        if (heartbeatco->parsed())
        {
            candleToolCO.heartbeatTest(cmd.id, cmd.newId, cmd.HeartbeatTimeout);
        }
        if (SDOsegmentedco->parsed())
        {
            if (SDOsegmentedReadco->parsed())
            {
                u16 reg = strtoul(cmd.reg.c_str(), nullptr, 16);
                candleToolCO.SDOsegmentedRead(cmd.id, reg, cmd.subReg);
            }
            if (SDOsegmentedWriteco->parsed())
            {
                u16 reg = strtoul(cmd.reg.c_str(), nullptr, 16);
                candleToolCO.SDOsegmentedWrite(cmd.id, reg, cmd.subReg, cmd.value);
            }
        }
        pdsCli.parse(&pds);
    }

    else
    {
        CandleTool candleTool(baud);
        Pds        pds(cmd.id, candleTool.getCandle());
        if (blink->parsed())
            candleTool.blink(cmd.id);
        if (bus->parsed())
            candleTool.bus(cmd.bus, cmd.variant);
        if (clear->parsed())
            candleTool.clearErrors(cmd.id, cmd.variant);
        if (config->parsed())
        {
            if (configBand->parsed())
                candleTool.configBandwidth(cmd.id, cmd.bandwidth);
            if (configCan->parsed())
                candleTool.configCan(cmd.id, cmd.newId, cmd.baud, cmd.canWatchdog);
            if (configClear->parsed())
                candleTool.configClear(cmd.id);
            if (configCurrent->parsed())
                candleTool.configCurrent(cmd.id, cmd.current);
            if (configSave->parsed())
                candleTool.configSave(cmd.id);
            if (configZero->parsed())
                candleTool.configZero(cmd.id);
        }
        if (encoder->parsed())
            candleTool.encoder(cmd.id);
        if (ping->parsed())
            candleTool.ping(cmd.variant);
        if (registr->parsed())
        {
            u16 reg    = strtoul(cmd.reg.c_str(), nullptr, 16);
            u8  subreg = (u8)cmd.subReg;

            if (regRead->parsed())
                candleTool.registerRead(cmd.id, reg, subreg);
            if (regWrite->parsed())
                candleTool.registerWrite(cmd.id, reg, cmd.value, subreg, cmd.dataSize);
        }
        if (reset->parsed())
            candleTool.reset(cmd.id);

        if (setup->parsed())
        {
            if (setupCalib->parsed())
                candleTool.setupCalibration(cmd.id);
            if (setupCalibOut->parsed())
                candleTool.setupCalibrationOutput(cmd.id);
            if (setupInfo->parsed())
                candleTool.setupInfo(cmd.id, (setupInfoAllFlag->count() > 0 ? true : false));
            if (setupMotor->parsed())
                candleTool.setupMotor(cmd.id, cmd.cfgPath, cmd.force);
            if (setupReadCfg->parsed())
                candleTool.setupReadConfig(cmd.id, cmd.value);
        }
        if (test->parsed())
        {
            if (testEncoderMain->parsed())
                candleTool.testEncoderMain(cmd.id);
            if (testEncoderOut->parsed())
                candleTool.testEncoderOutput(cmd.id);
            if (testLatency->parsed())
                candleTool.testLatency(cmd.id);
            if (testMoveAbs->parsed())
            {
                candleTool.testMoveAbsolute(cmd.id, cmd.pos, cmd.vel, cmd.acc, cmd.dcc);
            }
            if (testMoveRel->parsed())
                candleTool.testMove(cmd.id, cmd.pos);
        }

        if (update->parsed())
        {
            if (cmd.firmwareFileName == "no_file")
            {
                std::cout << "No filename given!" << std::endl;
                // Place holder for future feature :: Automatically detect hw version and
                // download firmware from our release pages...
                std::cout << "Fetching most recent firmware online [ Not implemented yet ... ]"
                          << std::endl;
            }
            else
            {
                // std::cout << "using mab file [ " << cmd.firmwareFileName << " ]" <<
                // std::endl;
            }

            if (candleUpdate->parsed())
            {
                candleTool.updateCandle(cmd.firmwareFileName);
                return EXIT_SUCCESS;
            }

            if (mdUpdate->parsed())
            {
                candleTool.updateMd(cmd.firmwareFileName, cmd.id, cmd.noReset);
                return EXIT_SUCCESS;
            }

            if (pdsUpdate->parsed())
            {
                candleTool.updatePds(pds, cmd.firmwareFileName, cmd.id, cmd.noReset);
                return EXIT_SUCCESS;
            }
        }

        pdsCli.parse(&pds);
    }

    return EXIT_SUCCESS;
}
