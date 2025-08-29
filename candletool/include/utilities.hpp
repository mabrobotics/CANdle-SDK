#pragma once
#include <memory>
#include <string>
#include "mab_types.hpp"
#include "md_types.hpp"
#include "CLI/CLI.hpp"

namespace mab
{

    constexpr std::string_view MAB_LOGO_HEADER =
        "   ___     _     _  _      _   _         _____               _ \n"
        "  / __|   /_\\   | \\| |  __| | | |  ___  |_   _|  ___   ___  | |\n"
        " | (__   / _ \\  | .` | / _` | | | / -_)   | |   / _ \\ / _ \\ | |\n"
        "  \\___| /_/ \\_\\ |_|\\_| \\__,_| |_| \\___|   |_|   \\___/ \\___/ |_|\n"
        "                                                               \n"
        "For more information please refer to the manual: "
        "\033[32mhttps://mabrobotics.pl/servos/manual\033[0m \n\n";

    std::string trim(const std::string_view s);

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
