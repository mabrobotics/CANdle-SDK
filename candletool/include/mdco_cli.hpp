#pragma once

#include "CLI/CLI.hpp"
#include "edsEntry.hpp"
#include "logger.hpp"
#include "MDCO.hpp"
#include "candle.hpp"
#include "mini/ini.h"
#include "configHelpers.hpp"
#include "edsParser.hpp"
#include "mab_types.hpp"
#include "mdco_cfg_map.hpp"
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

        /// @brief It returns the same string, without any spaces and all in lowercase.
        /// @param s Input string to be cleaned.
        void clean(std::string& s);

        /// @brief Check if the CANopen configuration file is complete
        bool isCanOpenConfigComplete(const std::filesystem::path& pathToConfig);

        /// @brief Validate and get the final configuration path
        /// @param cfgPath Path to the configuration file to verify
        /// @return Validated path to the configuration file
        std::string validateAndGetFinalConfigPath(const std::filesystem::path& cfgPath);

        std::unique_ptr<MDCO, std::function<void(MDCO*)>> getMdco(
            const std::shared_ptr<canId_t> mdCanId, std::shared_ptr<EDSObjectDictionary> od);

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
