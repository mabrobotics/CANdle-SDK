#include "mdco_cli.hpp"

using namespace mab;

UserCommandCO cmdCANopen;

std::string MdcoCli::validateAndGetFinalConfigPath(const std::string& cfgPath)
{
    // Check if the file exists, if not, check if it exists relative to the default config
    std::string finalConfigPath        = cfgPath;
    std::string pathRelToDefaultConfig = getMotorsConfigPath() + cfgPath;
    // Check if the file exists
    if (!fileExists(finalConfigPath))
    {
        if (!fileExists(pathRelToDefaultConfig))
        {
            m_log.error("Neither \"%s\", nor \"%s\", exists!.",
                        cfgPath.c_str(),
                        pathRelToDefaultConfig.c_str());
            exit(1);
        }
        finalConfigPath = pathRelToDefaultConfig;
    }
    // Check if the file is a valid config file
    if (!isConfigValid(finalConfigPath))
    {
        m_log.error("\"%s\" in not a valid motor .cfg file.", finalConfigPath.c_str());
        m_log.warn("Valid file must have .cfg extension, and size of < 1MB");
        exit(1);
    }
    std::string defaultConfigPath =
        finalConfigPath.substr(0, finalConfigPath.find_last_of('/') + 1) + "default.cfg";

    // If default config does not exist, warn the user
    if (!fileExists(defaultConfigPath))
    {
        m_log.warn("No default config found at expected location \"%s\"",
                   defaultConfigPath.c_str());
        m_log.warn("Cannot check completeness of the config file. Proceed with upload? [y/n]");
        if (!getConfirmation())
            exit(0);
    }
    // If default config exists, check if the user config is complete, if not, offer to generate
    // missing parts
    if (fileExists(defaultConfigPath) && !isCanOpenConfigComplete(finalConfigPath))
    {
        m_log.m_layer = Logger::ProgramLayer_E::TOP;
        m_log.error("\"%s\" is incomplete.", finalConfigPath.c_str());
        m_log.info("Generate updated file with all required fields? [y/n]");
        if (getConfirmation())
        {
            finalConfigPath = generateUpdatedConfigFile(finalConfigPath);
            m_log.info("Generated updated file \"%s\"", finalConfigPath.c_str());
        }
        else
            m_log.info("Proceeding with original file \"%s\"", finalConfigPath.c_str());
    }
    return finalConfigPath;
}

void MdcoCli::clean(std::string& s)
{
    size_t write_pos    = 0;
    bool   seenNonSpace = false;
    for (size_t read_pos = 0; read_pos < s.size(); ++read_pos)
    {
        unsigned char c = s[read_pos];
        if (std::isspace(c))
        {
            if (!seenNonSpace)
                continue;  // skip leading spaces
            continue;      // skip all spaces (internal & trailing)
        }
        seenNonSpace   = true;
        s[write_pos++] = std::tolower(c);  // pass all character in lower case
    }
    s.resize(write_pos);
}

bool MdcoCli::isCanOpenConfigComplete(const std::string& pathToConfig)
{
    mINI::INIFile      defaultFile(getMotorsConfigPath() + "/CANopen/default.cfg");
    mINI::INIStructure defaultIni;
    defaultFile.read(defaultIni);

    mINI::INIFile      userFile(pathToConfig);
    mINI::INIStructure userIni;
    userFile.read(userIni);

    // Loop fills all lacking fields in the user's config file.
    for (auto const& it : defaultIni)
    {
        auto const& section    = it.first;
        auto const& collection = it.second;
        for (auto const& it2 : collection)
        {
            auto const& key = it2.first;
            if (!userIni[section].has(key))
                return false;
        }
    }
    return true;
}

void MdcoCli::updateUserChoice()
{
    // update user choice for bus, device, data rate, etc. options
    if (m_candleBuilder->preBuildTask)
    {
        m_candleBuilder->preBuildTask();
    }
    else
    {
        m_log.error("Impossible to have user bus choice");
    }
}

