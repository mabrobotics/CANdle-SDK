#pragma once

#include "CLI/CLI.hpp"
#include "candle.hpp"
#include "logger.hpp"
#include "mab_types.hpp"

#include <filesystem>
#include <map>
#include <memory>
#include <string>

namespace mab
{
    /// @brief How the drive is put into the bootloader before the new firmware is sent
    enum class UpdateReset_E
    {
        /// @brief drives running MD firmware, reset over the MD protocol
        MD_PROTOCOL,
        /// @brief drives running CANopen firmware, reset with an SDO write on a CAN 2.0 link
        CANOPEN
    };

    /// @brief Arguments of the update command, shared by `md update` and `mdco update`
    struct UpdateOptions
    {
        /// @param rootCli subcommand the arguments are added to
        /// @param commandName name of the command the arguments belong to, quoted back in the
        /// examples the help prints
        UpdateOptions(CLI::App* rootCli, const std::string& commandName = "md")
            : fwVersion(std::make_shared<std::string>("")),
              pathToMabFile(std::make_shared<std::filesystem::path>("")),
              recovery(std::make_shared<bool>(false)),
              forceErase(std::make_shared<bool>(false))
        {
            optionsMap = std::map<std::string, CLI::Option*>{
                {"version",
                 rootCli->add_option("version",
                                     *fwVersion,
                                     "Version of fw to download (\"latest\" or X.X.X format). "
                                     "For example:  candletool " +
                                         commandName + " update latest")},
                {"path",
                 rootCli->add_option("-p,--path", *pathToMabFile, "Local path to .mab file")},
                {"recovery",
                 rootCli->add_flag(
                     "-r,--recovery", *recovery, "Driver recovery after failed flashing")},
                {"force_erase",
                 rootCli->add_flag("--force-erase", *forceErase, "Force full wipe of the driver")}};
        }
        const std::shared_ptr<std::string>           fwVersion;
        const std::shared_ptr<std::filesystem::path> pathToMabFile;
        const std::shared_ptr<bool>                  recovery;
        const std::shared_ptr<bool>                  forceErase;
        std::map<std::string, CLI::Option*>          optionsMap;
    };

    /// @brief Carry out the update command
    ///
    /// The firmware is sent over the MD protocol either way - only the reset that puts the drive
    /// into the bootloader depends on the firmware the drive is currently running.
    ///
    /// @param options arguments the command was called with
    /// @param resetMode protocol the drive is reset with
    /// @param mdCanId can node id of the drive to update
    /// @param candleBuilder builder of the candle used for communication
    /// @param packageEtcPath etc path of the package, holds candletool.ini
    /// @param log logger of the calling cli
    void updateMd(const UpdateOptions&                       options,
                  UpdateReset_E                              resetMode,
                  const std::shared_ptr<canId_t>             mdCanId,
                  const std::shared_ptr<const CandleBuilder> candleBuilder,
                  const std::filesystem::path&               packageEtcPath,
                  const Logger&                              log);
}  // namespace mab
