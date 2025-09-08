#pragma once

#include "CLI/CLI.hpp"
#include "logger.hpp"
#include "MDCO.hpp"
#include "candle.hpp"
#include "mini/ini.h"
#include "configHelpers.hpp"
#include "edsParser.hpp"

namespace mab
{

    struct UserCommandCO
    {
        u32           id           = 0x0000;
        u8            subReg       = 0x0;
        u8            dataSize     = 0x0;
        std::string   cfgPath      = "";
        std::string   value        = "";
        std::string   reg          = "0x0000";
        bool          force        = false;
        i32           desiredPos   = 0;
        i32           desiredSpeed = 0;
        moveParameter param;
        edsObject     edsObj;  // used for EDS parser
    };

    class MdcoCli
    {
      public:
        MdcoCli() = delete;
        MdcoCli(CLI::App& rootCli, const std::shared_ptr<CandleBuilder> candleBuilder);
        ~MdcoCli() = default;

        /// @brief Parse the command line arguments for the MDCO CLI
        /// @param baud The baud rate for the CANdle device
        void parse(mab::CANdleDatarate_E baud);

      private:
        Logger                               m_log;
        CLI::App&                            m_rootCli;
        const std::shared_ptr<CandleBuilder> m_candleBuilder;
        //  principal subcommands
        CLI::App* mdco         = nullptr;
        CLI::App* blink        = nullptr;
        CLI::App* clear        = nullptr;
        CLI::App* can          = nullptr;
        CLI::App* discover     = nullptr;
        CLI::App* eds          = nullptr;
        CLI::App* encoder      = nullptr;
        CLI::App* heartbeat    = nullptr;
        CLI::App* nmt          = nullptr;
        CLI::App* pdoTest      = nullptr;
        CLI::App* registr      = nullptr;
        CLI::App* reset        = nullptr;
        CLI::App* setup        = nullptr;
        CLI::App* Sync         = nullptr;
        CLI::App* SDOsegmented = nullptr;
        CLI::App* test         = nullptr;
        CLI::App* timeStamp    = nullptr;

        // eds subcommands
        CLI::App* edsLoad             = nullptr;
        CLI::App* edsUnload           = nullptr;
        CLI::App* edsDisplay          = nullptr;
        CLI::App* edsGen              = nullptr;
        CLI::App* edsGenMd            = nullptr;
        CLI::App* edsGenHtml          = nullptr;
        CLI::App* edsGencpp           = nullptr;
        CLI::App* edsGet              = nullptr;
        CLI::App* edsFind             = nullptr;
        CLI::App* edsModify           = nullptr;
        CLI::App* edsModifyAdd        = nullptr;
        CLI::App* edsModifyDel        = nullptr;
        CLI::App* edsModifyCorrection = nullptr;

        // encoder subcommands
        CLI::App* encoderDisplay  = nullptr;
        CLI::App* testEncoder     = nullptr;
        CLI::App* testEncoderMain = nullptr;
        CLI::App* testEncoderOut  = nullptr;
        // nmt subcommands
        CLI::App* nmtRead               = nullptr;
        CLI::App* nmtOperational        = nullptr;
        CLI::App* nmtStop               = nullptr;
        CLI::App* nmtPreOperational     = nullptr;
        CLI::App* nmtResetNode          = nullptr;
        CLI::App* nmtResetCommunication = nullptr;

        // register subcommands
        CLI::App* regRead  = nullptr;
        CLI::App* regWrite = nullptr;

        // pdoTestco subcommands
        CLI::App* PdoSpeed    = nullptr;
        CLI::App* PdoPosition = nullptr;
        CLI::App* PdoCustom   = nullptr;

        // setupco subcommands
        CLI::App*    setupCalib       = nullptr;
        CLI::App*    setupCalibOut    = nullptr;
        CLI::App*    setupInfo        = nullptr;
        CLI::App*    setupupload      = nullptr;
        CLI::App*    setupdownload    = nullptr;
        CLI::Option* setupInfoAllFlag = nullptr;

        // SDOsegmentedco subcommands
        CLI::App* SDOsegmentedRead  = nullptr;
        CLI::App* SDOsegmentedWrite = nullptr;

        // testco subcommands

        CLI::App* testLatency   = nullptr;
        CLI::App* testMove      = nullptr;
        CLI::App* testMoveAbs   = nullptr;
        CLI::App* testMoveRel   = nullptr;
        CLI::App* testMoveSpeed = nullptr;
        CLI::App* testImpedance = nullptr;

        void        clean(std::string& s);
        bool        isCanOpenConfigComplete(const std::string& pathToConfig);
        std::string validateAndGetFinalConfigPath(const std::string& cfgPath);
        void        updateUserChoice();

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

