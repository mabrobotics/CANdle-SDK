#pragma once
#include "CLI/CLI.hpp"
#include "candle.hpp"
#include "mab_types.hpp"
#include "logger.hpp"
#include "candle_types.hpp"

namespace mab
{

    class CandleCli
    {
      public:
        CandleCli() = delete;
        CandleCli(CLI::App* rootCli, const std::shared_ptr<const CandleBuilder> candleBuilder);
        ~CandleCli() = default;

      private:
        Logger m_logger = Logger(Logger::ProgramLayer_E::TOP, "CANDLE_CLI");

        struct UpdateOptions
        {
            UpdateOptions(CLI::App* rootCli) : pathToMabFile(std::make_shared<std::string>(""))
            {
                optionsMap = std::map<std::string, CLI::Option*>{
                    {"path",
                     rootCli->add_option("path", *pathToMabFile, "Path to .mab file")->required()}};
            }
            const std::shared_ptr<bool>         recovery;
            const std::shared_ptr<std::string>  pathToMabFile;
            std::map<std::string, CLI::Option*> optionsMap;
        };
    };
}  // namespace mab
