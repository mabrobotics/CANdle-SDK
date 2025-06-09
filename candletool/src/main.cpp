#include <cstdlib>
#include <memory>
#include "candle.hpp"
#include "candle_bootloader.hpp"
#include "candle_cli.hpp"
#include "candle_types.hpp"
#include "candletool.hpp"
#include "configHelpers.hpp"
#include "logger.hpp"
#include "mabFileParser.hpp"
#include "mab_crc.hpp"
#include "mab_types.hpp"
#include "md_cli.hpp"
#include "ui.hpp"
#include "CLI/CLI.hpp"
#include "pds_cli.hpp"

#include "utilities.hpp"

//     ___     _     _  _      _   _         _____               _
//    / __|   /_\   | \| |  __| | | |  ___  |_   _|  ___   ___  | |
//   | (__   / _ \  | .` | / _` | | | / -_)   | |   / _ \ / _ \ | |
//    \___| /_/ \_\ |_|\_| \__,_| |_| \___|   |_|   \___/ \___/ |_|
//

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
    bool     showCandleSDKVersion = false;
    app.add_flag("-v{1},--verbosity{1}", verbosityMode, "Verbose modes (1,2,3)")
        ->default_val(0)
        ->expected(0, 1);

    app.add_flag("--version", showCandleSDKVersion, "Show software version");

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

    CandleCli candleCli(&app, candleBuilder);
    MDCli     mdCli(&app, candleBuilder);
    PdsCli    pdsCli(app, candleBuilder);

    CLI11_PARSE(app, argc, argv);
    if (showCandleSDKVersion)
    {
        std::cout << "CandleSDK version: " << CANDLESDK_VERSION << "\n";
    }

    std::optional<mab::CANdleBaudrate_E> baudOpt = stringToBaudrate(cmd.baud);
    if (!baudOpt.has_value())
    {
        std::cerr << "Invalid baudrate: " << cmd.baud << std::endl;
        return EXIT_FAILURE;
    }

    mINI::INIFile      file(getCandletoolConfigPath());
    mINI::INIStructure ini;
    file.read(ini);

    std::string busString = ini["communication"]["bus"];

    // std::shared_ptr<mab::Candle> candle = nullptr;

    // CLI11_PARSE(app, argc, argv);

    // TODO: make use of busType and baudrate options when creating Candle object within CandleTool
    // Pds pds(cmd.id, candleTool.getCandle());

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
            MabFileParser candleFirmware(cmd.firmwareFileName,
                                         MabFileParser::TargetDevice_E::CANDLE);

            auto candle_bootloader = attachCandleBootloader();
            for (size_t i = 0; i < candleFirmware.m_fwEntry.size;
                 i += CandleBootloader::PAGE_SIZE_STM32G474)
            {
                std::array<u8, CandleBootloader::PAGE_SIZE_STM32G474> page;
                std::memcpy(page.data(), &candleFirmware.m_fwEntry.data->data()[i], page.size());
                u32 crc = crc32(page.data(), page.size());
                if (candle_bootloader->writePage(page, crc) != candleTypes::Error_t::OK)
                {
                    return EXIT_FAILURE;
                    break;
                }
            }
            return EXIT_SUCCESS;
        }

        if (mdUpdate->parsed())
        {
            // candleTool.updateMd(cmd.firmwareFileName, cmd.id, cmd.noReset);
            // return EXIT_SUCCESS;
        }

        if (pdsUpdate->parsed())
        {
            std::cout << "Implementation needed!\n";
            // candleTool.updatePds(pds, cmd.firmwareFileName, cmd.id, cmd.noReset); TODO
            return EXIT_SUCCESS;
        }
    }

    pdsCli.parse();

    return EXIT_SUCCESS;
}
