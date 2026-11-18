#pragma once
#include "CLI/CLI.hpp"
#include "candle.hpp"
#include "mab_types.hpp"
#include "logger.hpp"
#include "MD.hpp"
#include "utilities.hpp"
#include <filesystem>
#include <map>
#include <string>
#include <memory>
#include <optional>
#include <candle_types.hpp>

namespace mab
{
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
              forceErase(std::make_shared<bool>(false)),
              readStorage(std::make_shared<std::filesystem::path>("")),
              otherVariant(std::make_shared<bool>(false)),
              legacyMigration(std::make_shared<bool>(false))
        {
            // Drives accept both MD and MDCO firmware, the flag downloads the variant opposite
            // to the one the command resets the drive for
            const bool        canOpen     = commandName == "mdco";
            const std::string variantFlag = canOpen ? "--md" : "--mdco";
            CLI::Option*      pathOption =
                rootCli->add_option("-p,--path", *pathToMabFile, "Local path to .mab file");
            optionsMap = std::map<std::string, CLI::Option*>{
                {"version",
                 rootCli->add_option("version",
                                     *fwVersion,
                                     "Version of fw to download (\"latest\" or X.X.X format). "
                                     "For example:  candletool " +
                                         commandName + " update latest")},
                {"path", pathOption},
                {"other_variant",
                 rootCli
                     ->add_flag(variantFlag,
                                *otherVariant,
                                std::string("Download ") + (canOpen ? "MD" : "MDCO") +
                                    " firmware instead, switching the drive to it")
                     ->excludes(pathOption)},
                {"recovery",
                 rootCli->add_flag(
                     "-r,--recovery", *recovery, "Driver recovery after failed flashing")},
                {"force_erase",
                 rootCli->add_flag("--force-erase", *forceErase, "Force full wipe of the driver")}};
            optionsMap["read_storage"] =
                rootCli
                    ->add_option("--read-storage",
                                 *readStorage,
                                 "Save the config storage of a G5 drive to a file and start the "
                                 "firmware again, nothing is flashed")
                    ->excludes(optionsMap["force_erase"]);

            // Legacy drives may not report a usable firmware version, so they are reset into the
            // bootloader blindly and flashed as in recovery
            if (canOpen)
                optionsMap["legacy_migration"] =
                    rootCli
                        ->add_flag("--legacy-migration",
                                   *legacyMigration,
                                   "Reset the drive and flash it in recovery mode, without reading "
                                   "its firmware version. For drives running firmware older than "
                                   "3.0.0")
                        ->excludes(optionsMap["recovery"])
                        ->excludes(optionsMap["force_erase"]);
        }
        const std::shared_ptr<std::string>           fwVersion;
        const std::shared_ptr<std::filesystem::path> pathToMabFile;
        const std::shared_ptr<bool>                  recovery;
        const std::shared_ptr<bool>                  forceErase;
        const std::shared_ptr<std::filesystem::path> readStorage;
        const std::shared_ptr<bool>                  otherVariant;
        const std::shared_ptr<bool>                  legacyMigration;
        std::map<std::string, CLI::Option*>          optionsMap;
    };

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
        bool                       registerWrite(MD&                md,
                                                 u16                regAdress,
                                                 const std::string& value,
                                                 bool               quiet = false);
        std::optional<std::string> registerRead(MD& md, u16 regAdress);
        /// @brief Read a register not listed in MDRegisters_S
        /// @param type u8, i8, u16, i16, u32, i32, f32 or char[N]
        std::optional<std::string> registerReadRaw(MD& md, u16 regAdress, const std::string& type);

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
                  autodetect(std::make_shared<bool>(false))
            {
                optionsMap = std::map<std::string, CLI::Option*>{
                    {"encoder",
                     rootCli
                         ->add_option("-e,--encoder",
                                      *calibrationOfEncoder,
                                      "Type of encoder calibration to perform. "
                                      "Possible values: all, main, aux.")
                         ->default_str("all")},
                    {"autodetect",
                     rootCli->add_flag("--autodetect",
                                       *autodetect,
                                       "Clear motor resistance and inductance first, so "
                                       "the calibration measures them again.")}};
            }

            const std::shared_ptr<std::string>  calibrationOfEncoder;
            const std::shared_ptr<bool>         autodetect;
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
            ConfigOptions(CLI::App* rootCli)
                : configFile(std::make_shared<std::filesystem::path>(""))
            {
                optionsMap = std::map<std::string, CLI::Option*>{
                    {"file",
                     rootCli
                         ->add_option(
                             "file",
                             *configFile,
                             "Path to the MD .cfg file \n note: \ncan be absolute path, "
                             "\ncurrent directory relative path (starting with `./`, eg. "
                             "`./myCustomMotor.cfg`), "
                             "\ndefault config relative path (eg. `CubeMars/AK80-9.cfg`)")
                         ->required()}};
            }

            const std::shared_ptr<std::filesystem::path> configFile;
            std::map<std::string, CLI::Option*>          optionsMap;
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

        struct MoveTestOptions
        {
            MoveTestOptions(CLI::App* rootCli) : target(std::make_shared<float>(0.0f))
            {
                optionsMap = std::map<std::string, CLI::Option*>{
                    {"target",
                     rootCli->add_option("target", *target, "Position target of movement")}};
            }
            const std::shared_ptr<float>        target;
            std::map<std::string, CLI::Option*> optionsMap;
        };
        struct EncoderTestOptions
        {
            EncoderTestOptions(CLI::App* rootCli) : encoder(std::make_shared<std::string>("main"))
            {
                optionsMap = std::map<std::string, CLI::Option*>{
                    {"encoder",
                     rootCli->add_option(
                         "encoder", *encoder, "Encoder to test - `main` or `aux`")}};
            }
            const std::shared_ptr<std::string>  encoder;
            std::map<std::string, CLI::Option*> optionsMap;
        };
    };
}  // namespace mab
