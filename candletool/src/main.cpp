#include "candle_types.hpp"
#include "candletool.hpp"
#include "configHelpers.hpp"
#include "logger.hpp"
#include "ui.hpp"
#include "CLI/CLI.hpp"
#include "pds_cli.hpp"

int main(int argc, char** argv)
{
    // Logger::g_m_verbosity = Logger::Verbosity_E::VERBOSITY_3;
    std::cout << "CandleTool" << std::endl;
    CLI::App app{"candletool"};
    app.fallthrough();
    app.ignore_case();
    UserCommand cmd;

    auto* blink   = app.add_subcommand("blink", "Blink LEDs on MD drive.");
    auto* bus     = app.add_subcommand("bus", "Set CANdle bus parameters.");
    auto* clear   = app.add_subcommand("clear", "Clear Errors and Warnings.");
    auto* config  = app.add_subcommand("config", "Configure CAN parameters of MD drive.");
    auto* encoder = app.add_subcommand("encoder", "Display MD rotor position.");
    auto* ping    = app.add_subcommand("ping", "Discover MD drives on CAN bus.");
    auto* registr = app.add_subcommand("register", "Access MD drive via register read/write.");
    auto* reset   = app.add_subcommand("reset", "Reset MD drive.");
    auto* setup   = app.add_subcommand("setup", "Setup MD via config files, and calibrate.");
    auto* test    = app.add_subcommand("test", "Basic MD drive testing routines.");
    auto* update  = app.add_subcommand("update", "Firmware update.");

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

    // PING
    ping->add_option("<CAN_BAUD>", cmd.variant, "Can be one of: 1M, 2M, 5M, 8M, all. Default: all");

    // REGISTER
    auto* regRead  = registr->add_subcommand("read", "Read MD register.");
    auto* regWrite = registr->add_subcommand("write", "Write MD register.");

    regRead->add_option("<CAN_ID>", cmd.id, "CAN ID of the MD to interact with.")->required();
    regRead->add_option("<REG_ID>", cmd.reg, "Register ID (offset) to read data from.")->required();
    regWrite->add_option("<CAN_ID>", cmd.id, "CAN ID of the MD to interact with.")->required();
    regWrite->add_option("<REG_ID>", cmd.reg, "Register ID (offset) to write data to.")->required();
    regWrite->add_option("<VALUE>", cmd.value, "Value to write")->required();

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
    setupMotor->add_option(
        "<.cfg_FILENAME>",
        cmd.cfgPath,
        "Filename of motor config. Default config files are in:`/etc/candletool/config/motors/`.");
    setupMotor->add_flag("-f", cmd.force, "Force uploading config file, without verification.");
    setupReadCfg->add_option("<CAN_ID>", cmd.id, "CAN ID of the MD to interact with.")->required();
    setupReadCfg->add_option("<FILE>", cmd.value, "File to save config to.");

    // TEST
    auto* testEncoder = test->add_subcommand("encoder", "Perform accuracy test on encoder.");
    auto* testEncoderMain =
        testEncoder->add_subcommand("main", "Perfrom test routine on main encoder.");
    auto* testEncoderOut =
        testEncoder->add_subcommand("output", "Perfrom test routine on output encoder.");
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
    testLatency->add_option("<CAN_BAUD>", cmd.baud, "New CAN baudrate to set: 1M, 2M, 5M or 8M")
        ->required();
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

    // Verbosity
    uint32_t verbosityMode = 0;
    bool     silentMode{false};
    app.add_flag("-v{1},--verbosity{1}", verbosityMode, "Verbose modes (1,2,3)")
        ->default_val(0)
        ->expected(0, 1);

    app.add_flag("-s,--silent", silentMode, "Silent mode")->default_val(0);

    std::string logPath = "";
    app.add_flag("--log", logPath, "Redirect output to file")->default_val("")->expected(1);

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
    CandleTool candleTool(baud);
    Pds        pds(cmd.id, candleTool.getCandle());

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

    if (app.count_all() == 1)
        std::cerr << app.help() << std::endl;

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
        u16 reg = strtoul(cmd.reg.c_str(), nullptr, 16);
        if (regRead->parsed())
            candleTool.registerRead(cmd.id, reg);
        if (regWrite->parsed())
            candleTool.registerWrite(cmd.id, reg, cmd.value);
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
            candleTool.testLatency(cmd.baud, busString);
        if (testMoveAbs->parsed())
            candleTool.testMoveAbsolute(cmd.id, cmd.pos, cmd.vel, cmd.acc, cmd.dcc);
        if (testMoveRel->parsed())
            candleTool.testMove(cmd.id, cmd.pos);
    }

    if (update->parsed())
    {
        if (cmd.firmwareFileName == "no_file")
        {
            std::cout << "No filename given!" << std::endl;
            // Place holder for future feature :: Automatically detect hw version and download
            // firmware from our release pages...
            std::cout << "Fetching most recent firmware online [ Not implemented yet ... ]"
                      << std::endl;
        }
        else
        {
            // std::cout << "using mab file [ " << cmd.firmwareFileName << " ]" << std::endl;
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

    return EXIT_SUCCESS;
}
