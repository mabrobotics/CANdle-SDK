#pragma once
#include "CLI/CLI.hpp"
#include "candle.hpp"
#include "mab_types.hpp"
#include "logger.hpp"
#include "MD.hpp"
#include <algorithm>
#include <memory>
#include <candle_types.hpp>

namespace mab
{

    class MDCli
    {
      public:
        MDCli() = delete;
        MDCli(CLI::App* rootCli, const std::shared_ptr<const CandleBuilder> candleBuilder);
        ~MDCli() = default;

      private:
        Logger m_logger = Logger(Logger::ProgramLayer_E::TOP, "MD_CLI");
        std::unique_ptr<MD, std::function<void(MD*)>> getMd(
            const std::shared_ptr<canId_t>             mdCanId,
            const std::shared_ptr<const CandleBuilder> candleBuilder);

        struct CanOptions
        {
            CanOptions(CLI::App* rootCli)
                : canId(std::make_shared<canId_t>(100)),
                  datarate(std::make_shared<std::string>("1M")),
                  timeoutMs(std::make_shared<uint16_t>(200)),
                  save(std::make_shared<bool>(false))
            {
                optionsMap = std::map<std::string, CLI::Option*>{
                    {"id",
                     rootCli->add_option(
                         "-i,--id", *canId, "New CAN node id for the MD controller.")},
                    {"datarate",
                     rootCli->add_option(
                         "-d,--datarate", *datarate, "New datarate of the MD controller.")},
                    {"timeout",
                     rootCli->add_option(
                         "-t,--timeout", *timeoutMs, "New timeout of the MD controller.")},
                    {"save",
                     rootCli->add_flag(
                         "-s,--save", *save, "Save the new CAN parameters to the MD controller.")}};
            }
            const std::shared_ptr<canId_t>     canId;
            const std::shared_ptr<std::string> datarate;
            const std::shared_ptr<uint16_t>    timeoutMs;
            const std::shared_ptr<bool>        save;

            std::map<std::string, CLI::Option*> optionsMap;
        };
        struct CalibrationOptions
        {
            CalibrationOptions(CLI::App* rootCli)
                : calibrationOfEncoder(std::make_shared<std::string>("all"))
            {
                optionsMap = std::map<std::string, CLI::Option*>{

                    {"encoder",
                     rootCli
                         ->add_option("-e,--encoder",
                                      *calibrationOfEncoder,
                                      "Type of encoder calibration to perform. "
                                      "Possible values: all, main, aux.")
                         ->default_str("all")},
                };
            }

            const std::shared_ptr<std::string>  calibrationOfEncoder;
            std::map<std::string, CLI::Option*> optionsMap;
        };

        struct ClearOptions
        {
            ClearOptions(CLI::App* rootCli) : clearType(std::make_shared<std::string>("all"))
            {
                optionsMap = std::map<std::string, CLI::Option*>{
                    {"type",
                     rootCli
                         ->add_option("type",
                                      *clearType,
                                      "Type of clearing to perform. "
                                      "Possible values: all, warn, err")
                         ->default_str("all")},
                };
            }
            const std::shared_ptr<std::string>  clearType;
            std::map<std::string, CLI::Option*> optionsMap;
        };

        struct ConfigOptions
        {
            ConfigOptions(CLI::App* rootCli) : configFile(std::make_shared<std::string>(""))
            {
                optionsMap = std::map<std::string, CLI::Option*>{
                    {"file",
                     rootCli->add_option("-f,--file", *configFile, "Path to the MD config file.")
                         ->required()}};
            }

            const std::shared_ptr<std::string>  configFile;
            std::map<std::string, CLI::Option*> optionsMap;
        };
    };
}  // namespace mab
