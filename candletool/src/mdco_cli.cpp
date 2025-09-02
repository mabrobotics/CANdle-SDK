#include "mdco_cli.hpp"
#include "CLI/CLI.hpp"
#include "logger.hpp"
#include "candletoolCO.hpp"
#include "MDCO.hpp"

using namespace mab;

UserCommandCO cmdCANopen;

MdcoCli::MdcoCli(CLI::App& rootCli, const std::shared_ptr<CandleBuilder> candleBuilder)
    : m_rootCli(rootCli), m_candleBuilder(candleBuilder)
{
    m_log.m_tag   = "MDCO";
    m_log.m_layer = Logger::ProgramLayer_E::TOP;
    mdco          = m_rootCli.add_subcommand("mdco", "Send CANopen command instead of CAN FD.");

    // CANopen command
    blink     = mdco->add_subcommand("blink", "Blink LEDs on MD drive.");
    can       = mdco->add_subcommand("can", "Configure CAN parameters of MD drive.");
    clear     = mdco->add_subcommand("clear", "Clear Errors and Warnings.");
    eds       = mdco->add_subcommand("eds", "EDS file analizer and generator.");
    encoder   = mdco->add_subcommand("encoder", "Display MD motor position.");
    heartbeat = mdco->add_subcommand("heartbeat", "Test heartbeat detection of a device.");
    nmt       = mdco->add_subcommand("nmt", "Send NMT commands.");
    pdoTest   = mdco->add_subcommand("pdo", "Try to send pdo can frame instead of sdo.");
    ping      = mdco->add_subcommand("ping", "Discover MD drives on CAN bus.");
    registr   = mdco->add_subcommand("register", "Access MD drive via register read/write.");
    reset     = mdco->add_subcommand("reset", "Reset MD drive.");
    setup     = mdco->add_subcommand("setup", "Setup MD via config files, and calibrate.");
    Sync      = mdco->add_subcommand("sync", "Send a sync CANopen message.");
    SDOsegmented =
        mdco->add_subcommand("segmented", "Send a message using SDO segmented transfer.");
    test = mdco->add_subcommand("test", "Basic MD drive testing routines.");
    timeStamp =
        mdco->add_subcommand("time", "Send a time stamp message using the computer's clock.");

    // BLINK CANopen
    blink->add_option("--id", cmdCANopen.id, "CAN ID of the MD to interact with.")->required();

    // can CANopen
    can->add_option("--id", cmdCANopen.id, "Current can ID.")->required();
    can->add_option("--new_id", cmdCANopen.newId, "New can ID to set.")->required();
    can->add_option("--baud", cmdCANopen.baud, "New can baudrate to set: 1M or 500K")->required();
    can->add_option("--watchdog", cmdCANopen.canWatchdog, "New can watchdog value");
    can->add_flag("--save", cmdCANopen.save, "Save the new CAN parameters to the MD controller.")
        ->default_val(false);

    // CLEAR
    clear->add_option("--id", cmdCANopen.id, "CAN ID of the MD to interact with.")->required();
    clear->add_option("--level", cmdCANopen.variant, "Can be: warning, error, all.")->required();

    // EDS CANopen
    edsLoad    = eds->add_subcommand("load", "Load the path to the .eds file to analyze.");
    edsUnload  = eds->add_subcommand("unload", "Delete the path to the .eds file.");
    edsDisplay = eds->add_subcommand("display", "Display the content of the .eds file.");
    edsGen     = eds->add_subcommand("generate", "Generate documentation from the .eds file.");
    edsGet  = eds->add_subcommand("get", "Display the content of an object from his index number.");
    edsFind = eds->add_subcommand("find", "Find and display the content of an object from words.");
    edsModify = eds->add_subcommand("modify", "Modify the actual .eds file.");
    edsLoad->add_option("--file_path", cmdCANopen.value, "Path to the .eds file to analyze.")
        ->required();
    edsGenMd = edsGen->add_subcommand("md", "Generate .md file from .eds file.");
    edsGenMd->add_option(
        "--path", cmdCANopen.cfgPath, "Path where the user want to generate the md file");
    edsGenHtml = edsGen->add_subcommand("html", "Generate .html file from .eds file.");
    edsGenHtml->add_option(
        "--path", cmdCANopen.cfgPath, "Path where the user want to generate the html file");
    edsGencpp = edsGen->add_subcommand("cpp", "Generate .cpp & .hpp file from .eds file.");
    edsGencpp->add_option(
        "--path", cmdCANopen.cfgPath, "Path where the user want to generate the cpp & hpp file");
    edsGet->add_option("--index", cmdCANopen.id, "Index of the object to get.")->required();
    edsGet->add_option("--subindex", cmdCANopen.subReg, "Subindex of the object to get.")
        ->required();
    edsFind->add_option("--word", cmdCANopen.value, "Word to search in the .eds file.")->required();
    edsModifyAdd = edsModify->add_subcommand("add", "Add a new object to the .eds file.");
    edsModifyDel = edsModify->add_subcommand("del", "Delete an object from the .eds file.");
    edsModifyCorrection =
        edsModify->add_subcommand("correction", "Correct an object from the .eds file.");
    edsModifyAdd->add_option("--index", cmdCANopen.edsObj.index, "Index of the parameter to add.")
        ->required();
    edsModifyAdd
        ->add_option("--subindex", cmdCANopen.edsObj.subIndex, "Subindex of the parameter to add.")
        ->required();
    edsModifyAdd
        ->add_option("--name", cmdCANopen.edsObj.ParameterName, "Name of the parameter to add.")
        ->required();
    edsModifyAdd->add_option(
        "--StorageLocation",
        cmdCANopen.edsObj.StorageLocation,
        "Storage location of the parameter to add(e.g.RAM,PERSIST_COMM,etc.).");
    edsModifyAdd->add_option(
        "--DataType",
        cmdCANopen.edsObj.DataType,
        "Data type of the parameter to add (e.g. 0x0001=BOOLEAN, 0x0007=UNSIGNED32, etc.).");
    edsModifyAdd->add_option(
        "--DefaultValue", cmdCANopen.edsObj.defaultValue, "Default value of the parameter to add.");
    edsModifyAdd->add_option("--AccessType",
                             cmdCANopen.edsObj.accessType,
                             "Access type of the parameter to add (e.g. R, RW, etc.).");
    edsModifyAdd->add_option("--PdoMapping",
                             cmdCANopen.edsObj.PDOMapping,
                             "PDOMapping of the parameter to add (0=no, 1=yes).");
    edsModifyAdd->add_option(
        "--ObjectType",
        cmdCANopen.edsObj.sectionType,
        "Object type of the parameter to add (0=Mandatory, 1=Optional, 2=Manufacturer).");
    edsModifyDel
        ->add_option("--index", cmdCANopen.edsObj.index, "Index of the parameter to delete.")
        ->required();
    edsModifyDel
        ->add_option(
            "--subindex", cmdCANopen.edsObj.subIndex, "Subindex of the parameter to delete.")
        ->required();
    edsModifyCorrection->add_option("--index", cmdCANopen.id, "Index of the parameter to correct.")
        ->required();
    edsModifyCorrection
        ->add_option("--subindex", cmdCANopen.subReg, "Subindex of the parameter to correct.")
        ->required();
    edsModifyCorrection
        ->add_option(
            "--new_index", cmdCANopen.edsObj.index, "New index of the parameter to correct.")
        ->required();
    edsModifyCorrection
        ->add_option("--new_subindex",
                     cmdCANopen.edsObj.subIndex,
                     "New subindex of the parameter to correct.")
        ->required();
    edsModifyCorrection
        ->add_option(
            "--new_name", cmdCANopen.edsObj.ParameterName, "New name of the parameter to correct.")
        ->required();
    edsModifyCorrection->add_option(
        "--NewStorageLocation",
        cmdCANopen.edsObj.StorageLocation,
        "New storage location of the parameter to correct (e.g.RAM,PERSIST_COMM,etc.).");
    edsModifyCorrection->add_option("--NewDataType",
                                    cmdCANopen.edsObj.DataType,
                                    "New data type of the parameter to correct (e.g. "
                                    "0x0005=UNSIGNED8, 0x0007=UNSIGNED32, etc.).");
    edsModifyCorrection->add_option("--NewDefaultValue",
                                    cmdCANopen.edsObj.defaultValue,
                                    "New default value of the parameter to correct.");
    edsModifyCorrection->add_option(
        "--NewAccessType",
        cmdCANopen.edsObj.accessType,
        "New access type of the parameter to correct (e.g. R, RW, etc.).");
    edsModifyCorrection->add_option("--NewPdoMapping",
                                    cmdCANopen.edsObj.PDOMapping,
                                    "New PDOMapping of the parameter to correct (0=no, 1=yes).");
    edsModifyCorrection->add_option(
        "--ObjectType",
        cmdCANopen.edsObj.sectionType,
        "Object type of the parameter to add (0=Mandatory, 1=Optional, 2=Manufacturer).");

    // ENCODER CANopen
    encoder->add_option("--id", cmdCANopen.id, "CAN ID of the MD to interact with.")->required();
    testEncoder     = encoder->add_subcommand("test", "Perform accuracy test on encoder.");
    testEncoderMain = testEncoder->add_subcommand("main", "Perform test routine on main encoder.");
    testEncoderOut =
        testEncoder->add_subcommand("output", "Perform test routine on output encoder.");

    // Heartbeat
    heartbeat
        ->add_option(
            "--master_id", cmdCANopen.id, "CAN ID of the master who will produce the heartbeat.")
        ->required();
    heartbeat
        ->add_option("--slave_id",
                     cmdCANopen.newId,
                     "CAN ID of the slave who will follow the heartbeat (A MD must be attach).")
        ->required();
    heartbeat->add_option("-t,--timeout",
                          cmdCANopen.HeartbeatTimeout,
                          "Duration after which the engine goes into preoperational state if it "
                          "has not received a heartbeat from the master node [ms].");

    /// NMT CANopen
    nmtRead = nmt->add_subcommand("read_state", "Read the NMT state of the MD drive.");
    nmtOperational =
        nmt->add_subcommand("go_to_operational", "Send command to go to pre-operational state.");
    nmtStop = nmt->add_subcommand("stopped", "Send command to go to stop state.");
    nmtPreOperational =
        nmt->add_subcommand("pre_operational", "Send command to go to pre-operational state.");
    nmtResetNode = nmt->add_subcommand("reset_node", "Reset the node.");
    nmtResetCommunication =
        nmt->add_subcommand("reset_communication", "Reset the node communication.");
    nmtRead->add_option("--id", cmdCANopen.id, "CAN ID of the MD to interact with.")->required();
    nmtOperational->add_option("--id", cmdCANopen.id, "CAN ID of the MD to interact with.")
        ->required();
    nmtStop->add_option("--id", cmdCANopen.id, "CAN ID of the MD to interact with.")->required();
    nmtPreOperational->add_option("--id", cmdCANopen.id, "CAN ID of the MD to interact with.")
        ->required();
    nmtResetNode->add_option("--id", cmdCANopen.id, "CAN ID of the MD to interact with.")
        ->required();
    nmtResetCommunication->add_option("--id", cmdCANopen.id, "CAN ID of the MD to interact with.")
        ->required();

    // PING CANopen
    ping->add_option("<CAN_BAUD>", cmdCANopen.variant, "Can be one of: 1M, 500K Default: all.");

    // REGISTER CANopen
    regRead  = registr->add_subcommand("read", "Read MD register.");
    regWrite = registr->add_subcommand("write", "Write MD register.");

    regRead->add_option("--id", cmdCANopen.id, "CAN ID of the MD to interact with.")->required();
    regRead->add_option("--index", cmdCANopen.reg, "Register ID (offset) to read data from.")
        ->required();
    regRead->add_option("--subindex", cmdCANopen.subReg, "Register ID (offset) to read data from.")
        ->required();
    regRead
        ->add_flag("-f,--force",
                   cmdCANopen.force,
                   "Force reading message, without verification in the Object dictionary.")
        ->default_val(false);
    regWrite->add_option("--id", cmdCANopen.id, "CAN ID of the MD to interact with.")->required();
    regWrite->add_option("--index", cmdCANopen.reg, "Register ID (offset) to write data to.")
        ->required();
    regWrite
        ->add_option("--subindex",
                     cmdCANopen.subReg,
                     "Register ID (offset) to read data from. Mandatory for CANopen.")
        ->required();
    regWrite->add_option("--value", cmdCANopen.value, "Value to write.")->required();
    regWrite->add_option("--dataSize",
                         cmdCANopen.dataSize,
                         "Size of data to be sent. {1,2,4}[bytes]. Mandatory for CANopen.");
    regWrite
        ->add_flag("-f,--force",
                   cmdCANopen.force,
                   "Force reading message, without verification in the Object dictionary.")
        ->default_val(false);

    // RESET CANopen
    reset->add_option("--id", cmdCANopen.id, "CAN ID of the MD to interact with.")->required();

    // PDOTEST CANopen
    PdoSpeed    = pdoTest->add_subcommand("Speed", "Perform a speed loop.");
    PdoPosition = pdoTest->add_subcommand("Position", "Perform a position loop.");
    PdoCustom   = pdoTest->add_subcommand("Custom", "Allow the user to send custom PDOs.");
    PdoSpeed->add_option("--id", cmdCANopen.id, "CAN ID of the MD to interact with.")->required();
    PdoSpeed->add_option("--speed", cmdCANopen.desiredSpeed, "Desired motor speed [RPM].")
        ->required();
    PdoPosition->add_option("--id", cmdCANopen.id, "CAN ID of the MD to interact with.")
        ->required();
    PdoPosition->add_option("--position", cmdCANopen.desiredSpeed, "Desired motor position [inc].")
        ->required();
    PdoCustom->add_option("--id", cmdCANopen.id, "CAN ID of the MD to interact with.")->required();
    PdoCustom->add_option("--index", cmdCANopen.edsObj.index, "Index of the PDO.")->required();
    PdoCustom->add_option("--data", cmdCANopen.value, "Data message to send.")->required();

    // SETUP CANopen
    setupCalib    = setup->add_subcommand("calibration", "Calibrate main MD encoder.");
    setupCalibOut = setup->add_subcommand("calibration_out", "Calibrate output encoder.");
    setupInfo     = setup->add_subcommand("info", "Display info about the MD drive.");
    setupupload   = setup->add_subcommand("upload", "Upload actuator config from .cfg file.");
    setupdownload =
        setup->add_subcommand("download", "Download actuator config from MD to .cfg file.");
    setupInfoAllFlag = setupInfo->add_flag("-a", "Print ALL available info.");

    setupCalib->add_option("--id", cmdCANopen.id, "CAN ID of the MD to interact with.")->required();
    setupCalibOut->add_option("--id", cmdCANopen.id, "CAN ID of the MD to interact with.")
        ->required();
    setupInfo->add_option("--id", cmdCANopen.id, "CAN ID of the MD to interact with.")->required();
    setupupload->add_option("--id", cmdCANopen.id, "CAN ID of the MD to interact with.")
        ->required();
    setupupload->add_option("--file_path",
                            cmdCANopen.cfgPath,
                            "Filename of motor config. Default config files are "
                            "in:`/etc/candletool/config/motors/`.");
    setupupload->add_flag(
        "-f,--force", cmdCANopen.force, "Force uploading config file, without verification.");
    setupdownload->add_option("--id", cmdCANopen.id, "CAN ID of the MD to interact with.")
        ->required();
    setupdownload->add_option("--path", cmdCANopen.value, "File to save config to.")->required();

    // SDOsegmented CANopen
    SDOsegmentedRead = SDOsegmented->add_subcommand(
        "read", "Read a value from a register with more than 4 bytes of data.");
    SDOsegmentedWrite = SDOsegmented->add_subcommand(
        "write", "write a value in a register with more than 4 bytes of data.");
    SDOsegmentedRead->add_option("--id", cmdCANopen.id, "CAN ID of the MD to interact with.")
        ->required();
    SDOsegmentedRead
        ->add_option("--index", cmdCANopen.reg, "Register ID (offset) to read data from.")
        ->required();
    SDOsegmentedRead
        ->add_option("--subindex", cmdCANopen.subReg, "Register ID (offset) to read data from.")
        ->required();
    SDOsegmentedWrite->add_option("--id", cmdCANopen.id, "CAN ID of the MD to interact with.")
        ->required();
    SDOsegmentedWrite
        ->add_option("--index", cmdCANopen.reg, "Register ID (offset) to read data from.")
        ->required();
    SDOsegmentedWrite
        ->add_option("--subindex", cmdCANopen.subReg, "Register ID (offset) to read data from.")
        ->required();
    SDOsegmentedWrite->add_option("--data", cmdCANopen.value, "Value to write into the register.")
        ->required();

    // Sync
    Sync->add_option("--id", cmdCANopen.id, "CAN ID of the MD to interact with.")->required();

    // TEST CANopen
    testLatency = test->add_subcommand(
        "latency",
        "Test max data exchange rate between your computer and all MD connected  drives.");
    testMove      = test->add_subcommand("move", "Validate if motor can move.");
    testMoveAbs   = testMove->add_subcommand("absolute", "Move motor to absolute position.");
    testMoveRel   = testMove->add_subcommand("relative", "Move motor to relative position.");
    testMoveSpeed = testMove->add_subcommand("speed", "Move motor to desired speed.");
    testImpedance = testMove->add_subcommand(
        "impedance",
        "Put the motor into Impedance PD mode "
        "cf:https://mabrobotics.github.io/MD80-x-CANdle-Documentation/"
        "md_x_candle_ecosystem_overview/Motion%20modes.html#impedance-pd.");
    testLatency->add_option("--id", cmdCANopen.id, "CAN ID of the MD to interact with.")
        ->required();
    testMoveAbs->add_option("--id", cmdCANopen.id, "CAN ID of the MD to interact with.")
        ->required();
    testMoveAbs->add_option("--postion", cmdCANopen.desiredPos, "Absolute position to reach [rad].")
        ->required();
    testMoveAbs->add_option(
        "--MaxVelocity", cmdCANopen.param.MaxSpeed, "Profile max velocity [rad/s].");
    testMoveAbs->add_option(
        "--MaxAcceleration", cmdCANopen.param.accLimit, "Profile max acceleration [rad/s^2].");
    testMoveAbs->add_option(
        "--MaxDeceleration", cmdCANopen.param.dccLimit, "Profile max deceleration [rad/s^2].");
    testMoveAbs->add_option(
        "--MaxCurrent",
        cmdCANopen.param.MaxCurrent,
        "Configures the maximum allowed phase current in the motor. The value is "
        "expressed in permille of rated current. Example: rated current of the motor "
        "is 15A, and the maximum is 30A. The Max Current object should equal to 2000.");
    testMoveAbs->add_option(
        "--RatedCurrent",
        cmdCANopen.param.RatedCurrent,
        "Configures the maximum allowed torque in the motor. The value is expressed "
        "in permille of rated torque. Example: rated torque of the motor is 1Nm, and "
        "the maximum is 2Nm. The Max Torque object should equal to 2000.");
    testMoveAbs->add_option(
        "--MaxTorque",
        cmdCANopen.param.MaxTorque,
        "Configures the maximum allowed phase current in the motor. The value is "
        "expressed in permille of rated current. Example: rated current of the motor "
        "is 15A, and the maximum is 30A. The Max Current object should equal to 2000.");
    testMoveAbs->add_option(
        "--RatedTorque",
        cmdCANopen.param.RatedTorque,
        "Configures the motor rated torque expressed in mNm. This object is a "
        "reference for parameters such as 0x6072 Max Torque. The value should be "
        "taken from the motor's datasheet.");
    testMoveRel->add_option("--id", cmdCANopen.id, "CAN ID of the MD to interact with.")
        ->required();
    testMoveRel
        ->add_option("--position",
                     cmdCANopen.desiredPos,
                     "Relative position to reach.<0x0, "
                     "0xFFFFFFFF>[inc] ")
        ->required();
    testMoveRel->add_option(
        "--MaxSpeed",
        cmdCANopen.param.MaxSpeed,
        "Configures the target velocity for the profile position mode. If this value is "
        "greater than 0x6080 Max Motor Speed, it will be limited to Max Motor Speed.");
    testMoveRel->add_option(
        "--MaxCurrent",
        cmdCANopen.param.MaxCurrent,
        "Configures the maximum allowed phase current in the motor. The value is "
        "expressed in permille of rated current. Example: rated current of the motor "
        "is 15A, and the maximum is 30A. The Max Current object should equal to 2000.");
    testMoveRel->add_option(
        "--RatedCurrent",
        cmdCANopen.param.RatedCurrent,
        "Configures the maximum allowed torque in the motor. The value is expressed "
        "in permille of rated torque. Example: rated torque of the motor is 1Nm, and "
        "the maximum is 2Nm. The Max Torque object should equal to 2000.");
    testMoveRel->add_option(
        "--MaxTorque",
        cmdCANopen.param.MaxTorque,
        "Configures the maximum allowed phase current in the motor. The value is "
        "expressed in permille of rated current. Example: rated current of the motor "
        "is 15A, and the maximum is 30A. The Max Current object should equal to 2000.");
    testMoveRel->add_option(
        "--RatedTorque",
        cmdCANopen.param.RatedTorque,
        "Configures the motor rated torque expressed in mNm. This object is a "
        "reference for parameters such as 0x6072 Max Torque. The value should be "
        "taken from the motor's datasheet.");
    testMoveSpeed->add_option("--id", cmdCANopen.id, "CAN ID of the MD to interact with.")
        ->required();
    testMoveSpeed
        ->add_option(
            "--speed", cmdCANopen.desiredSpeed, "Sets the target velocity for all motion modes.")
        ->required();
    testMoveSpeed->add_option(
        "--MaxCurrent",
        cmdCANopen.param.MaxCurrent,
        "Configures the maximum allowed phase current in the motor. The value is "
        "expressed in permille of rated current. Example: rated current of the motor "
        "is 15A, and the maximum is 30A. The Max Current object should equal to 2000.");
    testMoveSpeed->add_option(
        "--RatedCurrent",
        cmdCANopen.param.RatedCurrent,
        "Configures the maximum allowed torque in the motor. The value is expressed "
        "in permille of rated torque. Example: rated torque of the motor is 1Nm, and "
        "the maximum is 2Nm. The Max Torque object should equal to 2000.");
    testMoveSpeed->add_option(
        "--MaxTorque",
        cmdCANopen.param.MaxTorque,
        "Configures the maximum allowed phase current in the motor. The value is "
        "expressed in permille of rated current. Example: rated current of the motor "
        "is 15A, and the maximum is 30A. The Max Current object should equal to 2000.");
    testMoveSpeed->add_option(
        "--RatedTorque",
        cmdCANopen.param.RatedTorque,
        "Configures the motor rated torque expressed in mNm. This object is a "
        "reference for parameters such as 0x6072 Max Torque. The value should be "
        "taken from the motor's datasheet.");
    testMoveSpeed->add_option(
        "--MaxSpeed",
        cmdCANopen.param.MaxSpeed,
        "Configures the target velocity for the profile position mode. If this value is "
        "greater than 0x6080 Max Motor Speed, it will be limited to Max Motor Speed.");
    testImpedance->add_option("--id", cmdCANopen.id, "CAN ID of the MD to interact with.")
        ->required();
    testImpedance
        ->add_option(
            "--speed", cmdCANopen.desiredSpeed, "Sets the target velocity for all motion modes.")
        ->required();
    testImpedance
        ->add_option("--positon",
                     cmdCANopen.desiredPos,
                     "Relative position to reach. <0x0, "
                     "0xFFFFFFFF>[inc].")
        ->required();
    testImpedance->add_option("--kp", cmdCANopen.param.kp, "Position gain.")->required();
    testImpedance->add_option("--kd", cmdCANopen.param.kd, "Velocity gain.")->required();
    testImpedance->add_option("--torque_ff", cmdCANopen.param.torqueff, "Torque FF.")->required();
    testImpedance->add_option(
        "--MaxSpeed",
        cmdCANopen.param.MaxSpeed,
        "Configures the target velocity for the profile position mode. If this value is "
        "greater than 0x6080 Max Motor Speed, it will be limited to Max Motor Speed.");
    testImpedance->add_option(
        "--MaxCurrent",
        cmdCANopen.param.MaxCurrent,
        "Configures the maximum allowed phase current in the motor. The value is "
        "expressed in permille of rated current. Example: rated current of the motor "
        "is 15A, and the maximum is 30A. The Max Current object should equal to 2000.");
    testImpedance->add_option(
        "--RatedCurrent",
        cmdCANopen.param.RatedCurrent,
        "Configures the maximum allowed torque in the motor. The value is expressed "
        "in permille of rated torque. Example: rated torque of the motor is 1Nm, and "
        "the maximum is 2Nm. The Max Torque object should equal to 2000.");
    testImpedance->add_option(
        "--MaxTorque",
        cmdCANopen.param.MaxTorque,
        "Configures the maximum allowed phase current in the motor. The value is "
        "expressed in permille of rated current. Example: rated current of the motor "
        "is 15A, and the maximum is 30A. The Max Current object should equal to 2000.");
    testImpedance->add_option(
        "--RatedTorque",
        cmdCANopen.param.RatedTorque,
        "Configures the motor rated torque expressed in mNm. This object is a "
        "reference for parameters such as 0x6072 Max Torque. The value should be "
        "taken from the motor's datasheet.");

    // TIME STAMP
    timeStamp->add_option("--id", cmdCANopen.id, "CAN ID of the MD to interact with.")->required();
}

