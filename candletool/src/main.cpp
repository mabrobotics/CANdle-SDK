#include <memory>
#include "candle.hpp"
#include "candle_types.hpp"
#include "candletool.hpp"
#include "configHelpers.hpp"
#include "logger.hpp"
#include "mab_types.hpp"
#include "md_cli.hpp"
#include "ui.hpp"
#include "CLI/CLI.hpp"
#include "pds_cli.hpp"

//     ___     _     _  _      _   _         _____               _
//    / __|   /_\   | \| |  __| | | |  ___  |_   _|  ___   ___  | |
//   | (__   / _ \  | .` | / _` | | | / -_)   | |   / _ \ / _ \ | |
//    \___| /_/ \_\ |_|\_| \__,_| |_| \___|   |_|   \___/ \___/ |_|
//

int main(int argc, char** argv)
{
    // Logger::g_m_verbosity = Logger::Verbosity_E::VERBOSITY_3;

    std::cout << "   ___     _     _  _      _   _         _____               _ \n";
    std::cout << "  / __|   /_\\   | \\| |  __| | | |  ___  |_   _|  ___   ___  | |\n";
    std::cout << " | (__   / _ \\  | .` | / _` | | | / -_)   | |   / _ \\ / _ \\ | |\n";
    std::cout << "  \\___| /_/ \\_\\ |_|\\_| \\__,_| |_| \\___|   |_|   \\___/ \\___/ |_|\n";
    std::cout << "                                                               \n";

    CLI::App app{};
    app.fallthrough();
    app.ignore_case();
    UserCommand cmd;

    app.add_option("-d,--datarate",
                   cmd.baud,
                   "Select FD CAN Datarate CANdleTOOL will use for communication.")
        ->default_val("1M")
        ->check(CLI::IsMember({"1M", "2M", "5M", "8M"}))
        ->expected(1);
    app.add_option("--bus", cmd.bus, "Select bus to use (only for CandleHAT).")
        ->check(CLI::IsMember({"USB", "SPI"}))
        ->default_val("USB");
    app.add_option("--device",
                   cmd.variant,
                   "For SPI: {path to kernel device endpoint} | For USB: {device serial number}");

    // Update
    auto* update       = app.add_subcommand("update", "Firmware update.");
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

    auto candleBuilder = std::make_shared<CandleBuilder>();
    auto busType       = std::make_shared<candleTypes::busTypes_t>(candleTypes::busTypes_t::USB);
    auto datarate      = std::make_shared<CANdleBaudrate_E>(CANdleBaudrate_E::CAN_BAUD_1M);

    candleBuilder->busType  = busType;
    candleBuilder->datarate = datarate;

    auto preBuildTask = [busType, datarate, &cmd]()
    {
        Logger log(Logger::ProgramLayer_E::TOP, "CANDLE_PREBUILD");
        log.debug("Running candle pre-build CLI parsing task...");
        // Parsing bus type
        if (cmd.bus.find("USB" != 0))
        {
            *busType = candleTypes::busTypes_t::USB;
        }
        else if (cmd.bus.find("SPI" != 0))
        {
            *busType = candleTypes::busTypes_t::SPI;
        }
        else
        {
            log.error("Specified bus is not valid!");
        }

        // Parsing baudrate
        auto parsedBaudOpt = CandleTool::stringToBaud(cmd.baud);
        if (parsedBaudOpt.has_value())
            *datarate = parsedBaudOpt.value();
        else
            log.error("Parsing of the datarate failed!");
    };

    // This is to keep the compatibility of std::string argument as a parsed value between instances
    // of parsers
    candleBuilder->preBuildTask = preBuildTask;

    MDCli  mdCli(&app, candleBuilder);
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

    // CLI11_PARSE(app, argc, argv);

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
        std::cerr << app.help(
                         "For more information please refer to the manual: "
                         "\033[32mhttps://mabrobotics.pl/servos/manual\033[0m \n\n")
                  << std::endl;

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
