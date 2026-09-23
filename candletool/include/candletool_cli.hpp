#pragma once

#include <compare>
#include <optional>
#include <string>

#include "CLI/CLI.hpp"
#include "logger.hpp"

#include "utilities.hpp"

namespace mab
{
    struct CandletoolVersion
    {
        int major = 0;
        int minor = 0;
        int patch = 0;

        auto operator<=>(const CandletoolVersion&) const = default;
    };

    /// @brief parses a release tag such as "v1.5.0"
    /// @return nullopt if the tag is not in [v]major.minor.patch form
    std::optional<CandletoolVersion> parseVersion(const std::string& tag);

    class CandletoolCli
    {
      public:
        CandletoolCli() = delete;
        CandletoolCli(CLI::App* rootCli, CANdleToolCtx_S ctx);
        ~CandletoolCli() = default;

      private:
        std::string repoUrl = "https://api.github.com/repos/mabrobotics/CANdle-SDK/releases/latest";
        std::optional<std::string> fetchUrl(const std::string& url);
        Logger                     m_logger = Logger(Logger::ProgramLayer_E::TOP, "CANDLETOOL_CLI");

        bool downloadFile(const std::string& url, const std::filesystem::path& outputPath);

        bool installPackage(const std::filesystem::path& path);
    };
}  // namespace mab