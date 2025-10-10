#pragma once
#include <memory>
#include <string>
#include <cstdlib>
#include <filesystem>
#include "mab_types.hpp"
#include "md_types.hpp"
#include "CLI/CLI.hpp"
#include "logger.hpp"
#include "candlelib.hpp"

namespace mab
{
    enum class sysArch_E
    {
        Unknown,
        X86_64,
        ARMHF,
        ARM64,
    };

    constexpr std::string_view MAB_LOGO_HEADER =
        "   ___     _     _  _      _   _         _____               _ \n"
        "  / __|   /_\\   | \\| |  __| | | |  ___  |_   _|  ___   ___  | |\n"
        " | (__   / _ \\  | .` | / _` | | | / -_)   | |   / _ \\ / _ \\ | |\n"
        "  \\___| /_/ \\_\\ |_|\\_| \\__,_| |_| \\___|   |_|   \\___/ \\___/ |_|\n"
        "                                                               \n"
        "For more information please refer to the manual: "
        "\033[32mhttps://mabrobotics.pl/servos/manual\033[0m \n\n";

    std::string trim(const std::string_view s);

    constexpr sysArch_E getSysArch()
    {
#if defined(_M_X64) || defined(__amd64__) || defined(__x86_64__)
        return sysArch_E::X86_64;
#elif defined(__aarch64__) || defined(_M_ARM64)
        return sysArch_E::ARM64;
#elif defined(__arm__) || defined(_M_ARM)
        return sysArch_E::ARMHF;
#else
        return sysArch_E::Unknown;
#endif
    }

    /// @brief command executor function
    /// @param command command to execute in the shell
    /// @return true on error, false on success
    inline bool executeCommand(const std::string_view command)
    {
        Logger log(Logger::ProgramLayer_E::LAYER_2, "CMD_EXECUTOR");
        log.debug("Executing command: %s", command.data());
        int ret = std::system(command.data());
        if (ret != 0)
        {
            log.error("Failed to execute command: %s", command.data());
            return true;
        }
        return false;
    }

    /// @brief this struct will be used to share information across CANdleTool
    struct CANdleToolCtx_S
    {
        std::vector<CANdleBranch_S>                  candleBranchVec;
        const std::shared_ptr<std::filesystem::path> packageEtcPath =
            std::make_shared<std::filesystem::path>(DEFAULT_CANDLETOOL_CONFIG_DIR);
    };

    class MABDescriptionFormatter : public CLI::Formatter
    {
        std::string make_description(const CLI::App* app) const override
        {
            std::string desc        = app->get_description();
            auto        min_options = app->get_require_option_min();
            auto        max_options = app->get_require_option_max();

            if (app->get_required())
            {
                desc += " " + get_label("REQUIRED") + " ";
            }

            if (min_options > 0)
            {
                if (max_options == min_options)
                {
                    desc += " \n[Exactly " + std::to_string(min_options) +
                            " of the following options are required]";
                }
                else if (max_options > 0)
                {
                    desc += " \n[Between " + std::to_string(min_options) + " and " +
                            std::to_string(max_options) + " of the following options are required]";
                }
                else
                {
                    desc += " \n[At least " + std::to_string(min_options) +
                            " of the following options are required]";
                }
            }
            else if (max_options > 0)
            {
                desc += " \n[At most " + std::to_string(max_options) +
                        " of the following options are allowed]";
            }

            return (!desc.empty()) ? std::string(MAB_LOGO_HEADER) + desc + "\n\n"
                                   : std::string(MAB_LOGO_HEADER);
        }
    };

}  // namespace mab
