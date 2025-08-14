#include "mdco_cli.hpp"
#include "CLI/CLI.hpp"
#include "logger.hpp"
#include "candletoolCO.hpp"
#include "MDCO.hpp"

using namespace mab;

UserCommandCO cmdCANopen;

MdcoCli::MdcoCli(CLI::App& rootCli) : m_rootCli(rootCli)
{
    m_log.m_tag   = "MDCO";
    m_log.m_layer = Logger::ProgramLayer_E::TOP;

    mdco = m_rootCli.add_subcommand("mdco", "Send CANopen command instead of CAN FD.");

    // CANopen command
    blinkco     = mdco->add_subcommand("blink", "Blink LEDs on MD drive.");
    clearco     = mdco->add_subcommand("clear", "Clear Errors and Warnings.");
    configco    = mdco->add_subcommand("config", "Configure CAN parameters of MD drive.");
    eds         = mdco->add_subcommand("eds", "EDS file analizer and generator.");
    encoderco   = mdco->add_subcommand("encoder", "Display MD rotor position.");
    heartbeatco = mdco->add_subcommand("heartbeat", "Test heartbeat detection of a device.");
    nmt         = mdco->add_subcommand("nmt", "Send NMT commands.");
    pdoTestco   = mdco->add_subcommand("pdo", "Try to send pdo can frame instead of sdo.");
    pingco      = mdco->add_subcommand("ping", "Discover MD drives on CAN bus.");
    registrco   = mdco->add_subcommand("register", "Access MD drive via register read/write.");
    resetco     = mdco->add_subcommand("reset", "Reset MD drive.");
    setupco     = mdco->add_subcommand("setup", "Setup MD via config files, and calibrate.");
    Sync        = mdco->add_subcommand("sync", "Send a sync CANopen message.");
    SDOsegmentedco =
        mdco->add_subcommand("segmented", "Send a message using SDO segmented transfer.");
    testco = mdco->add_subcommand("test", "Basic MD drive testing routines.");
    timeStamp =
        mdco->add_subcommand("time", "Send a time stamp message using the computer's clock.");

    // BLINK CANopen
    blinkco->add_option("<CAN_ID>", cmdCANopen.id, "CAN ID of the MD to interact with.")
        ->required();
    // CLEAR
    clearco->add_option("<CAN_ID>", cmdCANopen.id, "CAN ID of the MD to interact with.")
        ->required();
    clearco->add_option("<LEVEL>", cmdCANopen.variant, "Can be: warning, error, all.")->required();

    // CONFIG CANopen
    configBandco = configco->add_subcommand(
        "bandwidth", "Set the torque bandwidth without recalibrating the actuator.");
    configCanco  = configco->add_subcommand("can", "Set CAN parameters of MD drive.");
    configSaveco = configco->add_subcommand("save", "Save current config to MD flash memory.");
    configZeroco = configco->add_subcommand("zero", "Set MD zero position at current position.");

    configBandco->add_option("<CAN_ID>", cmdCANopen.id, "CAN ID of the MD to interact with.")
        ->required();
    configBandco->add_option("<HZ>", cmdCANopen.bandwidth, "Desired torque bandwidth.")->required();
    configCanco->add_option("<CAN_ID>", cmdCANopen.id, "Current CAN ID.")->required();
    configCanco->add_option("<NEW_CAN_ID>", cmdCANopen.newId, "New CAN ID to set.")->required();
    configCanco
        ->add_option("<CAN_BAUD>", cmdCANopen.baud, "New CAN baudrate to set: 1M, 2M, 5M or 8M.")
        ->required();
    configCanco
        ->add_option(
            "<CAN_WDG>", cmdCANopen.canWatchdog, "New CAN watchdog timeout (in ms) to set.")
        ->required();
    configSaveco->add_option("<CAN_ID>", cmdCANopen.id, "CAN ID of the MD to interact with.")
        ->required();
    configZeroco->add_option("<CAN_ID>", cmdCANopen.id, "CAN ID of the MD to interact with.")
        ->required();

    // ENCODER CANopen
    encoderco->add_option("<CAN_ID>", cmdCANopen.id, "CAN ID of the MD to interact with.")
        ->required();

    // EDS CANopen
    edsLoad    = eds->add_subcommand("load", "Load the path to the .eds file to analyze.");
    edsUnload  = eds->add_subcommand("unload", "Delete the path to the .eds file.");
    edsDisplay = eds->add_subcommand("display", "Display the content of the .eds file.");
    edsGen     = eds->add_subcommand("generate", "Generate documentation from the .eds file.");
    edsGet  = eds->add_subcommand("get", "Display the content of an object from his index number.");
    edsFind = eds->add_subcommand("find", "Find and display the content of an object from words.");
    edsModify = eds->add_subcommand("modify", "Modify the actual .eds file.");

    edsLoad->add_option("<EDS_FILE>", cmdCANopen.value, "Path to the .eds file to analyze.")
        ->required();
    edsGenMd   = edsGen->add_subcommand("md", "Generate .md file from .eds file.");
    edsGenHtml = edsGen->add_subcommand("html", "Generate .html file from .eds file.");
    edsGencpp  = edsGen->add_subcommand("cpp", "Generate .cpp & .hpp file from .eds file.");
    edsGet->add_option("<INDEX>", cmdCANopen.id, "Index of the object to get.")->required();
    edsGet->add_option("<SUBINDEX>", cmdCANopen.subReg, "Subindex of the object to get.")
        ->required();
    edsFind->add_option("<WORD>", cmdCANopen.value, "Word to search in the .eds file.")->required();

    edsModifyAdd = edsModify->add_subcommand("add", "Add a new object to the .eds file.");
    edsModifyDel = edsModify->add_subcommand("del", "Delete an object from the .eds file.");
    edsModifyCorrection =
        edsModify->add_subcommand("correction", "Correct an object from the .eds file.");
    edsModifyAdd->add_option("<INDEX>", cmdCANopen.edsObj.index, "Index of the parameter to add.")
        ->required();
    edsModifyAdd
        ->add_option("<SUBINDEX>", cmdCANopen.edsObj.subIndex, "Subindex of the parameter to add.")
        ->required();
    edsModifyAdd
        ->add_option("<NAME>", cmdCANopen.edsObj.ParameterName, "Name of the parameter to add.")
        ->required();
    edsModifyAdd->add_option(
        "StorageLocation",
        cmdCANopen.edsObj.StorageLocation,
        "Storage location of the parameter to add(e.g.RAM,PERSIST_COMM,etc.).");
    edsModifyAdd->add_option(
        "DataType",
        cmdCANopen.edsObj.DataType,
        "Data type of the parameter to add (e.g. 0x0001=BOOLEAN, 0x0007=UNSIGNED32, etc.).");
    edsModifyAdd->add_option(
        "DefaultValue", cmdCANopen.edsObj.defaultValue, "Default value of the parameter to add.");
    edsModifyAdd->add_option("AccessType",
                             cmdCANopen.edsObj.accessType,
                             "Access type of the parameter to add (e.g. R, RW, etc.).");
    edsModifyAdd->add_option("PdoMapping",
                             cmdCANopen.edsObj.PDOMapping,
                             "PDOMapping of the parameter to add (0=no, 1=yes).");
    edsModifyAdd->add_option(
        "ObjectType",
        cmdCANopen.edsObj.sectionType,
        "Object type of the parameter to add (0=Mandatory, 1=Optional, 2=Manufacturer).");
    edsModifyDel
        ->add_option("<INDEX>", cmdCANopen.edsObj.index, "Index of the parameter to delete.")
        ->required();
    edsModifyDel
        ->add_option(
            "<SUBINDEX>", cmdCANopen.edsObj.subIndex, "Subindex of the parameter to delete.")
        ->required();
    edsModifyCorrection->add_option("<INDEX>", cmdCANopen.id, "Index of the parameter to correct.")
        ->required();
    edsModifyCorrection
        ->add_option("<SUBINDEX>", cmdCANopen.subReg, "Subindex of the parameter to correct.")
        ->required();
    edsModifyCorrection
        ->add_option("NEW_INDEX", cmdCANopen.edsObj.index, "New index of the parameter to correct.")
        ->required();
    edsModifyCorrection
        ->add_option(
            "NEW_SUBINDEX", cmdCANopen.edsObj.subIndex, "New subindex of the parameter to correct.")
        ->required();
    edsModifyCorrection
        ->add_option(
            "NEW_NAME", cmdCANopen.edsObj.ParameterName, "New name of the parameter to correct.")
        ->required();
    edsModifyCorrection->add_option(
        "NewStorageLocation",
        cmdCANopen.edsObj.StorageLocation,
        "New storage location of the parameter to correct (e.g.RAM,PERSIST_COMM,etc.).");
    edsModifyCorrection->add_option("NewDataType",
                                    cmdCANopen.edsObj.DataType,
                                    "New data type of the parameter to correct (e.g. "
                                    "0x0005=UNSIGNED8, 0x0007=UNSIGNED32, etc.).");
    edsModifyCorrection->add_option("NewDefaultValue",
                                    cmdCANopen.edsObj.defaultValue,
                                    "New default value of the parameter to correct.");
    edsModifyCorrection->add_option(
        "NewAccessType",
        cmdCANopen.edsObj.accessType,
        "New access type of the parameter to correct (e.g. R, RW, etc.).");
    edsModifyCorrection->add_option("NewPdoMapping",
                                    cmdCANopen.edsObj.PDOMapping,
                                    "New PDOMapping of the parameter to correct (0=no, 1=yes).");
    edsModifyCorrection->add_option(
        "ObjectType",
        cmdCANopen.edsObj.sectionType,
        "Object type of the parameter to add (0=Mandatory, 1=Optional, 2=Manufacturer).");
    // Heartbeat
    heartbeatco
        ->add_option("<CAN_MASTER_ID>",
                     cmdCANopen.id,
                     "CAN ID of the master who will produce the heartbeat.")
        ->required();
    heartbeatco
        ->add_option("<CAN_SLAVE_ID>",
                     cmdCANopen.newId,
                     "CAN ID of the slave who will follow the heartbeat (A MD must be attach).")
        ->required();
    heartbeatco->add_option("-t,--timeout",
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

    nmtRead->add_option("<CAN_ID>", cmdCANopen.id, "CAN ID of the MD to interact with.")
        ->required();
    nmtOperational->add_option("<CAN_ID>", cmdCANopen.id, "CAN ID of the MD to interact with.")
        ->required();
    nmtStop->add_option("<CAN_ID>", cmdCANopen.id, "CAN ID of the MD to interact with.")
        ->required();
    nmtPreOperational->add_option("<CAN_ID>", cmdCANopen.id, "CAN ID of the MD to interact with.")
        ->required();
    nmtResetNode->add_option("<CAN_ID>", cmdCANopen.id, "CAN ID of the MD to interact with.")
        ->required();
    nmtResetCommunication
        ->add_option("<CAN_ID>", cmdCANopen.id, "CAN ID of the MD to interact with.")
        ->required();

    // PING CANopen
    pingco->add_option(
        "<CAN_BAUD>", cmdCANopen.variant, "Can be one of: 1M, 2M, 5M, 8M, all. Default: all.");

    // REGISTER CANopen
    regReadco  = registrco->add_subcommand("read", "Read MD register.");
    regWriteco = registrco->add_subcommand("write", "Write MD register.");

    regReadco->add_option("<CAN_ID>", cmdCANopen.id, "CAN ID of the MD to interact with.")
        ->required();
    regReadco->add_option("<REG_ID>", cmdCANopen.reg, "Register ID (offset) to read data from.")
        ->required();
    regReadco
        ->add_option("<REG_SUB_ID>", cmdCANopen.subReg, "Register ID (offset) to read data from.")
        ->required();
    regReadco
        ->add_flag("-f,--force",
                   cmdCANopen.force,
                   "Force reading message, without verification in the Object dictionary.")
        ->default_val(false);
    regWriteco->add_option("<CAN_ID>", cmdCANopen.id, "CAN ID of the MD to interact with.")
        ->required();
    regWriteco->add_option("<REG_ID>", cmdCANopen.reg, "Register ID (offset) to write data to.")
        ->required();
    regWriteco
        ->add_option("<REG_SUB_ID>",
                     cmdCANopen.subReg,
                     "Register ID (offset) to read data from. Mandatory for CANopen.")
        ->required();
    regWriteco->add_option("<VALUE>", cmdCANopen.value, "Value to write.")->required();

    regWriteco->add_option("dataSize",
                           cmdCANopen.dataSize,
                           "Size of data to be sent. {1,2,4}[bytes]. Mandatory for CANopen.");
    regWriteco
        ->add_flag("-f,--force",
                   cmdCANopen.force,
                   "Force reading message, without verification in the Object dictionary.")
        ->default_val(false);

    // RESET CANopen
    resetco->add_option("<CAN_ID>", cmdCANopen.id, "CAN ID of the MD to interact with.")
        ->required();

    // PDOTEST CANopen
    PdoSpeedco    = pdoTestco->add_subcommand("Speed", "Perform a speed loop.");
    PdoPositionco = pdoTestco->add_subcommand("Position", "Perform a position loop.");
    PdoCustomco   = pdoTestco->add_subcommand("Custom", "Allow the user to send custom PDOs.");
    PdoSpeedco->add_option("<CAN_ID>", cmdCANopen.id, "CAN ID of the MD to interact with.")
        ->required();
    PdoSpeedco->add_option("<SPEED>", cmdCANopen.desiredSpeed, "Desired motor speed [RPM].")
        ->required();
    PdoPositionco->add_option("<CAN_ID>", cmdCANopen.id, "CAN ID of the MD to interact with.")
        ->required();
    PdoPositionco
        ->add_option("<POSITION>", cmdCANopen.desiredSpeed, "Desired motor position [inc].")
        ->required();
    PdoCustomco->add_option("<CAN_ID>", cmdCANopen.id, "CAN ID of the MD to interact with.")
        ->required();
    PdoCustomco->add_option("<INDEX>", cmdCANopen.edsObj.index, "Index of the PDO.")->required();
    PdoCustomco->add_option("<DATA>", cmdCANopen.value, "Data message to send.")->required();

    // SETUP CANopen
    setupCalibco    = setupco->add_subcommand("calibration", "Calibrate main MD encoder.");
    setupCalibOutco = setupco->add_subcommand("calibration_out", "Calibrate output encoder.");
    setupInfoco     = setupco->add_subcommand("info", "Display info about the MD drive.");
    setupMotorco    = setupco->add_subcommand("motor", "Upload actuator config from .cfg file.");
    setupReadCfgco =
        setupco->add_subcommand("read_config", "Download actuator config from MD to .cfg file.");
    setupInfoAllFlagco = setupInfoco->add_flag("-a", "Print ALL available info.");

    setupCalibco->add_option("<CAN_ID>", cmdCANopen.id, "CAN ID of the MD to interact with.")
        ->required();
    setupCalibOutco->add_option("<CAN_ID>", cmdCANopen.id, "CAN ID of the MD to interact with.")
        ->required();
    setupInfoco->add_option("<CAN_ID>", cmdCANopen.id, "CAN ID of the MD to interact with.")
        ->required();
    setupMotorco->add_option("<CAN_ID>", cmdCANopen.id, "CAN ID of the MD to interact with.")
        ->required();
    setupMotorco->add_option("<.cfg_FILENAME>",
                             cmdCANopen.cfgPath,
                             "Filename of motor config. Default config files are "
                             "in:`/etc/candletool/config/motors/`.");
    setupMotorco->add_flag(
        "-f,--force", cmdCANopen.force, "Force uploading config file, without verification.");
    setupReadCfgco->add_option("<CAN_ID>", cmdCANopen.id, "CAN ID of the MD to interact with.")
        ->required();
    setupReadCfgco->add_option("<FILE>", cmdCANopen.value, "File to save config to.")->required();

    // SDOsegmented CANopen
    SDOsegmentedReadco = SDOsegmentedco->add_subcommand(
        "read", "Read a value from a register with more than 4 bytes of data.");
    SDOsegmentedWriteco = SDOsegmentedco->add_subcommand(
        "write", "write a value in a register with more than 4 bytes of data.");
    SDOsegmentedReadco->add_option("<CAN_ID>", cmdCANopen.id, "CAN ID of the MD to interact with.")
        ->required();
    SDOsegmentedReadco
        ->add_option("<REG_ID>", cmdCANopen.reg, "Register ID (offset) to read data from.")
        ->required();
    SDOsegmentedReadco
        ->add_option("<REG_SUB_ID>", cmdCANopen.subReg, "Register ID (offset) to read data from.")
        ->required();
    SDOsegmentedWriteco->add_option("<CAN_ID>", cmdCANopen.id, "CAN ID of the MD to interact with.")
        ->required();
    SDOsegmentedWriteco
        ->add_option("<REG_ID>", cmdCANopen.reg, "Register ID (offset) to read data from.")
        ->required();
    SDOsegmentedWriteco
        ->add_option("<REG_SUB_ID>", cmdCANopen.subReg, "Register ID (offset) to read data from.")
        ->required();
    SDOsegmentedWriteco->add_option("<DATA>", cmdCANopen.value, "Value to write into the register.")
        ->required();

    // Sync
    Sync->add_option("<CAN_ID>", cmdCANopen.id, "CAN ID of the MD to interact with.")->required();

    // TEST CANopen
    testEncoderco = testco->add_subcommand("encoder", "Perform accuracy test on encoder.");
    testEncoderMainco =
        testEncoderco->add_subcommand("main", "Perform test routine on main encoder.");
    testEncoderOutco =
        testEncoderco->add_subcommand("output", "Perform test routine on output encoder.");
    testLatencyco = testco->add_subcommand(
        "latency",
        "Test max data exchange rate between your computer and all MD connected  drives.");
    testMoveco      = testco->add_subcommand("move", "Validate if motor can move.");
    testMoveAbsco   = testMoveco->add_subcommand("absolute", "Move motor to absolute position.");
    testMoveRelco   = testMoveco->add_subcommand("relative", "Move motor to relative position.");
    testMoveSpeedco = testMoveco->add_subcommand("speed", "Move motor to desired speed.");
    testImpedanceco = testMoveco->add_subcommand(
        "impedance",
        "Put the motor into Impedance PD mode "
        "cf:https://mabrobotics.github.io/MD80-x-CANdle-Documentation/"
        "md_x_candle_ecosystem_overview/Motion%20modes.html#impedance-pd.");

    testEncoderMainco->add_option("<CAN_ID>", cmdCANopen.id, "CAN ID of the MD to interact with.")
        ->required();
    testEncoderOutco->add_option("<CAN_ID>", cmdCANopen.id, "CAN ID of the MD to interact with.")
        ->required();
    testLatencyco->add_option("<CAN_ID>", cmdCANopen.id, "CAN ID of the MD to interact with.")
        ->required();
    testMoveAbsco->add_option("<CAN_ID>", cmdCANopen.id, "CAN ID of the MD to interact with.")
        ->required();
    testMoveAbsco
        ->add_option("<ABS_POS>", cmdCANopen.desiredPos, "Absolute position to reach [rad].")
        ->required();
    testMoveAbsco->add_option(
        "MaxVelocity", cmdCANopen.param.MaxSpeed, "Profile max velocity [rad/s].");
    testMoveAbsco->add_option(
        "MaxAcceleration", cmdCANopen.param.accLimit, "Profile max acceleration [rad/s^2].");
    testMoveAbsco->add_option(
        "MaxDeceleration", cmdCANopen.param.dccLimit, "Profile max deceleration [rad/s^2].");
    testMoveAbsco->add_option(
        "MaxCurrent",
        cmdCANopen.param.MaxCurrent,
        "Configures the maximum allowed phase current in the motor. The value is "
        "expressed in permille of rated current. Example: rated current of the motor "
        "is 15A, and the maximum is 30A. The Max Current object should equal to 2000.");
    testMoveAbsco->add_option(
        "RatedCurrent",
        cmdCANopen.param.RatedCurrent,
        "Configures the maximum allowed torque in the motor. The value is expressed "
        "in permille of rated torque. Example: rated torque of the motor is 1Nm, and "
        "the maximum is 2Nm. The Max Torque object should equal to 2000.");
    testMoveAbsco->add_option(
        "MaxTorque",
        cmdCANopen.param.MaxTorque,
        "Configures the maximum allowed phase current in the motor. The value is "
        "expressed in permille of rated current. Example: rated current of the motor "
        "is 15A, and the maximum is 30A. The Max Current object should equal to 2000.");
    testMoveAbsco->add_option(
        "RatedTorque",
        cmdCANopen.param.RatedTorque,
        "Configures the motor rated torque expressed in mNm. This object is a "
        "reference for parameters such as 0x6072 Max Torque. The value should be "
        "taken from the motor's datasheet.");

    testMoveRelco->add_option("<CAN_ID>", cmdCANopen.id, "CAN ID of the MD to interact with.")
        ->required();
    testMoveRelco
        ->add_option("<REL_POS>",
                     cmdCANopen.desiredPos,
                     "Relative position to reach.<0x0, "
                     "0xFFFFFFFF>[inc] ")
        ->required();

    testMoveRelco->add_option(
        "MaxSpeed",
        cmdCANopen.param.MaxSpeed,
        "Configures the target velocity for the profile position mode. If this value is "
        "greater than 0x6080 Max Motor Speed, it will be limited to Max Motor Speed.");
    testMoveRelco->add_option(
        "MaxCurrent",
        cmdCANopen.param.MaxCurrent,
        "Configures the maximum allowed phase current in the motor. The value is "
        "expressed in permille of rated current. Example: rated current of the motor "
        "is 15A, and the maximum is 30A. The Max Current object should equal to 2000.");
    testMoveRelco->add_option(
        "RatedCurrent",
        cmdCANopen.param.RatedCurrent,
        "Configures the maximum allowed torque in the motor. The value is expressed "
        "in permille of rated torque. Example: rated torque of the motor is 1Nm, and "
        "the maximum is 2Nm. The Max Torque object should equal to 2000.");
    testMoveRelco->add_option(
        "MaxTorque",
        cmdCANopen.param.MaxTorque,
        "Configures the maximum allowed phase current in the motor. The value is "
        "expressed in permille of rated current. Example: rated current of the motor "
        "is 15A, and the maximum is 30A. The Max Current object should equal to 2000.");
    testMoveRelco->add_option(
        "RatedTorque",
        cmdCANopen.param.RatedTorque,
        "Configures the motor rated torque expressed in mNm. This object is a "
        "reference for parameters such as 0x6072 Max Torque. The value should be "
        "taken from the motor's datasheet.");

    testMoveSpeedco->add_option("<CAN_ID>", cmdCANopen.id, "CAN ID of the MD to interact with.")
        ->required();
    testMoveSpeedco
        ->add_option("<DESIRED_SPEED>",
                     cmdCANopen.desiredSpeed,
                     "Sets the target velocity for all motion modes.")
        ->required();
    testMoveSpeedco->add_option(
        "MaxCurrent",
        cmdCANopen.param.MaxCurrent,
        "Configures the maximum allowed phase current in the motor. The value is "
        "expressed in permille of rated current. Example: rated current of the motor "
        "is 15A, and the maximum is 30A. The Max Current object should equal to 2000.");
    testMoveSpeedco->add_option(
        "RatedCurrent",
        cmdCANopen.param.RatedCurrent,
        "Configures the maximum allowed torque in the motor. The value is expressed "
        "in permille of rated torque. Example: rated torque of the motor is 1Nm, and "
        "the maximum is 2Nm. The Max Torque object should equal to 2000.");
    testMoveSpeedco->add_option(
        "MaxTorque",
        cmdCANopen.param.MaxTorque,
        "Configures the maximum allowed phase current in the motor. The value is "
        "expressed in permille of rated current. Example: rated current of the motor "
        "is 15A, and the maximum is 30A. The Max Current object should equal to 2000.");
    testMoveSpeedco->add_option(
        "RatedTorque",
        cmdCANopen.param.RatedTorque,
        "Configures the motor rated torque expressed in mNm. This object is a "
        "reference for parameters such as 0x6072 Max Torque. The value should be "
        "taken from the motor's datasheet.");
    testMoveSpeedco->add_option(
        "MaxSpeed",
        cmdCANopen.param.MaxSpeed,
        "Configures the target velocity for the profile position mode. If this value is "
        "greater than 0x6080 Max Motor Speed, it will be limited to Max Motor Speed.");
    testImpedanceco->add_option("<CAN_ID>", cmdCANopen.id, "CAN ID of the MD to interact with.")
        ->required();
    testImpedanceco
        ->add_option("<DESIRED_SPEED>",
                     cmdCANopen.desiredSpeed,
                     "Sets the target velocity for all motion modes.")
        ->required();
    testImpedanceco
        ->add_option("<REL_POS>",
                     cmdCANopen.desiredPos,
                     "Relative position to reach. <0x0, "
                     "0xFFFFFFFF>[inc].")
        ->required();
    testImpedanceco->add_option("<KP>", cmdCANopen.param.kp, "Position gain.")->required();
    testImpedanceco->add_option("<KD>", cmdCANopen.param.kd, "Velocity gain.")->required();
    testImpedanceco->add_option("<TORQUE_FF>", cmdCANopen.param.torqueff, "Torque FF.")->required();
    testImpedanceco->add_option(
        "MaxSpeed",
        cmdCANopen.param.MaxSpeed,
        "Configures the target velocity for the profile position mode. If this value is "
        "greater than 0x6080 Max Motor Speed, it will be limited to Max Motor Speed.");
    testImpedanceco->add_option(
        "MaxCurrent",
        cmdCANopen.param.MaxCurrent,
        "Configures the maximum allowed phase current in the motor. The value is "
        "expressed in permille of rated current. Example: rated current of the motor "
        "is 15A, and the maximum is 30A. The Max Current object should equal to 2000.");
    testImpedanceco->add_option(
        "RatedCurrent",
        cmdCANopen.param.RatedCurrent,
        "Configures the maximum allowed torque in the motor. The value is expressed "
        "in permille of rated torque. Example: rated torque of the motor is 1Nm, and "
        "the maximum is 2Nm. The Max Torque object should equal to 2000.");
    testImpedanceco->add_option(
        "MaxTorque",
        cmdCANopen.param.MaxTorque,
        "Configures the maximum allowed phase current in the motor. The value is "
        "expressed in permille of rated current. Example: rated current of the motor "
        "is 15A, and the maximum is 30A. The Max Current object should equal to 2000.");
    testImpedanceco->add_option(
        "RatedTorque",
        cmdCANopen.param.RatedTorque,
        "Configures the motor rated torque expressed in mNm. This object is a "
        "reference for parameters such as 0x6072 Max Torque. The value should be "
        "taken from the motor's datasheet.");

    // TIME STAMP
    timeStamp->add_option("<CAN_ID>", cmdCANopen.id, "CAN ID of the MD to interact with.")
        ->required();
}

void MdcoCli::parse(mab::CANdleBaudrate_E baud)
{
    if (!mdco->parsed())
        return;
    if (mdco->parsed())
    {
        CandleToolCO candleToolCO(baud);
        if (blinkco->parsed())
            candleToolCO.blink(cmdCANopen.id);
        if (clearco->parsed())
            candleToolCO.clearErrors(cmdCANopen.id, cmdCANopen.variant);
        if (configco->parsed())
        {
            if (configBandco->parsed())
                candleToolCO.configBandwidth(cmdCANopen.id, cmdCANopen.bandwidth);
            if (configCanco->parsed())
                candleToolCO.configCan(
                    cmdCANopen.id, cmdCANopen.newId, cmdCANopen.baud, cmdCANopen.canWatchdog);
            if (configSaveco->parsed())
                candleToolCO.configSave(cmdCANopen.id);
            if (configZeroco->parsed())
                candleToolCO.configZero(cmdCANopen.id);
        }
        if (encoderco->parsed())
            candleToolCO.encoder(cmdCANopen.id);

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
                    candleToolCO.edsGenerateMarkdown();  // generate .md file
                if (edsGenHtml->parsed())
                    candleToolCO.edsGenerateHtml();  // generate .html file
                if (edsGencpp->parsed())
                    candleToolCO.edsGenerateCpp();  // generate .cpp file
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
                // candleToolCO.edsModify(cmdCANopen.value);
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
        if (pingco->parsed())
            candleToolCO.ping(cmdCANopen.variant);
        if (registrco->parsed())
        {
            u16 reg    = strtoul(cmdCANopen.reg.c_str(), nullptr, 16);
            u8  subreg = (u8)cmdCANopen.subReg;

            if (regReadco->parsed())
                candleToolCO.registerRead(cmdCANopen.id, reg, subreg, cmdCANopen.force);
            if (regWriteco->parsed())
                candleToolCO.registerWrite(cmdCANopen.id,
                                           reg,
                                           cmdCANopen.value,
                                           subreg,
                                           cmdCANopen.dataSize,
                                           cmdCANopen.force);
        }
        if (resetco->parsed())
            candleToolCO.reset(cmdCANopen.id);
        if (pdoTestco->parsed())
        {
            if (PdoSpeedco->parsed())
                candleToolCO.sendPdoSpeed(cmdCANopen.id, cmdCANopen.desiredSpeed);
            if (PdoPositionco->parsed())
                candleToolCO.sendPdoPosition(cmdCANopen.id, cmdCANopen.desiredSpeed);
            if (PdoCustomco->parsed())
            {
                unsigned long long data = strtoul((cmdCANopen.value.c_str()), nullptr, 16);
                candleToolCO.SendCustomPdo(cmdCANopen.id, cmdCANopen.edsObj, data);
            }
        }
        if (setupco->parsed())
        {
            if (setupCalibco->parsed())
                candleToolCO.setupCalibration(cmdCANopen.id);
            if (setupCalibOutco->parsed())
                candleToolCO.setupCalibrationOutput(cmdCANopen.id);
            if (setupInfoco->parsed())
                candleToolCO.setupInfo(cmdCANopen.id,
                                       (setupInfoAllFlagco->count() > 0 ? true : false));
            if (setupMotorco->parsed())
                candleToolCO.setupMotor(cmdCANopen.id, cmdCANopen.cfgPath, cmdCANopen.force);
            if (setupReadCfgco->parsed())
                candleToolCO.setupReadConfig(cmdCANopen.id, cmdCANopen.value);
        }
        // Sync CANopen
        if (Sync->parsed())
        {
            candleToolCO.SendSync(cmdCANopen.id);
        }
        if (testco->parsed())
        {
            if (testEncoderMainco->parsed())
                candleToolCO.testEncoderMain(cmdCANopen.id);
            if (testEncoderOutco->parsed())
                candleToolCO.testEncoderOutput(cmdCANopen.id);
            if (testLatencyco->parsed())
                candleToolCO.testLatency(cmdCANopen.id);
            if (testMoveco->parsed())
            {
                if (testMoveAbsco->parsed())
                {
                    candleToolCO.testMoveAbsolute(
                        cmdCANopen.id, cmdCANopen.desiredPos, cmdCANopen.param);
                }
                if (testMoveRelco->parsed())
                    candleToolCO.testMove(cmdCANopen.id, cmdCANopen.desiredPos, cmdCANopen.param);

                if (testMoveSpeedco->parsed())
                    candleToolCO.testMoveSpeed(
                        cmdCANopen.id, cmdCANopen.param, cmdCANopen.desiredSpeed);
                if (testImpedanceco->parsed())
                    candleToolCO.testMoveImpedance(cmdCANopen.id,
                                                   cmdCANopen.desiredSpeed,
                                                   cmdCANopen.desiredPos,
                                                   cmdCANopen.param);
            }
        }
        if (heartbeatco->parsed())
        {
            candleToolCO.heartbeatTest(
                cmdCANopen.id, cmdCANopen.newId, cmdCANopen.HeartbeatTimeout);
        }
        if (SDOsegmentedco->parsed())
        {
            if (SDOsegmentedReadco->parsed())
            {
                u16 reg = strtoul(cmdCANopen.reg.c_str(), nullptr, 16);
                candleToolCO.SDOsegmentedRead(cmdCANopen.id, reg, cmdCANopen.subReg);
            }
            if (SDOsegmentedWriteco->parsed())
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
