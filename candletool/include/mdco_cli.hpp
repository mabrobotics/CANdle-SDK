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
#include <memory>

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
            CanOptions(CLI::App* rootCli)
                : canId(std::make_shared<canId_t>(10)),
                  datarate(std::make_shared<std::string>("1M")),
                  timeoutMs(std::make_shared<uint16_t>(200)),
                  save(std::make_shared<bool>(false))
            {
                optionsMap = std::map<std::string, CLI::Option*>{
                    {"id",
                     rootCli
                         ->add_option("--new_id", *canId, "New CAN node id for the MD controller.")
                         ->required()},
                    {"datarate",
                     rootCli->add_option("--new_datarate",
                                         *datarate,
                                         "New datarate of the MD controller. 1M or 500K")},
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

        struct ReadWriteOptions
        {
            ReadWriteOptions(CLI::App* rootCli)
                : index(std::make_shared<u16>()),
                  subindex(std::make_shared<u8>(0)),
                  force(std::make_shared<bool>(false))

            {
                optionsMap = std::map<std::string, CLI::Option*>{
                    {"index",
                     rootCli
                         ->add_option("--index", *index, "Register ID (offset) to read data from.")
                         ->required()},
                    {"subindex",
                     rootCli->add_option(
                         "--subindex", *subindex, "Register ID (offset) to read data from.")},
                    {"force",
                     rootCli
                         ->add_flag("-f,--force",
                                    *force,
                                    "Force reading message, without verification in the Object "
                                    "dictionary.")
                         ->default_val(false)}};
            }

            const std::shared_ptr<u16>          index;
            const std::shared_ptr<u8>           subindex;
            const std::shared_ptr<bool>         force;
            std::map<std::string, CLI::Option*> optionsMap;
        };
    };
}  // namespace mab