void MdcoCli::parse(mab::CANdleDatarate_E baud)
{
    if (!mdco->parsed())
        return;
    if (mdco->parsed())
    {
        if (m_candleBuilder->preBuildTask)
        {
            m_candleBuilder->preBuildTask();
        }
        else
        {
            m_log.error("Impossible to have user bus choice");
        }
        CandleToolCO candleToolCO(baud, *(m_candleBuilder->busType));
        if (blink->parsed())
            candleToolCO.blink(cmdCANopen.id);
        if (clear->parsed())
            candleToolCO.clearErrors(cmdCANopen.id, cmdCANopen.variant);
        if (can->parsed())
        {
            candleToolCO.configCan(cmdCANopen.id,
                                   cmdCANopen.newId,
                                   cmdCANopen.baud,
                                   cmdCANopen.canWatchdog,
                                   cmdCANopen.save);
        }
        if (encoder->parsed())
        {
            candleToolCO.encoder(cmdCANopen.id);
            if (testEncoderMain->parsed())
                candleToolCO.testEncoderMain(cmdCANopen.id);
            if (testEncoderOut->parsed())
                candleToolCO.testEncoderOutput(cmdCANopen.id);
        }

        if (eds->parsed())
        {
            if (edsLoad->parsed())
                candleToolCO.edsLoad(cmdCANopen.value);
            if (edsUnload->parsed())
                candleToolCO.edsUnload();
            if (edsDisplay->parsed())
                candleToolCO.edsDisplay();
            if (edsGen->parsed())
            {
                if (edsGenMd->parsed())
                    candleToolCO.edsGenerateMarkdown(cmdCANopen.cfgPath);  // generate .md file
                if (edsGenHtml->parsed())
                    candleToolCO.edsGenerateHtml(cmdCANopen.cfgPath);  // generate .html file
                if (edsGencpp->parsed())
                    candleToolCO.edsGenerateCpp(cmdCANopen.cfgPath);  // generate .cpp file
            }
            if (edsGet->parsed())
            {
                candleToolCO.edsGet(cmdCANopen.id, cmdCANopen.subReg);
            }
            if (edsFind->parsed())
            {
                candleToolCO.edsFind(cmdCANopen.value);
            }
            if (edsModify->parsed())
            {
                if (edsModifyAdd->parsed())
                {
                    candleToolCO.edsAddObject(cmdCANopen.edsObj);
                }

                if (edsModifyDel->parsed())
                    candleToolCO.edsDeleteObject(cmdCANopen.edsObj.index,
                                                 cmdCANopen.edsObj.subIndex);
                if (edsModifyCorrection->parsed())
                {
                    candleToolCO.edsModifyCorrection(
                        cmdCANopen.edsObj, cmdCANopen.id, cmdCANopen.subReg);
                }
            }
        }
        if (nmt->parsed())
        {
            if (nmtRead->parsed())
                candleToolCO.ReadHeartbeat(cmdCANopen.id);
            if (nmtOperational->parsed())
                candleToolCO.SendNMT(cmdCANopen.id, 0x01);
            if (nmtStop->parsed())
                candleToolCO.SendNMT(cmdCANopen.id, 0x02);
            if (nmtPreOperational->parsed())
                candleToolCO.SendNMT(cmdCANopen.id, 0x80);
            if (nmtResetNode->parsed())
                candleToolCO.SendNMT(cmdCANopen.id, 0x81);
            if (nmtResetCommunication->parsed())
                candleToolCO.SendNMT(cmdCANopen.id, 0x82);
        }
        if (ping->parsed())
            candleToolCO.ping(cmdCANopen.variant);
        if (registr->parsed())
        {
            u16 reg    = strtoul(cmdCANopen.reg.c_str(), nullptr, 16);
            u8  subreg = (u8)cmdCANopen.subReg;

            if (regRead->parsed())
                candleToolCO.registerRead(cmdCANopen.id, reg, subreg, cmdCANopen.force);
            if (regWrite->parsed())
                candleToolCO.registerWrite(cmdCANopen.id,
                                           reg,
                                           cmdCANopen.value,
                                           subreg,
                                           cmdCANopen.dataSize,
                                           cmdCANopen.force);
        }
        if (reset->parsed())
            candleToolCO.reset(cmdCANopen.id);
        if (pdoTest->parsed())
        {
            if (PdoSpeed->parsed())
                candleToolCO.sendPdoSpeed(cmdCANopen.id, cmdCANopen.desiredSpeed);
            if (PdoPosition->parsed())
                candleToolCO.sendPdoPosition(cmdCANopen.id, cmdCANopen.desiredSpeed);
            if (PdoCustom->parsed())
            {
                unsigned long long data = strtoul((cmdCANopen.value.c_str()), nullptr, 16);
                candleToolCO.SendCustomPdo(cmdCANopen.id, cmdCANopen.edsObj, data);
            }
        }
        if (setup->parsed())
        {
            if (setupCalib->parsed())
                candleToolCO.setupCalibration(cmdCANopen.id);
            if (setupCalibOut->parsed())
                candleToolCO.setupCalibrationOutput(cmdCANopen.id);
            if (setupInfo->parsed())
                candleToolCO.setupInfo(cmdCANopen.id,
                                       (setupInfoAllFlag->count() > 0 ? true : false));
            if (setupupload->parsed())
                candleToolCO.setupMotor(cmdCANopen.id, cmdCANopen.cfgPath, cmdCANopen.force);
            if (setupdownload->parsed())
                candleToolCO.setupReadConfig(cmdCANopen.id, cmdCANopen.value);
        }
        // Sync CANopen
        if (Sync->parsed())
        {
            candleToolCO.SendSync(cmdCANopen.id);
        }
        if (test->parsed())
        {
            if (testLatency->parsed())
                candleToolCO.testLatency(cmdCANopen.id);
            if (testMove->parsed())
            {
                if (testMoveAbs->parsed())
                {
                    candleToolCO.testMoveAbsolute(
                        cmdCANopen.id, cmdCANopen.desiredPos, cmdCANopen.param);
                }
                if (testMoveRel->parsed())
                    candleToolCO.testMove(cmdCANopen.id, cmdCANopen.desiredPos, cmdCANopen.param);

                if (testMoveSpeed->parsed())
                    candleToolCO.testMoveSpeed(
                        cmdCANopen.id, cmdCANopen.param, cmdCANopen.desiredSpeed);
                if (testImpedance->parsed())
                    candleToolCO.testMoveImpedance(cmdCANopen.id,
                                                   cmdCANopen.desiredSpeed,
                                                   cmdCANopen.desiredPos,
                                                   cmdCANopen.param);
            }
        }
        if (heartbeat->parsed())
        {
            candleToolCO.heartbeatTest(
                cmdCANopen.id, cmdCANopen.newId, cmdCANopen.HeartbeatTimeout);
        }
        if (SDOsegmented->parsed())
        {
            if (SDOsegmentedRead->parsed())
            {
                u16 reg = strtoul(cmdCANopen.reg.c_str(), nullptr, 16);
                candleToolCO.SDOsegmentedRead(cmdCANopen.id, reg, cmdCANopen.subReg);
            }
            if (SDOsegmentedWrite->parsed())
            {
                u16 reg = strtoul(cmdCANopen.reg.c_str(), nullptr, 16);
                candleToolCO.SDOsegmentedWrite(
                    cmdCANopen.id, reg, cmdCANopen.subReg, cmdCANopen.value);
            }
        }
        if (timeStamp->parsed())
        {
            candleToolCO.SendTime(cmdCANopen.id);
        }
    }
}