        struct HeartbeatOptions
        {
            HeartbeatOptions(CLI::App* rootCli)
                : masterCanId(std::make_shared<canId_t>(10)),
                  slaveCanId(std::make_shared<canId_t>(11)),
                  timeout(std::make_shared<uint32_t>(1000))
            {
                optionsMap = std::map<std::string, CLI::Option*>{
                    {"master_id",
                     rootCli
                         ->add_option("--master_id",
                                      *masterCanId,
                                      "CAN ID of the master who will produce the heartbeat.")
                         ->required()},
                    {"slave_id",
                     rootCli
                         ->add_option(
                             "--slave_id",
                             *slaveCanId,
                             "CAN ID of the slave who will follow the heartbeat (A MD must "
                             "be attach).")
                         ->required()},
                    {"timeout",
                     rootCli->add_option(
                         "-t,--timeout",
                         *timeout,
                         "Duration after which the engine goes into preoperational state if it "
                         "has not received a heartbeat from the master node [ms].")}};
            }
            const std::shared_ptr<canId_t>  masterCanId;
            const std::shared_ptr<canId_t>  slaveCanId;
            const std::shared_ptr<uint32_t> timeout;

            std::map<std::string, CLI::Option*> optionsMap;
        };

        struct EdsObjectOPtions
        {
            EdsObjectOPtions(CLI::App* rootCli) : EdsObject(std::make_shared<edsObject>())
            {
                optionsMap = std::map<std::string, CLI::Option*>{
                    {"index",
                     rootCli
                         ->add_option(
                             "--index", (*EdsObject).index, "Index of the parameter to add.")
                         ->required()},
                    {"subindex",
                     rootCli
                         ->add_option("--subindex",
                                      (*EdsObject).subIndex,
                                      "Subindex of the parameter to add.")
                         ->required()},
                    {"name",
                     rootCli
                         ->add_option(
                             "--name", (*EdsObject).ParameterName, "Name of the parameter to add.")
                         ->required()},
                    {"StorageLocation",
                     rootCli->add_option(
                         "--StorageLocation",
                         (*EdsObject).StorageLocation,
                         "Storage location of the parameter to add(e.g.RAM,PERSIST_COMM,etc.).")},
                    {"DataType",
                     rootCli->add_option("--DataType",
                                         (*EdsObject).DataType,
                                         "Data type of the parameter to add (e.g. "
                                         "0x0001=BOOLEAN, 0x0007=UNSIGNED32, etc.).")},
                    {"DefaultValue",
                     rootCli->add_option("--DefaultValue",
                                         (*EdsObject).defaultValue,
                                         "Default value of the parameter to add.")},
                    {"AccessType",
                     rootCli->add_option(
                         "--AccessType",
                         (*EdsObject).accessType,
                         "Access type of the parameter to add (e.g. R, RW, etc.).")},
                    {"PdoMapping",
                     rootCli->add_option("--PdoMapping",
                                         (*EdsObject).PDOMapping,
                                         "PDOMapping of the parameter to add (0=no, 1=yes).")},
                    {"ObjectType",
                     rootCli->add_option("--ObjectType",
                                         (*EdsObject).sectionType,
                                         "Object type of the parameter to add (0=Mandatory, "
                                         "1=Optional, 2=Manufacturer).")}};
            }
            const std::shared_ptr<edsObject> EdsObject;

            std::map<std::string, CLI::Option*> optionsMap;
        };

        struct MovementLimitsOPtions
        {
            MovementLimitsOPtions(CLI::App* rootCli) : param(std::make_shared<moveParameter>())
            {
                optionsMap = std::map<std::string, CLI::Option*>{
                    {"MaxVelocity",
                     rootCli->add_option(
                         "--MaxVelocity", (*param).MaxSpeed, "Profile max velocity [rad/s].")},
                    {"MaxAcceleration",
                     rootCli->add_option("--MaxAcceleration",
                                         (*param).accLimit,
                                         "Profile max acceleration [rad/s^2].")},
                    {"MaxDeceleration",
                     rootCli->add_option("--MaxDeceleration",
                                         (*param).dccLimit,
                                         "Profile max deceleration [rad/s^2].")},
                    {"MaxCurrent",
                     rootCli->add_option("--MaxCurrent",
                                         (*param).MaxCurrent,
                                         "Configures the maximum allowed phase current in the "
                                         "motor. The value is "
                                         "expressed in permille of rated current.")},
                    {"RatedCurrent",
                     rootCli->add_option("--RatedCurrent",
                                         (*param).RatedCurrent,
                                         "Configures the maximum allowed torque in the motor. "
                                         "The value is expressed "
                                         "in permille of rated torque.")},
                    {"MaxTorque",
                     rootCli->add_option("--MaxTorque",
                                         (*param).MaxTorque,
                                         "Configures the maximum allowed phase current in the "
                                         "motor. The value is "
                                         "expressed in permille of rated current.")},
                    {"RatedTorque",
                     rootCli->add_option("--RatedTorque",
                                         (*param).RatedTorque,
                                         "Configures the motor rated torque expressed in mNm.")}};
            }
            const std::shared_ptr<moveParameter> param;

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