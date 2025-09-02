#pragma once
#include "CLI/CLI.hpp"
#include "candle.hpp"
#include "mab_types.hpp"
#include "logger.hpp"
#include "candle_types.hpp"
#include "utilities.hpp"

namespace mab
{

    class CandleCli
    {
      public:
        CandleCli() = delete;
        CandleCli(CLI::App* rootCli, CANdleToolCtx_S ctx);
        ~CandleCli() = default;

      private:
        Logger m_logger = Logger(Logger::ProgramLayer_E::TOP, "CANDLE_CLI");

        struct UpdateOptions
        {
            UpdateOptions(CLI::App* rootCli)
                : fwVersion(std::make_shared<std::string>("")),
                  pathToMabFile(std::make_shared<std::string>("")),
                  metadataFile(std::make_shared<std::string>(""))
            {
                optionsMap = std::map<std::string, CLI::Option*>{
                    {"version",
                     rootCli->add_option("version",
                                         *fwVersion,
                                         "Version of fw to download (latest or X.X.X format)")},
                    {"path",
                     rootCli->add_option("-p,--path",
                                         *pathToMabFile,
                                         "Override download and provide local path to .mab file")},
                    {"meta_file",
                     rootCli->add_option("-m,--meta-file",
                                         *metadataFile,
                                         "File with file metadata for managing downloads.")}};
            }
            const std::shared_ptr<std::string>  fwVersion;
            const std::shared_ptr<std::string>  pathToMabFile;
            const std::shared_ptr<std::string>  metadataFile;
            std::map<std::string, CLI::Option*> optionsMap;
        };
    };
}  // namespace mab