MdcoCli::MdcoCli(CLI::App& rootCli, const std::shared_ptr<CandleBuilder> candleBuilder)
    : m_rootCli(rootCli), m_candleBuilder(candleBuilder)
{
    m_log.m_tag   = "MDCO";
    m_log.m_layer = Logger::ProgramLayer_E::TOP;
    mdco          = m_rootCli.add_subcommand("mdco", "Send CANopen command instead of CAN FD.");
    const std::shared_ptr<canId_t> mdCanId = std::make_shared<canId_t>(10);
    auto*                          mdCanIdOption =
        mdco->add_option("-i,--id", *mdCanId, "CAN ID of the MD to interact with.");

    // BLINK ============================================================================

    blink = mdco->add_subcommand("blink", "Blink LEDs on MD drive.")->needs(mdCanIdOption);
    blink->callback(
        [this, candleBuilder, mdCanId]()
        {
            updateUserChoice();
            auto candle = attachCandle(*(candleBuilder->datarate), *(candleBuilder->busType), true);
            MDCO mdco((*mdCanId), candle);
            if (mdco.blinkOpenTest() != MDCO::Error_t::OK)
            {
                m_log.error("Failed to blink MD device with ID %d", *mdCanId);
                detachCandle(candle);
                return;
            }
            m_log.success("Blinking MD device with ID %d", *mdCanId);

            detachCandle(candle);
        });

    // CAN ============================================================================

    can = mdco->add_subcommand("can", "Configure CAN network parameters id, datarate and timeout.")
              ->needs(mdCanIdOption)
              ->require_option();
    CanOptions canOptions(can);
    can->callback(
        [this, candleBuilder, mdCanId, canOptions]()
        {
            updateUserChoice();
            auto candle = attachCandle(*(candleBuilder->datarate), *(candleBuilder->busType), true);
            MDCO mdco(*mdCanId, candle);
            long newbaud;
            if (*canOptions.datarate == "1M")
                newbaud = 1000000;
            else if (*canOptions.datarate == "500K")
                newbaud = 500000;
            else
            {
                m_log.error("Invalid baudrate for CANopen, only 1M and 500K is supported");
                detachCandle(candle);
                return;
            }
            MDCO::Error_t err =
                mdco.newCanOpenConfig(*canOptions.canId, newbaud, *canOptions.timeoutMs);
            if (err != MDCO::OK)
            {
                m_log.error("Error setting CANopen config");
                detachCandle(candle);
                return;
            }
            if (*canOptions.save)
            {
                err = mdco.openSave();
                if (err != MDCO::OK)
                {
                    m_log.error("Error saving CANopen config");
                    detachCandle(candle);
                    return;
                }
            }
            detachCandle(candle);
        });

    // CLEAR ============================================================================

    clear =
        mdco->add_subcommand("clear", "Clear MD drive errors and warnings.")->needs(mdCanIdOption);
    ClearOptions clearOptions(clear);
    clear->callback(
        [this, candleBuilder, mdCanId, clearOptions]()
        {
            updateUserChoice();
            auto candle = attachCandle(*(candleBuilder->datarate), *(candleBuilder->busType), true);
            MDCO mdco((*mdCanId), candle);
            m_log.info("sending in canOpen clear error \n");
            MDCO::Error_t err = MDCO::OK;
            if (*clearOptions.clearType == "error")
                err = mdco.clearOpenErrors(1);
            else if (*clearOptions.clearType == "warning")
                err = mdco.clearOpenErrors(2);
            else if (*clearOptions.clearType == "all")
                err = mdco.clearOpenErrors(3);
            else
            {
                m_log.error("Unknown command");
                detachCandle(candle);
                return;
            }
            if (err != MDCO::OK)
            {
                m_log.error("Error clearing errors (%s)", (*clearOptions.clearType).c_str());
                detachCandle(candle);
                return;
            }
            detachCandle(candle);
        });

    // DISCOVER ============================================================================
    auto* discover = mdco->add_subcommand("discover",
                                          "Discover MD drives on the"
                                          " network.")
                         ->excludes(mdCanIdOption);

    discover->callback(
        [this, candleBuilder]()
        {
            updateUserChoice();
            auto candle = attachCandle(*(candleBuilder->datarate), *(candleBuilder->busType), true);
            std::vector<canId_t> mdIds;
            mdIds = MDCO::discoverOpenMDs(candle);
            m_log.info("Discovered MDCOs: ");
            for (const auto& id : mdIds)
            {
                m_log.info("- %d", id);
            }
            detachCandle(candle);
        });

    // EDS ============================================================================
    eds = mdco->add_subcommand("eds", "EDS file analyzer and generator.")->excludes(mdCanIdOption);

    // EDS load
    edsLoad = eds->add_subcommand("load", "Load the path to the .eds file to analyze.");
    edsLoad->add_option("--file_path", cmdCANopen.value, "Path to the .eds file to analyze.")
        ->required();
    edsLoad->callback(
        [this]()
        {
            edsParser MyEdsParser;
            MyEdsParser.load(cmdCANopen.value);
        });

    // EDS unload
    edsUnload = eds->add_subcommand("unload", "Delete the path to the .eds file.");
    edsUnload->callback(
        [this]()
        {
            edsParser MyEdsParser;
            MyEdsParser.unload();
        });

    // EDS display
    edsDisplay = eds->add_subcommand("display", "Display the content of the .eds file.");
    edsDisplay->callback(
        [this]()
        {
            edsParser MyEdsParser;
            MyEdsParser.display();
        });

    // EDS generate
    edsGen = eds->add_subcommand("generate", "Generate documentation from the .eds file.");

    // EDS generate md
    edsGenMd = edsGen->add_subcommand("md", "Generate .md file from .eds file.");
    edsGenMd->add_option(
        "--path", cmdCANopen.cfgPath, "Path where the user want to generate the md file");
    edsGenMd->callback(
        [this]()
        {
            edsParser MyEdsParser;
            MyEdsParser.generateMarkdown(cmdCANopen.cfgPath);
        });

    // EDS generate html
    edsGenHtml = edsGen->add_subcommand("html", "Generate .html file from .eds file.");
    edsGenHtml->add_option(
        "--path", cmdCANopen.cfgPath, "Path where the user want to generate the html file");
    edsGenHtml->callback(
        [this]()
        {
            edsParser MyEdsParser;
            MyEdsParser.generateHtml(cmdCANopen.cfgPath);
        });

    // EDS generate c++
    edsGencpp = edsGen->add_subcommand("cpp", "Generate .cpp & .hpp file from .eds file.");
    edsGencpp->add_option(
        "--path", cmdCANopen.cfgPath, "Path where the user want to generate the cpp & hpp file");
    edsGencpp->callback(
        [this]()
        {
            edsParser MyEdsParser;
            MyEdsParser.generateCpp(cmdCANopen.cfgPath);
        });

    // EDS get
    edsGet = eds->add_subcommand("get", "Display the content of an object from his index number.");
    edsGet->add_option("--index", cmdCANopen.id, "Index of the object to get.")->required();
    edsGet->add_option("--subindex", cmdCANopen.subReg, "Subindex of the object to get.")
        ->required();
    edsGet->callback(
        [this]()
        {
            edsParser MyEdsParser;
            MyEdsParser.get(cmdCANopen.id, cmdCANopen.subReg);
        });

    // EDS find
    edsFind = eds->add_subcommand("find", "Find and display the content of an object from words.");
    edsFind->add_option("--word", cmdCANopen.value, "Word to search in the .eds file.")->required();
    edsFind->callback(
        [this]()
        {
            edsParser MyEdsParser;
            MyEdsParser.find(cmdCANopen.value);
        });

    // EDS modify
    edsModify = eds->add_subcommand("modify", "Modify the actual .eds file.");

    // EDS modify add
    edsModifyAdd = edsModify->add_subcommand("add", "Add a new object to the .eds file.");

    EdsObjectOPtions myObject(edsModifyAdd);

    edsModifyAdd->callback(
        [this, myObject]()
        {
            edsParser MyEdsParser;
            MyEdsParser.addObject(*myObject.EdsObject);
        });

    // EDS modify delete
    edsModifyDel = edsModify->add_subcommand("del", "Delete an object from the .eds file.");
    edsModifyDel
        ->add_option("--index", cmdCANopen.edsObj.index, "Index of the parameter to delete.")
        ->required();
    edsModifyDel
        ->add_option(
            "--subindex", cmdCANopen.edsObj.subIndex, "Subindex of the parameter to delete.")
        ->required();
    edsModifyDel->callback(
        [this]()
        {
            edsParser MyEdsParser;
            MyEdsParser.deleteObject(cmdCANopen.edsObj.index, cmdCANopen.edsObj.subIndex);
        });

    // EDS modify correction
    edsModifyCorrection =
        edsModify->add_subcommand("correction", "Correct an object from the .eds file.");
    edsModifyCorrection
        ->add_option("--old_index", cmdCANopen.id, "Index of the parameter to correct.")
        ->required();
    edsModifyCorrection
        ->add_option("--old_subindex", cmdCANopen.subReg, "Subindex of the parameter to correct.")
        ->required();

    EdsObjectOPtions corectObject(edsModifyCorrection);

    edsModifyCorrection->callback(
        [this, corectObject]()
        {
            edsParser MyEdsParser;
            MyEdsParser.modifyObject(*corectObject.EdsObject, cmdCANopen.id, cmdCANopen.subReg);
        });

    // ENCODER CANopen ============================================================================
    encoder = mdco->add_subcommand("encoder", "Encoder test");

    // ENCODER display
    encoderDisplay = encoder->add_subcommand("display", "Display MD motor position.");
    encoderDisplay->callback(
        [this, candleBuilder, mdCanId]()
        {
            updateUserChoice();
            auto candle       = attachCandle(*(candleBuilder->datarate), *(candleBuilder->busType));
            MDCO mdco         = MDCO(*mdCanId, candle);
            MDCO::Error_t err = mdco.readOpenRegisters(0x6064, 0);
            if (err != MDCO::OK)
            {
                m_log.error("Error reading encoder value");
            }
            detachCandle(candle);
        });

    // ENCODER test
    testEncoder = encoder->add_subcommand("test", "Perform accuracy test on encoder.");

    // ENCODER test main
    testEncoderMain = testEncoder->add_subcommand("main", "Perform test routine on main encoder.");
    testEncoderMain->callback(
        [this, candleBuilder, mdCanId]()
        {
            updateUserChoice();
            auto candle       = attachCandle(*(candleBuilder->datarate), *(candleBuilder->busType));
            MDCO mdco         = MDCO(*mdCanId, candle);
            MDCO::Error_t err = mdco.testEncoder(true, false);
            if (err != MDCO::OK)
            {
                m_log.error("Error running main encoder test");
            }
            detachCandle(candle);
        });

    // ENCODER test output
    testEncoderOut =
        testEncoder->add_subcommand("output", "Perform test routine on output encoder.");
    testEncoderOut->callback(
        [this, candleBuilder, mdCanId]()
        {
            updateUserChoice();
            auto candle       = attachCandle(*(candleBuilder->datarate), *(candleBuilder->busType));
            MDCO mdco         = MDCO(*mdCanId, candle);
            MDCO::Error_t err = mdco.testEncoder(false, true);
            if (err != MDCO::OK)
            {
                m_log.error("Error running main encoder test");
            }
            detachCandle(candle);
        });

    // Heartbeat ============================================================================

    heartbeat = mdco->add_subcommand("heartbeat", "Test heartbeat detection of a device.")
                    ->excludes(mdCanIdOption);

    HeartbeatOptions HeartbeatOptions(heartbeat);

    heartbeat->callback(
        [this, candleBuilder, mdCanId, HeartbeatOptions]()
        {
            updateUserChoice();
            auto candle = attachCandle(*(candleBuilder->datarate), *(candleBuilder->busType), true);
            MDCO::Error_t err;
            if (*HeartbeatOptions.masterCanId > 0x7F || *HeartbeatOptions.slaveCanId > 0x7f)
            {
                m_log.error("id > 0x7F");
                detachCandle(candle);
                return;
            }
            MDCO            md    = MDCO(*HeartbeatOptions.slaveCanId, candle);
            std::vector<u8> frame = {0x05};
            long            DataSlave;
            u8              bytes1 = ((u8)(*HeartbeatOptions.masterCanId >> 8));
            u8              bytes2 = ((u8)(*HeartbeatOptions.masterCanId));
            u8              bytes3 = ((u8)(*HeartbeatOptions.timeout >> 8));
            u8              bytes4 = ((u8)(*HeartbeatOptions.timeout));
            DataSlave              = bytes4 + (bytes3 << 8) + (bytes2 << 16) + (bytes1 << 24);
            err = md.sendCustomData(0x700 + *HeartbeatOptions.masterCanId, frame);
            if (err != MDCO::OK)
            {
                m_log.error("Error sending heartbeat");
                detachCandle(candle);
                return;
            }
            if (md.getValueFromOpenRegister(0x1003, 0x00) != 00)
            {
                m_log.error("The Driver is in fault state before testing the heartbeat");
                detachCandle(candle);
                return;
            }
            err = md.writeOpenRegisters(0x1016, 0x01, DataSlave, 4);
            if (err != MDCO::OK)
            {
                m_log.error("Error setting slave data for heartbeat");
                detachCandle(candle);
                return;
            }
            err = md.sendCustomData(0x700 + *HeartbeatOptions.masterCanId, frame);
            if (err != MDCO::OK)
            {
                m_log.error("Error sending heartbeat");
                detachCandle(candle);
                return;
            }
            auto start   = std::chrono::steady_clock::now();
            auto timeout = std::chrono::milliseconds((*HeartbeatOptions.timeout / 100));
            while (std::chrono::steady_clock::now() - start < timeout)
            {
            }
            // verify if error state before HeartbeatTimeout
            if (md.getValueFromOpenRegister(0x1001, 0) != 0)
            {
                m_log.error("The driver enter fault mode before the Heartbeat timeout");
                detachCandle(candle);
                return;
            }
            start   = std::chrono::steady_clock::now();
            timeout = std::chrono::milliseconds((*HeartbeatOptions.timeout * 2));
            while (std::chrono::steady_clock::now() - start < timeout)
            {
            }
            // verify if error state after HeartbeatTimeout
            if (md.getValueFromOpenRegister(0x1001, 0) != 0)
            {
                m_log.success("The driver enter fault mode after the Heartbeat timeout");
                detachCandle(candle);
                return;
            }
            md.sendCustomData(0x700 + *HeartbeatOptions.masterCanId, frame);
            if (md.getValueFromOpenRegister(0x1001, 0) != 0)
            {
                m_log.error("The driver still in error mode");
            }
            detachCandle(candle);
        });

    // NMT ============================================================================
    nmt = mdco->add_subcommand("nmt", "Send NMT commands.")->needs(mdCanIdOption);

    // NMT read
    nmtRead = nmt->add_subcommand("read_state", "Read the NMT state of the MD drive.");
    nmtRead->callback(
        [this, candleBuilder, mdCanId]()
        {
            updateUserChoice();
            auto candle       = attachCandle(*(candleBuilder->datarate), *(candleBuilder->busType));
            MDCO md           = MDCO(*mdCanId, candle);
            MDCO::Error_t err = md.testHeartbeat();
            if (err != MDCO::OK)
            {
                m_log.error("Error reading heartbeat for node %d", *mdCanId);
            }
            detachCandle(candle);
        });

    // NMT operational
    nmtOperational =
        nmt->add_subcommand("go_to_operational", "Send command to go to pre-operational state.");
    nmtOperational->callback(
        [this, candleBuilder, mdCanId]()
        {
            updateUserChoice();
            auto candle = attachCandle(*(candleBuilder->datarate), *(candleBuilder->busType), true);
            MDCO mdco   = MDCO(*mdCanId, candle);
            std::vector<u8> data = {0x01, (u8)*mdCanId};
            MDCO::Error_t   err  = mdco.writeOpenPDORegisters(0x000, data);
            if (err != MDCO::OK)
            {
                m_log.error("Error sending NMT command %d to node %d", 0x01, *mdCanId);
                return;
            }

            detachCandle(candle);
        });

    // NMT stop
    nmtStop = nmt->add_subcommand("stopped", "Send command to go to stop state.");
    nmtStop->callback(
        [this, candleBuilder, mdCanId]()
        {
            updateUserChoice();
            auto candle = attachCandle(*(candleBuilder->datarate), *(candleBuilder->busType), true);
            MDCO mdco   = MDCO(*mdCanId, candle);
            std::vector<u8> data = {0x02, (u8)*mdCanId};
            MDCO::Error_t   err  = mdco.writeOpenPDORegisters(0x000, data);
            if (err != MDCO::OK)
            {
                m_log.error("Error sending NMT command %d to node %d", 0x02, *mdCanId);
                return;
            }
            detachCandle(candle);
        });

    // NMT operational
    nmtPreOperational =
        nmt->add_subcommand("pre_operational", "Send command to go to pre-operational state.");
    nmtPreOperational->callback(
        [this, candleBuilder, mdCanId]()
        {
            updateUserChoice();
            auto candle = attachCandle(*(candleBuilder->datarate), *(candleBuilder->busType), true);
            MDCO mdco   = MDCO(*mdCanId, candle);
            std::vector<u8> data = {0x80, (u8)*mdCanId};
            MDCO::Error_t   err  = mdco.writeOpenPDORegisters(0x000, data);
            if (err != MDCO::OK)
            {
                m_log.error("Error sending NMT command %d to node %d", 0x80, *mdCanId);
            }
            detachCandle(candle);
        });

    // NMT reset node
    nmtResetNode = nmt->add_subcommand("reset_node", "Reset the node.");
    nmtResetNode->callback(
        [this, candleBuilder, mdCanId]()
        {
            updateUserChoice();
            auto candle = attachCandle(*(candleBuilder->datarate), *(candleBuilder->busType), true);
            MDCO mdco   = MDCO(*mdCanId, candle);
            std::vector<u8> data = {0x81, (u8)*mdCanId};
            MDCO::Error_t   err  = mdco.writeOpenPDORegisters(0x000, data);
            if (err != MDCO::OK)
            {
                m_log.error("Error sending NMT command %d to node %d", 0x81, *mdCanId);
                return;
            }
            detachCandle(candle);
        });

    // NMT reset communication
    nmtResetCommunication =
        nmt->add_subcommand("reset_communication", "Reset the node communication.");
    nmtResetCommunication->callback(
        [this, candleBuilder, mdCanId]()
        {
            updateUserChoice();
            auto candle = attachCandle(*(candleBuilder->datarate), *(candleBuilder->busType), true);
            MDCO mdco   = MDCO(*mdCanId, candle);
            std::vector<u8> data = {0x82, (u8)*mdCanId};
            MDCO::Error_t   err  = mdco.writeOpenPDORegisters(0x000, data);
            if (err != MDCO::OK)
            {
                m_log.error("Error sending NMT command %d to node %d", 0x82, *mdCanId);
                return;
            }
            detachCandle(candle);
        });

    // PDOTEST ============================================================================

    pdoTest = mdco->add_subcommand("pdo", "Try to send pdo can frame instead of sdo.")
                  ->needs(mdCanIdOption);

    // PDOTEST speed
    PdoSpeed = pdoTest->add_subcommand("speed", "Perform a speed loop.");
    PdoSpeed->add_option("--speed", cmdCANopen.desiredSpeed, "Desired motor speed [RPM].")
        ->required();
    PdoSpeed->callback(
        [this, candleBuilder, mdCanId]()
        {
            updateUserChoice();
            auto candle = attachCandle(*(candleBuilder->datarate), *(candleBuilder->busType), true);
            MDCO md     = MDCO(*mdCanId, candle);

            m_log.info("Sending SDO for motor setup!");

            moveParameter param;
            param.MaxCurrent   = 500;
            param.MaxTorque    = 500;
            param.RatedTorque  = 1000;
            param.RatedCurrent = 1000;
            param.MaxSpeed     = 200;
            MDCO::Error_t err  = md.setProfileParameters(param);
            if (err != MDCO::OK)
            {
                m_log.error("Error setting profile parameters");
                detachCandle(candle);
                return;
            }
            err = md.enableDriver(CyclicSyncVelocity);
            if (err != MDCO::OK)
            {
                m_log.error("Error enabling driver");
                detachCandle(candle);
                return;
            }

            m_log.info("Sending PDO for speed loop control");
            std::vector<u8> frameSetup = {0x0F, 0x00, 0x09};
            err                        = md.writeOpenPDORegisters(0x300 + *mdCanId, frameSetup);
            if (err != MDCO::OK)
            {
                m_log.error("Error sending setup PDO");
                detachCandle(candle);
                return;
            }

            std::vector<u8> frameSpeed = {0x0F,
                                          0x00,
                                          (u8)(cmdCANopen.desiredSpeed),
                                          (u8)(cmdCANopen.desiredSpeed >> 8),
                                          (u8)(cmdCANopen.desiredSpeed >> 16),
                                          (u8)(cmdCANopen.desiredSpeed >> 24)};
            err                        = md.writeOpenPDORegisters(0x500 + *mdCanId, frameSpeed);
            if (err != MDCO::OK)
            {
                m_log.error("Error sending speed PDO");
                detachCandle(candle);
                return;
            }
            auto start   = std::chrono::steady_clock::now();
            auto timeout = std::chrono::seconds((5));
            while (std::chrono::steady_clock::now() - start < timeout)
            {
            }
            if ((int)md.getValueFromOpenRegister(0x606C, 0x00) <= cmdCANopen.desiredSpeed + 5 &&
                (int)md.getValueFromOpenRegister(0x606C, 0x00) >= cmdCANopen.desiredSpeed - 5)
            {
                m_log.success("Velocity Target reached with +/- 5RPM");
            }
            else
            {
                m_log.error("Velocity Target not reached");
            }
            err = md.disableDriver();
            if (err != MDCO::OK)
            {
                m_log.error("Error disabling driver");
                detachCandle(candle);
                return;
            }
            detachCandle(candle);
        });

    // PDOTEST position
    PdoPosition = pdoTest->add_subcommand("position", "Perform a position loop.");
    PdoPosition->add_option("--position", cmdCANopen.desiredPos, "Desired motor position [inc].")
        ->required();
    PdoPosition->callback(
        [this, candleBuilder, mdCanId]()
        {
            updateUserChoice();
            auto candle = attachCandle(*(candleBuilder->datarate), *(candleBuilder->busType), true);
            MDCO md     = MDCO(*mdCanId, candle);

            m_log.info("Sending SDO for motor setup!");
            moveParameter param;
            param.MaxCurrent   = 500;
            param.MaxTorque    = 500;
            param.RatedTorque  = 1000;
            param.RatedCurrent = 1000;
            param.MaxSpeed     = 200;
            MDCO::Error_t err  = md.setProfileParameters(param);
            if (err != MDCO::OK)
            {
                m_log.error("Error setting profile parameters");
                detachCandle(candle);
                return;
            }
            err = md.enableDriver(CyclicSyncPosition);
            if (err != MDCO::OK)
            {
                m_log.error("Error enabling driver");
                detachCandle(candle);
                return;
            }

            auto start   = std::chrono::steady_clock::now();
            auto timeout = std::chrono::seconds((1));

            m_log.info("Sending PDO for speed loop control");

            std::vector<u8> frameSetup = {0x0F, 0x00, 0x08};
            err                        = md.writeOpenPDORegisters(0x300 + *mdCanId, frameSetup);
            if (err != MDCO::OK)
            {
                m_log.error("Error sending setup PDO");
                detachCandle(candle);
                return;
            }
            std::vector<u8> framePosition = {0x0F,
                                             0x00,
                                             (u8)(cmdCANopen.desiredPos),
                                             (u8)(cmdCANopen.desiredPos >> 8),
                                             (u8)(cmdCANopen.desiredPos >> 16),
                                             (u8)(cmdCANopen.desiredPos >> 24)};
            err = md.writeOpenPDORegisters(0x400 + *mdCanId, framePosition);
            if (err != MDCO::OK)
            {
                m_log.error("Error sending position PDO");
                detachCandle(candle);
                return;
            }

            m_log.debug("position ask : %d\n", cmdCANopen.desiredPos);

            start           = std::chrono::steady_clock::now();
            auto lastSend   = start;
            timeout         = std::chrono::seconds(5);
            auto sendPeriod = std::chrono::milliseconds(100);

            while (std::chrono::steady_clock::now() - start < timeout &&
                   !((int)md.getValueFromOpenRegister(0x6064, 0) > (cmdCANopen.desiredPos - 100) &&
                     (int)md.getValueFromOpenRegister(0x6064, 0) < (cmdCANopen.desiredPos + 100)))
            {
                auto now = std::chrono::steady_clock::now();
                if (now - lastSend >= sendPeriod)
                {
                    MDCO::Error_t err =
                        md.writeOpenRegisters("Motor Target Position", cmdCANopen.desiredPos, 4);
                    if (err != MDCO::OK)
                    {
                        m_log.error("Error setting Motor Target Position");
                        detachCandle(candle);
                        return;
                    }
                    lastSend = now;
                }
            }

            m_log.debug("position actual : %d\n", (int)md.getValueFromOpenRegister(0x6064, 0));

            if (((int)md.getValueFromOpenRegister(0x6064, 0) > (cmdCANopen.desiredPos - 200) &&
                 (int)md.getValueFromOpenRegister(0x6064, 0) < (cmdCANopen.desiredPos + 200)))
            {
                m_log.success("Position reached in less than 5s");
            }
            else
            {
                m_log.error("Position not reached in less than 5s");
            }

            err = md.disableDriver();
            if (err != MDCO::OK)
            {
                m_log.error("Error disabling driver");
                detachCandle(candle);
                return;
            }

            detachCandle(candle);
        });

    // PDOTEST custom
    PdoCustom = pdoTest->add_subcommand("custom", "Allow the user to send custom PDOs.");
    PdoCustom->add_option("--index", cmdCANopen.edsObj.index, "Index of the PDO.")->required();
    PdoCustom->add_option("--data", cmdCANopen.value, "Data message to send.")->required();
    PdoCustom->callback(
        [this, candleBuilder, mdCanId]()
        {
            updateUserChoice();
            unsigned long long data = strtoul((cmdCANopen.value.c_str()), nullptr, 16);
            auto candle = attachCandle(*(candleBuilder->datarate), *(candleBuilder->busType), true);
            MDCO mdco   = MDCO(*mdCanId, candle);

            MDCO::Error_t err;
            if (cmdCANopen.edsObj.index == (0x1600))
            {
                std::vector<u8> frame = {(u8)(data >> 8), (u8)(data)};
                err                   = mdco.writeOpenPDORegisters(0x200 + *mdCanId, frame);
                if (err != MDCO::OK)
                {
                    m_log.error("Error sending custom PDO 0x1600");
                    detachCandle(candle);
                    return;
                }
            }
            else if (cmdCANopen.edsObj.index == (0x1601))
            {
                std::vector<u8> frame = {(u8)(data >> 16), (u8)(data >> 8), (u8)(data)};
                err                   = mdco.writeOpenPDORegisters(0x300 + *mdCanId, frame);
                if (err != MDCO::OK)
                {
                    m_log.error("Error sending custom PDO 0x1601");
                    detachCandle(candle);
                    return;
                }
            }
            else if (cmdCANopen.edsObj.index == (0x1602))
            {
                std::vector<u8> frame = {(u8)(data >> 40),
                                         (u8)(data >> 32),
                                         (u8)(data >> 24),
                                         (u8)(data >> 16),
                                         (u8)(data >> 8),
                                         (u8)(data)};
                err                   = mdco.writeOpenPDORegisters(0x400 + *mdCanId, frame);
                if (err != MDCO::OK)
                {
                    m_log.error("Error sending custom PDO 0x1602");
                    detachCandle(candle);
                    return;
                }
            }
            else if (cmdCANopen.edsObj.index == (0x1603))
            {
                std::vector<u8> frame = {(u8)(data >> 40),
                                         (u8)(data >> 32),
                                         (u8)(data >> 24),
                                         (u8)(data >> 16),
                                         (u8)(data >> 8),
                                         (u8)(data)};
                err                   = mdco.writeOpenPDORegisters(0x500 + *mdCanId, frame);
                if (err != MDCO::OK)
                {
                    m_log.error("Error sending custom PDO 0x1603");
                    detachCandle(candle);
                    return;
                }
            }
            else
            {
                m_log.error("Please enter a index between 0x1600 & 0x1603 (Transmit PDO)");
                detachCandle(candle);
                return;
            }

            detachCandle(candle);
        });

    // REGISTER ============================================================================

    registr = mdco->add_subcommand("register", "Access MD drive via register read/write.")
                  ->needs(mdCanIdOption);

    // REGISTER read
    regRead = registr->add_subcommand("read", "Read MD register.");

    ReadWriteOptions readOption(regRead);

    regRead->callback(
        [this, candleBuilder, mdCanId, readOption]()
        {
            updateUserChoice();
            auto candle = attachCandle(*(candleBuilder->datarate), *(candleBuilder->busType), true);
            MDCO mdco   = MDCO(*mdCanId, candle);
            std::vector<u8> data;
            int dataSize = mdco.dataSizeOfEdsObject(*readOption.index, *readOption.subindex);
            if (!*readOption.force)
            {
                if (dataSize == 1 || dataSize == 2 || dataSize == 4)
                    mdco.readOpenRegisters(
                        *readOption.index, *readOption.subindex, *readOption.force);
                else if (dataSize == 8 || dataSize == 0)
                    mdco.readLongOpenRegisters(*readOption.index, *readOption.subindex, data);
                else
                {
                    m_log.error("Unknown register size for register 0x%04X subindex %d",
                                *readOption.index,
                                *readOption.subindex);
                }
            }
            else
                mdco.readOpenRegisters(*readOption.index, *readOption.subindex, *readOption.force);

            detachCandle(candle);
        });

    // REGISTER write
    regWrite = registr->add_subcommand("write", "Write MD register.");
    ReadWriteOptions writeOption(regWrite);
    regWrite->add_option("--value", cmdCANopen.value, "Value to write.")->required();
    regWrite->add_option("--dataSize",
                         cmdCANopen.dataSize,
                         "Size of data to be sent. {1,2,4}[bytes]. Mandatory for CANopen.");

    regWrite->callback(
        [this, candleBuilder, mdCanId, writeOption]()
        {
            updateUserChoice();
            auto candle = attachCandle(*(candleBuilder->datarate), *(candleBuilder->busType), true);
            MDCO mdco   = MDCO(*mdCanId, candle);

            MDCO::Error_t err;
            // if no value is given, we read the value from the object dictionary
            if (cmdCANopen.dataSize == 0)
            {
                if (*writeOption.force)
                {
                    m_log.error("Please enter the data size in bytes if you use the -f flag");
                    detachCandle(candle);
                    return;
                }
                else
                    cmdCANopen.dataSize =
                        mdco.dataSizeOfEdsObject(*writeOption.index, *writeOption.subindex);
            }

            if (cmdCANopen.dataSize == 1 || cmdCANopen.dataSize == 2 || cmdCANopen.dataSize == 4)
            {
                unsigned long data = strtoul((cmdCANopen.value.c_str()), nullptr, 16);
                err                = mdco.writeOpenRegisters(*writeOption.index,
                                              *writeOption.subindex,
                                              data,
                                              cmdCANopen.dataSize,
                                              *writeOption.force);
                if (err != MDCO::OK)
                {
                    m_log.error("Error writing register 0x%04X subindex %d",
                                *writeOption.index,
                                *writeOption.subindex);
                    detachCandle(candle);
                    return;
                }
                detachCandle(candle);
                return;
            }
            else if (cmdCANopen.dataSize == 8)
            {
                err = mdco.writeLongOpenRegisters(*writeOption.index,
                                                  *writeOption.subindex,
                                                  cmdCANopen.value,
                                                  *writeOption.force);
                if (err != MDCO::OK)
                {
                    m_log.error("Error writing long register 0x%04X subindex %d",
                                *writeOption.index,
                                *writeOption.subindex);
                    detachCandle(candle);
                    return;
                }
                detachCandle(candle);
                return;
            }
            else if (cmdCANopen.dataSize == 0)
            {
                err = mdco.writeLongOpenRegisters(*writeOption.index,
                                                  *writeOption.subindex,
                                                  cmdCANopen.value,
                                                  *writeOption.force);
                if (err != MDCO::OK)
                {
                    m_log.error("Error writing string register 0x%04X subindex %d",
                                *writeOption.index,
                                *writeOption.subindex);
                    detachCandle(candle);
                    return;
                }
                detachCandle(candle);
                return;
            }
            else
            {
                m_log.error("Wrong/unknow data size");
                detachCandle(candle);
                return;
            }

            detachCandle(candle);
        });

    // RESET ============================================================================
    reset = mdco->add_subcommand("reset", "Reset MD drive.")->needs(mdCanIdOption);
    reset->callback(
        [this, candleBuilder, mdCanId]()
        {
            updateUserChoice();
            auto candle = attachCandle(*(candleBuilder->datarate), *(candleBuilder->busType), true);
            MDCO mdco((*mdCanId), candle);
            MDCO::Error_t err = mdco.openReset();
            if (err != MDCO::OK)
            {
                m_log.error("Error resetting MD device with ID %d", *mdCanId);
                detachCandle(candle);
                return;
            }

            detachCandle(candle);
        });

    // SETUP ============================================================================
    setup = mdco->add_subcommand("setup", "Setup MD via config files, and calibrate.")
                ->needs(mdCanIdOption);

    // SETUP calibration
    setupCalib = setup->add_subcommand("calibration", "Calibrate main MD encoder.");
    setupCalib->callback(
        [this, candleBuilder, mdCanId]()
        {
            updateUserChoice();
            auto candle = attachCandle(*(candleBuilder->datarate), *(candleBuilder->busType), true);
            MDCO mdco((*mdCanId), candle);
            MDCO::Error_t err = mdco.encoderCalibration(1, 0);
            if (err != MDCO::OK)
            {
                m_log.error("Error running main encoder calibration");
            }

            detachCandle(candle);
        });

    // SETUP calibration output
    setupCalibOut = setup->add_subcommand("calibration_out", "Calibrate output encoder.");
    setupCalibOut->callback(
        [this, candleBuilder, mdCanId]()
        {
            updateUserChoice();
            auto candle = attachCandle(*(candleBuilder->datarate), *(candleBuilder->busType), true);
            MDCO mdco((*mdCanId), candle);
            MDCO::Error_t err = mdco.encoderCalibration(0, 1);
            if (err != MDCO::OK)
            {
                m_log.error("Error running output encoder calibration");
            }

            detachCandle(candle);
        });

    // SETUP info
    setupInfo        = setup->add_subcommand("info", "Display info about the MD drive.");
    setupInfoAllFlag = setupInfo->add_flag("-a", "Print ALL available info.");
    setupInfo->callback(
        [this, candleBuilder, mdCanId]()
        {
            updateUserChoice();
            auto candle = attachCandle(*(candleBuilder->datarate), *(candleBuilder->busType), true);
            MDCO mdco((*mdCanId), candle);
            bool printAll = false;
            setupInfoAllFlag->count() > 0 ? printAll = true : printAll = false;

            if (!printAll)
            {
                long devicetype = mdco.getValueFromOpenRegister(0x1000, 0);
                m_log.info("Device type:", devicetype);
                long int    hexValue = mdco.getValueFromOpenRegister(0x1008, 0);
                std::string motorName;
                for (int i = 0; i <= 3; i++)
                {
                    char c = (hexValue >> (8 * i)) & 0xFF;
                    motorName += c;
                }
                m_log.info("Manufacturer Device Name: %s", motorName.c_str());
                long Firmware = mdco.getValueFromOpenRegister(0x200A, 3);
                m_log.info("Firmware version: %li", Firmware);
                long Bootloader = mdco.getValueFromOpenRegister(0x200B, 4);
                m_log.info("Bootloader version: %li", Bootloader);
            }
            else
            {
                mdco.printAllInfo();
            }

            detachCandle(candle);
        });

    // SETUP upload
    setupupload = setup->add_subcommand("upload", "Upload actuator config from .cfg file.");
    setupupload
        ->add_option("--file_path",
                     cmdCANopen.cfgPath,
                     "Filename of motor config. Default config files are "
                     "in:`/etc/candletool/config/motors/`.")
        ->required();
    setupupload->add_flag(
        "-f,--force", cmdCANopen.force, "Force uploading config file, without verification.");
    setupupload->callback(
        [this, candleBuilder, mdCanId]()
        {
            updateUserChoice();
            std::string finalConfigPath = cmdCANopen.cfgPath;
            if (!cmdCANopen.force)
                finalConfigPath = validateAndGetFinalConfigPath(cmdCANopen.cfgPath);
            else
            {
                m_log.warn("Omitting config validation on user request!");
                if (!fileExists(finalConfigPath))
                {
                    finalConfigPath = getMotorsConfigPath() + cmdCANopen.cfgPath;
                    if (!fileExists(finalConfigPath))
                    {
                        m_log.error("Neither \"%s\", nor \"%s\", exists!.",
                                    cmdCANopen.cfgPath.c_str(),
                                    finalConfigPath.c_str());
                        exit(1);
                    }
                }
            }
            m_log.info("Uploading config from \"%s\"", finalConfigPath.c_str());
            mINI::INIFile      motorCfg(finalConfigPath);
            mINI::INIStructure cfg;
            motorCfg.read(cfg);
            auto candle = attachCandle(*(candleBuilder->datarate), *(candleBuilder->busType), true);
            MDCO mdco((*mdCanId), candle);
            MDCOConfigMap configMap;

            for (const auto& entry : configMap.m_map)
            {
                const auto& addr    = entry.first;
                const auto& element = entry.second;

                if (element.Section == "motor" && element.Key == "KV")
                    continue;

                if (!cfg.has(element.Section))
                    continue;

                auto& sectionMap = cfg[element.Section];
                if (!sectionMap.has(element.Key))
                    continue;

                std::string   value = sectionMap[element.Key];
                MDCO::Error_t err   = MDCO::OK;

                switch (element.Type)
                {
                    case MDCOValueType::STRING:
                        err = mdco.writeLongOpenRegisters(addr.first, addr.second, value, true);
                        break;

                    case MDCOValueType::FLOAT:
                    {
                        float    f = std::stof(value);
                        uint32_t as_long;
                        std::memcpy(&as_long, &f, sizeof(float));
                        err = mdco.writeOpenRegisters(addr.first, addr.second, as_long, 4);
                        break;
                    }

                    case MDCOValueType::INT:
                    {
                        int i = std::stoi(value);
                        err   = mdco.writeOpenRegisters(addr.first, addr.second, i);
                        break;
                    }
                }

                if (err != MDCO::OK)
                {
                    m_log.error("Error setting  skipping section [%s] element %s",
                                element.Section.c_str(),
                                element.Key.c_str());
                }
            }

            m_log.success("Don't forget to save the config before shutting down the MD!");
            detachCandle(candle);
        });

    // SETUP download
    setupdownload =
        setup->add_subcommand("download", "Download actuator config from MD to .cfg file.");
    setupdownload->add_option("--path", cmdCANopen.value, "File to save config to.")->required();
    setupdownload->callback(
        [this, candleBuilder, mdCanId]()
        {
            updateUserChoice();
            auto candle = attachCandle(*(candleBuilder->datarate), *(candleBuilder->busType), true);
            MDCO mdco((*mdCanId), candle);
            MDCOConfigMap configMap;
            std::ofstream cfg(cmdCANopen.value);

            if (!cfg.is_open())
            {
                m_log.error("Impossible to open %s in writing mode", cmdCANopen.value.c_str());
                detachCandle(candle);
                return;
            }
            cfg << std::fixed << std::setprecision(5);
            std::string currentSection;
            for (const auto& [address, element] : configMap.m_map)
            {
                if (currentSection != element.Section)
                {
                    currentSection = element.Section;
                    cfg << "\n[" << currentSection << "]\n";
                }
                if (element.Section == "motor" && element.Key == "KV")
                    continue;
                if (element.Key == "name")
                {
                    std::vector<u8> name;
                    auto            err =
                        mdco.readLongOpenRegisters(address.first, address.second, name, true);
                    if (err != 0)
                    {
                        name.clear();
                    }
                    cfg << element.Key << " = " << std::string(name.begin(), name.end()) << "\n";
                }
                else
                {
                    long raw_data = mdco.getValueFromOpenRegister(address.first, address.second);
                    if (element.Key == "kp" || element.Key == "ki" || element.Key == "kd" ||
                        element.Key == "windup" ||
                        element.Key.find("constant") != std::string::npos ||
                        element.Key.find("ratio") != std::string::npos)
                    {
                        float f;
                        std::memcpy(&f, &raw_data, sizeof(float));
                        cfg << element.Key << " = " << f << "\n";
                    }
                    else
                    {
                        cfg << element.Key << " = " << raw_data << "\n";
                    }
                }
            }
            cfg.close();
            m_log.success("File %s generate with success.", cmdCANopen.value.c_str());
            detachCandle(candle);
        });

    // SDOsegmented ============================================================================

    SDOsegmented =
        mdco->add_subcommand("segmented", "Send a message using SDO segmented transfer.");

    // SDOsegmented read
    SDOsegmentedRead = SDOsegmented->add_subcommand(
        "read", "Read a value from a register with more than 4 bytes of data.");
    SDOsegmentedRead
        ->add_option("--index", cmdCANopen.reg, "Register ID (offset) to read data from.")
        ->required();
    SDOsegmentedRead
        ->add_option("--subindex", cmdCANopen.subReg, "Register ID (offset) to read data from.")
        ->required();
    SDOsegmentedRead->callback(
        [this, candleBuilder, mdCanId]()
        {
            updateUserChoice();
            u16  reg    = strtoul(cmdCANopen.reg.c_str(), nullptr, 16);
            auto candle = attachCandle(*(candleBuilder->datarate), *(candleBuilder->busType), true);
            MDCO mdco((*mdCanId), candle);
            std::vector<u8> data;
            MDCO::Error_t   err = mdco.readLongOpenRegisters(reg, cmdCANopen.subReg, data);
            if (err != MDCO::OK)
            {
                m_log.error(
                    "Error reading segmented SDO 0x%04X subindex %d", reg, cmdCANopen.subReg);
            }
            detachCandle(candle);
        });

    // SDOsegmented write
    SDOsegmentedWrite = SDOsegmented->add_subcommand(
        "write", "write a value in a register with more than 4 bytes of data.");
    SDOsegmentedWrite
        ->add_option("--index", cmdCANopen.reg, "Register ID (offset) to read data from.")
        ->required();
    SDOsegmentedWrite
        ->add_option("--subindex", cmdCANopen.subReg, "Register ID (offset) to read data from.")
        ->required();
    SDOsegmentedWrite->add_option("--data", cmdCANopen.value, "Value to write into the register.")
        ->required();
    SDOsegmentedWrite->callback(
        [this, candleBuilder, mdCanId]()
        {
            updateUserChoice();
            u16  reg    = strtoul(cmdCANopen.reg.c_str(), nullptr, 16);
            auto candle = attachCandle(*(candleBuilder->datarate), *(candleBuilder->busType), true);
            MDCO mdco((*mdCanId), candle);

            MDCO::Error_t err =
                mdco.writeLongOpenRegisters(reg, cmdCANopen.subReg, cmdCANopen.value);
            if (err != MDCO::OK)
            {
                m_log.error(
                    "Error writing segmented SDO 0x%04X subindex %d", reg, cmdCANopen.subReg);
            }

            detachCandle(candle);
        });

    // Sync ============================================================================
    Sync = mdco->add_subcommand("sync", "Send a sync CANopen message.")->needs(mdCanIdOption);
    Sync->callback(
        [this, candleBuilder, mdCanId]()
        {
            updateUserChoice();
            auto candle = attachCandle(*(candleBuilder->datarate), *(candleBuilder->busType), true);
            MDCO mdco((*mdCanId), candle);
            long SyncMessageValue = mdco.getValueFromOpenRegister(0x1005, 0x0);
            std::vector<u8> data;
            MDCO::Error_t   err;
            if (SyncMessageValue != -1)
            {
                err = mdco.writeOpenPDORegisters((int)SyncMessageValue, data);
                if (err != MDCO::OK)
                {
                    m_log.error("Error sending sync message");
                    detachCandle(candle);
                    return;
                }
                m_log.success("Sync message send with value:0x%x (default value is 0x80)",
                              SyncMessageValue);
            }
            else
                m_log.error("MD with ID:0x%x is not detected", SyncMessageValue);

            detachCandle(candle);
        });

    // TEST ============================================================================
    test = mdco->add_subcommand("test", "Test the MD drive movement.")
               ->needs(mdCanIdOption)
               ->require_subcommand();

    // TEST latency
    testLatency = test->add_subcommand(
        "latency",
        "Test max data exchange rate between your computer and all MD connected  drives.");
    testLatency->callback(
        [this, candleBuilder, mdCanId]()
        {
            updateUserChoice();
            auto candle = attachCandle(*(candleBuilder->datarate), *(candleBuilder->busType), true);
            MDCO mdco((*mdCanId), candle);
            u64  total_latency = 0;
            bool testOk        = true;

            for (int i = 0; i < 100; ++i)
            {
                auto start = std::chrono::steady_clock::now();

                if (mdco.readOpenRegisters(0x1000, 0) != MDCO::Error_t::OK)
                {
                    testOk = false;
                }

                auto end = std::chrono::steady_clock::now();
                auto duration =
                    std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
                total_latency += static_cast<u64>(duration);
            }

            if (testOk)
            {
                u64 average = total_latency / 100;
                m_log.info("---------------Latency---------------\n");
                m_log.info("Total: %lu µs\n", total_latency);
                m_log.info("Result (average of 100 attempts): %lu µs\n", average);
            }

            else
            {
                m_log.error("MD driver not answering");
            }

            detachCandle(candle);
        });

    // TEST move
    testMove = test->add_subcommand("move", "Validate if motor can move.")->require_subcommand();

    // TEST move absolute
    testMoveAbs = testMove->add_subcommand("absolute", "Move motor to absolute position.");
    testMoveAbs
        ->add_option("--position", cmdCANopen.desiredPos, "Absolute position to reach [rad].")
        ->required();

    MovementLimitsOPtions moveAbsParam(testMoveAbs);

    testMoveAbs->callback(
        [this, candleBuilder, mdCanId, moveAbsParam]()
        {
            updateUserChoice();
            auto candle = attachCandle(*(candleBuilder->datarate), *(candleBuilder->busType), true);
            MDCO mdco((*mdCanId), candle);
            MDCO::Error_t err;
            err = mdco.setProfileParameters(*moveAbsParam.param);
            if (err != MDCO::OK)
            {
                m_log.error("Error setting profile parameters");
                detachCandle(candle);
                return;
            }
            err = mdco.enableDriver(ProfilePosition);
            if (err != MDCO::OK)
            {
                m_log.error("Error enabling driver");
                detachCandle(candle);
                return;
            }
            mdco.movePosition(cmdCANopen.desiredPos);
            err = mdco.disableDriver();
            if (err != MDCO::OK)
            {
                m_log.error("Error disabling driver");
                detachCandle(candle);
                return;
            }

            detachCandle(candle);
        });

    // TEST move relative
    testMoveRel = testMove->add_subcommand("relative", "Move motor to relative position.");
    testMoveRel
        ->add_option("--position",
                     cmdCANopen.desiredPos,
                     "Relative position to reach.<0x0, "
                     "0xFFFFFFFF>[inc] ")
        ->required();

    MovementLimitsOPtions moveRelParam(testMoveRel);

    testMoveRel->callback(
        [this, candleBuilder, mdCanId, moveRelParam]()
        {
            updateUserChoice();
            auto candle = attachCandle(*(candleBuilder->datarate), *(candleBuilder->busType), true);
            MDCO mdco((*mdCanId), candle);
            MDCO::Error_t err;
            err = mdco.setProfileParameters(*moveRelParam.param);
            if (err != MDCO::OK)
            {
                m_log.error("Error setting profile parameters");
                detachCandle(candle);
                return;
            }
            err = mdco.enableDriver(CyclicSyncPosition);
            if (err != MDCO::OK)
            {
                m_log.error("Error enabling driver");
                detachCandle(candle);
                return;
            }
            mdco.movePosition(cmdCANopen.desiredPos);
            err = mdco.disableDriver();
            if (err != MDCO::OK)
            {
                m_log.error("Error disabling driver");
                detachCandle(candle);
                return;
            }

            detachCandle(candle);
        });

    // TEST move speed
    testMoveSpeed = testMove->add_subcommand("speed", "Move motor to desired speed.");
    testMoveSpeed
        ->add_option(
            "--speed", cmdCANopen.desiredSpeed, "Sets the target velocity for all motion modes.")
        ->required();

    MovementLimitsOPtions moveSpeedParam(testMoveSpeed);

    testMoveSpeed->callback(
        [this, candleBuilder, mdCanId, moveSpeedParam]()
        {
            updateUserChoice();
            auto candle = attachCandle(*(candleBuilder->datarate), *(candleBuilder->busType), true);
            MDCO mdco((*mdCanId), candle);
            MDCO::Error_t err;
            err = mdco.setProfileParameters(*moveSpeedParam.param);
            if (err != MDCO::OK)
            {
                m_log.error("Error setting profile parameters");
                detachCandle(candle);
                return;
            }
            err = mdco.enableDriver(CyclicSyncVelocity);
            if (err != MDCO::OK)
            {
                m_log.error("Error enabling driver");
                detachCandle(candle);
                return;
            }
            mdco.moveSpeed(cmdCANopen.desiredSpeed);
            err = mdco.disableDriver();
            if (err != MDCO::OK)
            {
                m_log.error("Error disabling driver");
                detachCandle(candle);
                return;
            }

            detachCandle(candle);
        });

    // TEST move impedance
    testImpedance = testMove->add_subcommand(
        "impedance",
        "Put the motor into Impedance PD mode "
        "cf:https://mabrobotics.github.io/MD80-x-CANdle-Documentation/"
        "md_x_candle_ecosystem_overview/Motion%20modes.html#impedance-pd.");
    testImpedance
        ->add_option(
            "--speed", cmdCANopen.desiredSpeed, "Sets the target velocity for all motion modes.")
        ->required();
    testImpedance
        ->add_option("--position",
                     cmdCANopen.desiredPos,
                     "Relative position to reach. <0x0, "
                     "0xFFFFFFFF>[inc].")
        ->required();
    testImpedance->add_option("--kp", cmdCANopen.param.kp, "Position gain.")->required();
    testImpedance->add_option("--kd", cmdCANopen.param.kd, "Velocity gain.")->required();
    testImpedance->add_option("--torque_ff", cmdCANopen.param.torqueff, "Torque FF.")->required();

    MovementLimitsOPtions moveImpedanceParam(testImpedance);

    testImpedance->callback(
        [this, candleBuilder, mdCanId, moveImpedanceParam]()
        {
            updateUserChoice();
            auto candle = attachCandle(*(candleBuilder->datarate), *(candleBuilder->busType), true);
            MDCO mdco((*mdCanId), candle);
            MDCO::Error_t err;
            err = mdco.setProfileParameters(*moveImpedanceParam.param);
            if (err != MDCO::OK)
            {
                m_log.error("Error setting profile parameters");
                detachCandle(candle);
                return;
            }
            err = mdco.openSave();
            if (err != MDCO::OK)
            {
                m_log.error("Error enabling driver");
                detachCandle(candle);
                return;
            }
            err = mdco.enableDriver(Impedance);
            if (err != MDCO::OK)
            {
                m_log.error("Error enabling driver");
                detachCandle(candle);
                return;
            }
            err = mdco.moveImpedance(
                cmdCANopen.desiredSpeed, cmdCANopen.desiredPos, cmdCANopen.param, 5000);
            if (err != MDCO::OK)
            {
                m_log.error("Error moving impedance");
                detachCandle(candle);
                return;
            }
            err = mdco.disableDriver();
            if (err != MDCO::OK)
            {
                m_log.error("Error disabling driver");
                detachCandle(candle);
                return;
            }

            detachCandle(candle);
        });

    // TIME STAMP ============================================================================
    timeStamp =
        mdco->add_subcommand("time", "Send a time stamp message using the computer's clock.")
            ->needs(mdCanIdOption);

    timeStamp->callback(
        [this, candleBuilder, mdCanId]()
        {
            updateUserChoice();
            auto candle = attachCandle(*(candleBuilder->datarate), *(candleBuilder->busType), true);
            MDCO mdco((*mdCanId), candle);

            auto now = std::chrono::system_clock::now();

            std::tm epoch_tm  = {};
            epoch_tm.tm_year  = 84;
            epoch_tm.tm_mon   = 0;
            epoch_tm.tm_mday  = 1;
            epoch_tm.tm_hour  = 0;
            epoch_tm.tm_min   = 0;
            epoch_tm.tm_sec   = 0;
            epoch_tm.tm_isdst = -1;

            auto epoch_time_t = std::mktime(&epoch_tm);
            auto epoch_tp     = std::chrono::system_clock::from_time_t(epoch_time_t);
            auto days_since = std::chrono::duration_cast<std::chrono::days>(now - epoch_tp).count();
            std::time_t now_time_t  = std::chrono::system_clock::to_time_t(now);
            std::tm     local_tm    = *std::localtime(&now_time_t);
            std::tm     midnight_tm = local_tm;

            midnight_tm.tm_hour = 0;
            midnight_tm.tm_min  = 0;
            midnight_tm.tm_sec  = 0;

            auto midnight_time_t       = std::mktime(&midnight_tm);
            auto midnight_tp           = std::chrono::system_clock::from_time_t(midnight_time_t);
            long millis_since_midnight = static_cast<long>(
                std::chrono::duration_cast<std::chrono::milliseconds>(now - midnight_tp).count());

            m_log.info("The actual time according to your computer is: %s",
                       std::asctime(&local_tm));
            m_log.info("Number of days since 1st January 1984: %ld", days_since);
            m_log.info("Number of millis since midnight: %ld", millis_since_midnight);

            long TimeMessageId = mdco.getValueFromOpenRegister(0x1012, 0x00);

            std::vector<u8> frame = {
                ((u8)millis_since_midnight),
                ((u8)(millis_since_midnight >> 8)),
                ((u8)(millis_since_midnight >> 16)),
                ((u8)(millis_since_midnight >> 24)),
                ((u8)days_since),
                ((u8)(days_since >> 8)),
            };

            MDCO::Error_t err = mdco.writeOpenPDORegisters(TimeMessageId, frame);
            if (err != MDCO::OK)
            {
                m_log.error("Error sending time message");
                detachCandle(candle);
                return;
            }
            else
            {
                m_log.success("message send");
            }

            detachCandle(candle);
        });
}