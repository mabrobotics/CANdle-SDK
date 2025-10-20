#include <cstdlib>
#include <memory>
#include <string>
#include "candle.hpp"
#include "candle_bootloader.hpp"
#include "candle_cli.hpp"
#include "candle_types.hpp"
#include "configHelpers.hpp"
#include "logger.hpp"
#include "mabFileParser.hpp"
#include "mab_crc.hpp"
#include "mab_types.hpp"
#include "md_cli.hpp"
#include "CLI/CLI.hpp"
#include "pds_cli.hpp"
#include "mdco_cli.hpp"

#include "utilities.hpp"

//     ___     _     _  _      _   _         _____               _
//    / __|   /_\   | \| |  __| | | |  ___  |_   _|  ___   ___  | |
//   | (__   / _ \  | .` | / _` | | | / -_)   | |   / _ \ / _ \ | |
//    \___| /_/ \_\ |_|\_| \__,_| |_| \___|   |_|   \___/ \___/ |_|
//

struct UserCommand
{
    std::string data;
    std::string bus;
    std::string variant;
};

int main(int argc, char** argv)
{
    // Logger::g_m_verbosity = Logger::Verbosity_E::VERBOSITY_3;

    auto mabDescriptionFormatter = std::make_shared<MABDescriptionFormatter>();
    mabDescriptionFormatter->enable_description_formatting(false);
    CLI::App app{};
    app.fallthrough();
    app.formatter(mabDescriptionFormatter);
    app.ignore_case();
    UserCommand cmd;

    app.add_option("-d,--datarate",
                   cmd.data,
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

    // Verbosity
    uint32_t verbosityMode = 0;
    bool     silentMode{false};
    bool     showCandleSDKVersion = false;
    app.add_flag("-v{1},--verbosity{1}", verbosityMode, "Verbose modes (1,2,3)")
        ->expected(0, 1)
        ->check(
            [](const std::string& input) -> std::string
            {
                Logger::g_m_verbosity = Logger::Verbosity_E(std::stoi(input));
                return std::string();
            });

    app.add_flag("--version", showCandleSDKVersion, "Show software version");

    app.add_flag("-s,--silent", silentMode, "Silent mode")
        ->check(
            [](const std::string& input) -> std::string
            {
                Logger::g_m_verbosity = Logger::Verbosity_E::SILENT;
                return std::string();
            });

    std::string logPath = "";
    app.add_flag("--log", logPath, "Redirect output to file")->default_val("")->expected(1);

    auto candleBuilder = std::make_shared<CandleBuilder>();
    auto busType       = std::make_shared<candleTypes::busTypes_t>(candleTypes::busTypes_t::USB);
    auto datarate      = std::make_shared<CANdleDatarate_E>(CANdleDatarate_E::CAN_DATARATE_1M);

    candleBuilder->busType  = busType;
    candleBuilder->datarate = datarate;

    CANdleBranch_S canBranch;
    canBranch.candleBuilder = candleBuilder;

    CANdleToolCtx_S candleToolCtx;
    candleToolCtx.candleBranchVec.push_back(canBranch);

    auto preBuildTask = [busType, datarate, &cmd]()
    {
        Logger log(Logger::ProgramLayer_E::TOP, "CANDLE_PREBUILD");
        log.debug("Running candle pre-build CLI parsing task...");
        // Parsing bus type
        if (cmd.bus.find("USB") != std::string::npos)
        {
            log.debug("Using USB bus");
            *busType = candleTypes::busTypes_t::USB;
        }
        else if (cmd.bus.find("SPI") != std::string::npos)
        {
            log.debug("Using SPI bus");
            *busType = candleTypes::busTypes_t::SPI;
        }
        else
        {
            log.error("Specified bus is not valid!");
        }

        // Parsing datarate
        auto parsedDataOpt = stringToData(cmd.data);
        if (parsedDataOpt.has_value())
            *datarate = parsedDataOpt.value();
        else
            log.error("Parsing of the datarate failed!");
    };

    // This is to keep the compatibility of std::string argument as a parsed value between instances
    // of parsers
    candleBuilder->preBuildTask = preBuildTask;

    CandleCli candleCli(&app, candleToolCtx);
    MDCli     mdCli(&app, candleToolCtx);
    PdsCli    pdsCli(app, candleBuilder);
    MdcoCli   MdcoCli(app, candleBuilder);

    CLI11_PARSE(app, argc, argv);
    if (showCandleSDKVersion)
    {
        std::cout << "CandleSDK version: " << CANDLESDK_VERSION << "\n";
    }

    std::optional<mab::CANdleDatarate_E> dataOpt = stringToData(cmd.data);
    if (!dataOpt.has_value())
    {
        std::cerr << "Invalid datarate: " << cmd.data << std::endl;
        return EXIT_FAILURE;
    }

    mINI::INIFile      file(getCandletoolConfigPath());
    mINI::INIStructure ini;
    file.read(ini);

    std::string busString = ini["communication"]["bus"];

    if (logPath != "")
    {
        if (!Logger::setStream(logPath.c_str()))
            throw std::runtime_error("Could not create log file!");
    }

    if (app.count_all() == 1)
        std::cerr << app.help() << std::endl;

    pdsCli.parse();
    return EXIT_SUCCESS;
}
