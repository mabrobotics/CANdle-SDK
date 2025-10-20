#pragma once
#include "CLI/CLI.hpp"
#include "candle.hpp"
#include "mab_types.hpp"
#include "logger.hpp"
#include "MD.hpp"
#include "utilities.hpp"
#include <algorithm>
#include <memory>
#include <optional>
#include <candle_types.hpp>

namespace mab
{

    class MDCli
    {
      public:
        MDCli() = delete;
        MDCli(CLI::App* rootCli, CANdleToolCtx_S ctx);
        ~MDCli() = default;

      private:
        Logger m_logger = Logger(Logger::ProgramLayer_E::TOP, "MD_CLI");
        std::unique_ptr<MD, std::function<void(MD*)>> getMd(
            const std::shared_ptr<canId_t>             mdCanId,
            const std::shared_ptr<const CandleBuilder> candleBuilder);
        bool                       registerWrite(MD& md, u16 regAdress, const std::string& value);
        std::optional<std::string> registerRead(MD& md, u16 regAdress);

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
                         "--new_id", *canId, "New CAN node id for the MD controller.")},
                    {"datarate",
                     rootCli->add_option("--new_datarate",
                                         *datarate,
                                         "New datarate of the MD controller. [1M, 2M, 5M, 8M]")},
                    {"timeout",
                     rootCli->add_option(
                         "--new_timeout", *timeoutMs, "New timeout of the MD controller.")},
                    {"save",
                     rootCli->add_flag(
                         "--save", *save, "Save the new CAN parameters to the MD controller.")}};
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
                : calibrationOfEncoder(std::make_shared<std::string>("all")),
                  runTests(std::make_shared<bool>(false))
            {
                optionsMap = std::map<std::string, CLI::Option*>{

                    {"encoder",
                     rootCli
                         ->add_option("-e,--encoder",
                                      *calibrationOfEncoder,
                                      "Type of encoder calibration to perform. "
                                      "Possible values: all, main, aux.")
                         ->default_str("all")},
                    {"run-tests",
                     rootCli->add_flag(
                         "-t,--run-tests", *runTests, "Run accuracy tests after calibration. ")}};
            }

            const std::shared_ptr<std::string>  calibrationOfEncoder;
            const std::shared_ptr<bool>         runTests;
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
                     rootCli
                         ->add_option(
                             "file",
                             *configFile,
                             "Path to the MD config file \n note: if no \"/\" sign is present than "
                             "global config path will be prepended.")
                         ->required()}};
            }

            const std::shared_ptr<std::string>  configFile;
            std::map<std::string, CLI::Option*> optionsMap;
        };

        struct RegisterReadOption
        {
            RegisterReadOption(CLI::App* rootCli)
                : registerAddressOrName(std::make_shared<std::string>("0x001"))
            {
                optionsMap = std::map<std::string, CLI::Option*>{
                    {"register",
                     rootCli
                         ->add_option("register",
                                      *registerAddressOrName,
                                      "Name or address (must start with 0x and be in hex) of a "
                                      "register to interact with")
                         ->required()}};
            }

            const std::shared_ptr<std::string>  registerAddressOrName;
            std::map<std::string, CLI::Option*> optionsMap;
        };

        struct RegisterWriteOption : public RegisterReadOption
        {
            RegisterWriteOption(CLI::App* rootCli)
                : RegisterReadOption(rootCli), registerValue(std::make_shared<std::string>("0x0"))
            {
                optionsMap["value"] =
                    rootCli->add_option("value", *registerValue, "Value to write to register")
                        ->required();
            }

            const std::shared_ptr<std::string> registerValue;
        };

        struct TestOptions
        {
            TestOptions(CLI::App* rootCli) : target(std::make_shared<float>(0.0f))
            {
                optionsMap = std::map<std::string, CLI::Option*>{
                    {"target",
                     rootCli->add_option("target", *target, "Position target of movement")}};
            }
            const std::shared_ptr<float>        target;
            std::map<std::string, CLI::Option*> optionsMap;
        };

        struct UpdateOptions
        {
            UpdateOptions(CLI::App* rootCli)
                : fwVersion(std::make_shared<std::string>("")),
                  pathToMabFile(std::make_shared<std::string>("")),
                  recovery(std::make_shared<bool>(false)),
                  metadataFile(std::make_shared<std::string>(""))
            {
                optionsMap = std::map<std::string, CLI::Option*>{
                    {"version",
                     rootCli->add_option("version",
                                         *fwVersion,
                                         "Version of fw to download (\"latest\" or X.X.X format). "
                                        "For example:  candletool md update latest")},
                    {"path",
                     rootCli->add_option("-p,--path",
                                         *pathToMabFile,
                                         "Local path to .mab file")},
                    {"recovery",
                     rootCli->add_flag(
                         "-r,--recovery", *recovery, "Driver recovery after failed flashing")},
                    {"meta_file",
                     rootCli->add_option("-m,--meta-file",
                                         *metadataFile,
                                         "File with file metadata for managing downloads.")}};
            }
            const std::shared_ptr<std::string>  fwVersion;
            const std::shared_ptr<std::string>  pathToMabFile;
            const std::shared_ptr<bool>         recovery;
            const std::shared_ptr<std::string>  metadataFile;
            std::map<std::string, CLI::Option*> optionsMap;
        };  // namespace mab
    };
}  // namespace mab
