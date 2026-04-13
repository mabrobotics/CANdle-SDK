#pragma once

#include "CLI/CLI.hpp"
#include "candle/objectDictionary/edsEntry.hpp"
#include "logger.hpp"
#include "candle/MD/MDCO.hpp"
#include "candle/communication_device/candle.hpp"
#include "mini/ini.h"
#include "configHelpers.hpp"
#include "candle/objectDictionary/edsParser.hpp"
#include "mab_types.hpp"
#include "utilities.hpp"

#include <filesystem>
#include <limits>
#include <memory>
#include <string>

namespace mab
{

    class MdcoCli
    {
      public:
        MdcoCli() = delete;
        MdcoCli(CLI::App& rootCli, CANdleToolCtx_S ctx);
        ~MdcoCli() = default;

      private:
        Logger                         m_log;
        CLI::App&                      m_rootCli;
        std::shared_ptr<CandleBuilder> m_candleBuilder;
        CANdleToolCtx_S                m_ctx;

        std::unique_ptr<MDCO, std::function<void(MDCO*)>> getMdco(
            const std::shared_ptr<canId_t> mdCanId, std::shared_ptr<EDSObjectDictionary> od);

        struct CalibrationOptions
        {
            CalibrationOptions(CLI::App* rootCli)
                : calibrationOfEncoder(std::make_shared<std::string>("main"))
            {
                optionsMap = std::map<std::string, CLI::Option*>{

                    {"encoder",
                     rootCli
                         ->add_option("-e,--encoder",
                                      *calibrationOfEncoder,
                                      "Type of encoder calibration to perform. "
                                      "Possible values: main, aux.")
                         ->default_val("main")}};
            }

            const std::shared_ptr<std::string>  calibrationOfEncoder;
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

        struct CanOptions
        {
            CanOptions(CLI::App* rootCli) : canId(std::make_shared<canId_t>(10))
            {
                optionsMap = std::map<std::string, CLI::Option*>{
                    {"id",
                     rootCli
                         ->add_option("--new_id", *canId, "New CAN node id for the MD controller.")
                         ->required()}};
            }
            const std::shared_ptr<canId_t> canId;

            std::map<std::string, CLI::Option*> optionsMap;
        };

        struct ReadOptions
        {
            ReadOptions(CLI::App* rootCli)
                : index(std::make_shared<u16>(0)), subindex(std::make_shared<std::optional<u8>>())
            {
                optionsMap = std::map<std::string, CLI::Option*>{
                    {"index",
                     rootCli
                         ->add_option("--index", *index, "Register ID (offset) to read data from.")
                         ->required()},
                    {"subindex",
                     rootCli->add_option(
                         "--subindex", *subindex, "Register ID (offset) to read data from.")}};
            }

            const std::shared_ptr<u16>               index;
            const std::shared_ptr<std::optional<u8>> subindex;
            std::map<std::string, CLI::Option*>      optionsMap;
        };

        struct MoveOptions
        {
            MoveOptions(CLI::App* rootCli) : position(std::make_shared<i32>(0))
            {
                rootCli
                    ->add_option(
                        "position", *position, "Absolute position to reach [encoder ticks].")
                    ->required();
            }
            const std::shared_ptr<i32>          position;
            std::map<std::string, CLI::Option*> optionsMap;
        };

        struct WriteOptions
        {
            WriteOptions(CLI::App* rootCli)
                : index(std::make_shared<u16>(0)),
                  subindex(std::make_shared<std::optional<u8>>()),
                  valueStr(std::make_shared<std::string>())
            {
                optionsMap = std::map<std::string, CLI::Option*>{
                    {"index",
                     rootCli
                         ->add_option("--index", *index, "Register ID (offset) to read data from.")
                         ->required()},
                    {"subindex",
                     rootCli->add_option(
                         "--subindex", *subindex, "Register ID (offset) to read data from.")},
                    {"value",
                     rootCli->add_option("--value", *valueStr, "Value to write by sdo")
                         ->required()}};
            }
            const std::shared_ptr<u16>               index;
            const std::shared_ptr<std::optional<u8>> subindex;
            const std::shared_ptr<std::string>       valueStr;
            std::map<std::string, CLI::Option*>      optionsMap;
        };
    };
}  // namespace mab
