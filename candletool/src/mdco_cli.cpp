#include "mdco_cli.hpp"

using namespace mab;

UserCommandCO cmdCANopen;

std::string MdcoCli::validateAndGetFinalConfigPath(const std::string& cfgPath)
{
    std::string finalConfigPath        = cfgPath;
    std::string pathRelToDefaultConfig = getMotorsConfigPath() + cfgPath;
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

    if (!isConfigValid(finalConfigPath))
    {
        m_log.error("\"%s\" in not a valid motor .cfg file.", finalConfigPath.c_str());
        m_log.warn("Valid file must have .cfg extension, and size of < 1MB");
        exit(1);
    }
    std::string defaultConfigPath =
        finalConfigPath.substr(0, finalConfigPath.find_last_of('/') + 1) + "default.cfg";

    if (!fileExists(defaultConfigPath))
    {
        m_log.warn("No default config found at expected location \"%s\"",
                   defaultConfigPath.c_str());
        m_log.warn("Cannot check completeness of the config file. Proceed with upload? [y/n]");
        if (!getConfirmation())
            exit(0);
    }

    if (fileExists(defaultConfigPath) && !isCanOpenConfigComplete(finalConfigPath))
    {
        m_log.m_layer = Logger::ProgramLayer_E::TOP;
        m_log.error("\"%s\" is incomplete.", finalConfigPath.c_str());
        m_log.info("Generate updated file with all required fields? [y/n]");
        // TODO: that is so dumb
        //  if (!getConfirmation())
        //      exit(0);
        finalConfigPath = generateUpdatedConfigFile(finalConfigPath);
        m_log.info("Generated updated file \"%s\"", finalConfigPath.c_str());
        m_log.info("Setup MD with newly generated config? [y/n]");
        // TODO: that is so dumb, again!
        // if (!getConfirmation())
        //     exit(0);
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
        s[write_pos++] = std::tolower(c);
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
            m_log.info("Discovered MDs: ");
            for (const auto& id : mdIds)
            {
                m_log.info("- %d", id);
            }
            detachCandle(candle);
        });

    // EDS ============================================================================
    eds = mdco->add_subcommand("eds", "EDS file analizer and generator.")->excludes(mdCanIdOption);

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
                long Bootloder = mdco.getValueFromOpenRegister(0x200B, 4);
                m_log.info("Bootloder version: %li", Bootloder);
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
            mINI::INIFile      file(getCanOpenConfigPath());
            mINI::INIStructure ini;
            file.read(ini);
            auto candle = attachCandle(*(candleBuilder->datarate), *(candleBuilder->busType), true);
            MDCO mdco((*mdCanId), candle);

            std::string motor_name            = "";
            int         motor_polepairs       = 0;
            int         motor_kv              = 0;
            float       motor_torqueconstant  = 0.0;
            float       motor_gearratio       = 0.0;
            int         motor_torquebandwidth = 0;
            int         motor_shutdowntemp    = 0;

            int limits_ratedtorque     = 0;
            int limits_maxtorque       = 0;
            int limits_ratedcurrent    = 0;
            int limits_maxcurrent      = 0;
            int limits_maxvelocity     = 0;
            int limits_maxposition     = 0;
            int limits_minposition     = 0;
            int limits_maxacceleration = 0;
            int limits_maxdeceleration = 0;

            int profile_acceleration = 0;
            int profile_deceleration = 0;
            int profile_velocity     = 0;

            int outputencoder_outputencoder     = 0;
            int outputencoder_outputencodermode = 0;

            float positionpid_kp     = 0.0;
            float positionpid_ki     = 0.0;
            float positionpid_kd     = 0.0;
            float positionpid_windup = 0.0;

            float velocitypid_kp     = 0.0;
            float velocitypid_ki     = 0.0;
            float velocitypid_kd     = 0.0;
            float velocitypid_windup = 0.0;

            float impedancepd_kp = 0.0;
            float impedancepd_kd = 0.0;

            std::ifstream infile(finalConfigPath);

            if (!infile.is_open())
            {
                std::cerr << "Error: Unable to open config file\n";
                return;
            }

            std::string section, line;
            while (std::getline(infile, line))
            {
                if (line.empty() || line[0] == '#')
                    continue;
                if (line.front() == '[' && line.back() == ']')
                {
                    section = line.substr(1, line.size() - 2);
                    clean(section);
                    continue;
                }
                std::istringstream iss(line);
                std::string        left, right;
                if (!std::getline(iss, left, '='))
                    continue;
                if (!std::getline(iss, right))
                    continue;
                clean(left);
                clean(right);

                std::string fullkey = section.empty() ? left : section + "_" + left;
                if (fullkey == "motor_name")
                    motor_name = right;
                else if (fullkey == "motor_polepairs")
                    motor_polepairs = std::stoi(right);
                else if (fullkey == "motor_kv")
                    motor_kv = std::stoi(right);
                else if (fullkey == "motor_torqueconstant")
                    motor_torqueconstant = std::stod(right);
                else if (fullkey == "motor_gearratio")
                    motor_gearratio = std::stod(right);
                else if (fullkey == "motor_torquebandwidth")
                    motor_torquebandwidth = std::stoi(right);
                else if (fullkey == "motor_shutdowntemp")
                    motor_shutdowntemp = std::stoi(right);

                else if (fullkey == "limits_ratedtorque")
                    limits_ratedtorque = std::stoi(right);
                else if (fullkey == "limits_maxtorque")
                    limits_maxtorque = std::stoi(right);
                else if (fullkey == "limits_ratedcurrent")
                    limits_ratedcurrent = std::stoi(right);
                else if (fullkey == "limits_maxcurrent")
                    limits_maxcurrent = std::stoi(right);
                else if (fullkey == "limits_maxvelocity")
                    limits_maxvelocity = std::stoi(right);
                else if (fullkey == "limits_maxposition")
                    limits_maxposition = std::stoi(right);
                else if (fullkey == "limits_minposition")
                    limits_minposition = std::stoi(right);
                else if (fullkey == "limits_maxacceleration")
                    limits_maxacceleration = std::stoi(right);

                else if (fullkey == "profile_acceleration")
                    profile_acceleration = std::stoi(right);
                else if (fullkey == "profile_deceleration")
                    profile_deceleration = std::stoi(right);
                else if (fullkey == "profile_velocity")
                    profile_velocity = std::stoi(right);

                else if (fullkey == "outputencoder_outputencoder")
                    outputencoder_outputencoder = std::stoi(right);
                else if (fullkey == "outputencoder_outputencodermode")
                    outputencoder_outputencodermode = std::stoi(right);

                else if (fullkey == "positionpid_kp")
                    positionpid_kp = std::stod(right);
                else if (fullkey == "positionpid_ki")
                    positionpid_ki = std::stod(right);
                else if (fullkey == "positionpid_kd")
                    positionpid_kd = std::stod(right);
                else if (fullkey == "positionpid_windup")
                    positionpid_windup = std::stod(right);

                else if (fullkey == "velocitypid_kp")
                    velocitypid_kp = std::stod(right);
                else if (fullkey == "velocitypid_ki")
                    velocitypid_ki = std::stod(right);
                else if (fullkey == "velocitypid_kd")
                    velocitypid_kd = std::stod(right);
                else if (fullkey == "velocitypid_windup")
                    velocitypid_windup = std::stod(right);

                else if (fullkey == "impedancepd_kp")
                    impedancepd_kp = std::stod(right);
                else if (fullkey == "impedancepd_kd")
                    impedancepd_kd = std::stod(right);
            }
            infile.close();

            std::stringstream ss;

            ss << " ---------- value read from config file ---------- " << '\n';
            ss << "motor_name = " << motor_name << '\n'
               << "motor_polepairs = " << motor_polepairs << '\n'
               << "motor_kv = " << motor_kv << '\n'
               << "motor_torqueconstant = " << motor_torqueconstant << '\n'
               << "motor_gearratio = " << motor_gearratio << '\n'
               << "motor_torquebandwidth = " << motor_torquebandwidth << '\n'
               << "motor_shutdowntemp = " << motor_shutdowntemp << "\n\n"

               << "limits_ratedtorque = " << limits_ratedtorque << '\n'
               << "limits_maxtorque = " << limits_maxtorque << '\n'
               << "limits_ratedcurrent = " << limits_ratedcurrent << '\n'
               << "limits_maxcurrent = " << limits_maxcurrent << '\n'
               << "limits_maxvelocity = " << limits_maxvelocity << '\n'
               << "limits_maxposition = " << limits_maxposition << '\n'
               << "limits_minposition = " << limits_minposition << '\n'
               << "limits_maxacceleration = " << limits_maxacceleration << '\n'
               << "limits_maxdeceleration = " << limits_maxdeceleration << "\n\n"

               << "profile_acceleration = " << profile_acceleration << '\n'
               << "profile_deceleration = " << profile_deceleration << '\n'
               << "profile_velocity = " << profile_velocity << "\n\n"

               << "outputencoder_outputencoder = " << outputencoder_outputencoder << '\n'
               << "outputencoder_outputencodermode = " << outputencoder_outputencodermode << "\n\n"

               << "positionpid_kp = " << positionpid_kp << '\n'
               << "positionpid_ki = " << positionpid_ki << '\n'
               << "positionpid_kd = " << positionpid_kd << '\n'
               << "positionpid_windup = " << positionpid_windup << "\n\n"

               << "velocitypid_kp = " << velocitypid_kp << '\n'
               << "velocitypid_ki = " << velocitypid_ki << '\n'
               << "velocitypid_kd = " << velocitypid_kd << '\n'
               << "velocitypid_windup = " << velocitypid_windup << "\n\n"

               << "impedancepd_kp = " << impedancepd_kp << '\n'
               << "impedancepd_kd = " << impedancepd_kd << "\n\n";

            m_log.info("%s\n", ss.str().c_str());

            MDCO::Error_t err;
            err = mdco.writeLongOpenRegisters(0x2000, 0x06, motor_name);
            if (err != MDCO::OK)
            {
                m_log.error("Error setting motor_name");
                return;
            }
            err = mdco.writeOpenRegisters(0x2000, 0x01, motor_polepairs, 4);
            if (err != MDCO::OK)
            {
                m_log.error("Error setting motor_polepairs");
                return;
            }
            uint32_t motor_torqueconstant_as_long;
            std::memcpy(&motor_torqueconstant_as_long, &motor_torqueconstant, sizeof(float));
            err = mdco.writeOpenRegisters(0x2000, 0x02, motor_torqueconstant_as_long, 4);
            if (err != MDCO::OK)
            {
                m_log.error("Error setting motor_torqueconstant");
                return;
            }
            uint32_t motor_gearratio_as_long;
            std::memcpy(&motor_gearratio_as_long, &motor_gearratio, sizeof(float));
            err = mdco.writeOpenRegisters(0x2000, 0x08, motor_gearratio_as_long, 4);
            if (err != MDCO::OK)
            {
                m_log.error("Error setting motor_gearratio");
                return;
            }
            err = mdco.writeOpenRegisters(0x2000, 0x05, motor_torquebandwidth, 2);
            if (err != MDCO::OK)
            {
                m_log.error("Error setting motor_torquebandwidth");
                return;
            }
            err = mdco.writeOpenRegisters(0x2000, 0x07, motor_shutdowntemp, 1);
            if (err != MDCO::OK)
            {
                m_log.error("Error setting motor_shutdowntemp");
                return;
            }
            err = mdco.writeOpenRegisters(0x2005, 0x03, outputencoder_outputencodermode, 1);
            if (err != MDCO::OK)
            {
                m_log.error("Error setting outputencoder_outputencodermode");
                return;
            }
            err = mdco.writeOpenRegisters(0x607D, 0x01, limits_minposition);
            if (err != MDCO::OK)
            {
                m_log.error("Error setting limits_minposition");
                return;
            }
            err = mdco.writeOpenRegisters(0x607D, 0x02, limits_maxposition);
            if (err != MDCO::OK)
            {
                m_log.error("Error setting limits_maxposition");
                return;
            }
            err = mdco.writeOpenRegisters(0x6076, 0x00, 1000);
            if (err != MDCO::OK)
            {
                m_log.error("Error setting limits_ratedtorque");
                return;
            }
            err = mdco.writeOpenRegisters(0x6072, 0x00, limits_maxtorque);
            if (err != MDCO::OK)
            {
                m_log.error("Error setting limits_maxtorque");
                return;
            }
            err = mdco.writeOpenRegisters(0x6075, 0x00, 1000);
            if (err != MDCO::OK)
            {
                m_log.error("Error setting limits_ratedcurrent");
                return;
            }
            err = mdco.writeOpenRegisters(0x6073, 0x00, limits_maxcurrent);
            if (err != MDCO::OK)
            {
                m_log.error("Error setting limits_maxcurrent");
                return;
            }
            err = mdco.writeOpenRegisters(0x6080, 0x00, limits_maxvelocity);
            if (err != MDCO::OK)
            {
                m_log.error("Error setting limits_maxvelocity");
                return;
            }
            err = mdco.writeOpenRegisters(0x60C5, 0x00, limits_maxacceleration);
            if (err != MDCO::OK)
            {
                m_log.error("Error setting limits_maxacceleration");
                return;
            }
            err = mdco.writeOpenRegisters(0x60C6, 0x00, limits_maxdeceleration);
            if (err != MDCO::OK)
            {
                m_log.error("Error setting limits_maxdeceleration");
                return;
            }
            uint32_t positionpid_kp_as_long;
            std::memcpy(&positionpid_kp_as_long, &positionpid_kp, sizeof(float));
            err = mdco.writeOpenRegisters(0x2002, 0x01, positionpid_kp_as_long, 4);
            if (err != MDCO::OK)
            {
                m_log.error("Error setting positionpid_kp");
                return;
            }
            uint32_t positionpid_ki_as_long;
            std::memcpy(&positionpid_ki_as_long, &positionpid_ki, sizeof(float));
            err = mdco.writeOpenRegisters(0x2002, 0x02, positionpid_ki_as_long, 4);
            if (err != MDCO::OK)
            {
                m_log.error("Error setting positionpid_ki");
                return;
            }
            uint32_t positionpid_kd_as_long;
            std::memcpy(&positionpid_kd_as_long, &positionpid_kd, sizeof(float));
            err = mdco.writeOpenRegisters(0x2002, 0x03, positionpid_kd_as_long, 4);
            if (err != MDCO::OK)
            {
                m_log.error("Error setting positionpid_kd");
                return;
            }
            uint32_t positionpid_windup_as_long;
            std::memcpy(&positionpid_windup_as_long, &positionpid_windup, sizeof(float));
            err = mdco.writeOpenRegisters(0x2002, 0x04, positionpid_windup_as_long, 4);
            if (err != MDCO::OK)
            {
                m_log.error("Error setting positionpid_windup");
                return;
            }
            uint32_t velocitypid_kp_as_long;
            std::memcpy(&velocitypid_kp_as_long, &velocitypid_kp, sizeof(float));
            err = mdco.writeOpenRegisters(0x2001, 0x01, velocitypid_kp_as_long, 4);
            if (err != MDCO::OK)
            {
                m_log.error("Error setting velocitypid_kp");
                return;
            }
            uint32_t velocitypid_ki_as_long;
            std::memcpy(&velocitypid_ki_as_long, &velocitypid_ki, sizeof(float));
            err = mdco.writeOpenRegisters(0x2001, 0x02, velocitypid_ki_as_long, 4);
            if (err != MDCO::OK)
            {
                m_log.error("Error setting velocitypid_ki");
                return;
            }
            uint32_t velocitypid_kd_as_long;
            std::memcpy(&velocitypid_kd_as_long, &velocitypid_kd, sizeof(float));
            err = mdco.writeOpenRegisters(0x2001, 0x03, velocitypid_kd_as_long, 4);
            if (err != MDCO::OK)
            {
                m_log.error("Error setting velocitypid_kd");
                return;
            }
            uint32_t velocitypid_windup_as_long;
            std::memcpy(&velocitypid_windup_as_long, &velocitypid_windup, sizeof(float));
            err = mdco.writeOpenRegisters(0x2001, 0x04, velocitypid_windup_as_long, 4);
            if (err != MDCO::OK)
            {
                m_log.error("Error setting velocitypid_windup");
                return;
            }
            uint32_t impedancepd_kp_as_long;
            std::memcpy(&impedancepd_kp_as_long, &impedancepd_kp, sizeof(float));
            err = mdco.writeOpenRegisters(0x200C, 0x01, impedancepd_kp, 4);
            if (err != MDCO::OK)
            {
                m_log.error("Error setting impedancepd_kp");
                return;
            }
            uint32_t impedancepd_kd_as_long;
            std::memcpy(&impedancepd_kd_as_long, &impedancepd_kd, sizeof(float));
            err = mdco.writeOpenRegisters(0x200C, 0x02, impedancepd_kd, 4);
            if (err != MDCO::OK)
            {
                m_log.error("Error setting impedancepd_kd");
                return;
            }
            m_log.info("Don't forget to save the config before shutting down the MD!");

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
            mINI::INIStructure readIni; /*< mINI structure for read data */
            MDRegisters_S      regs;    /*< read register */
            std::string        configName = cmdCANopen.value;
            mdco.m_timeout                = 10;

            long raw_data = 0;
            /*────────────────────────────
              VARIABLES  –  section [motor]
            ────────────────────────────*/
            std::vector<u8> motor_name;
            mdco.readLongOpenRegisters(0x2000, 0x06, motor_name);
            int   motor_kv         = 100;
            u32   motor_pole_pairs = mdco.getValueFromOpenRegister(0x2000, 0x01);
            float motor_torque_constant;
            raw_data = (float)mdco.getValueFromOpenRegister(0x2000, 0x02);
            std::memcpy(&motor_torque_constant, &raw_data, sizeof(float));
            raw_data = (float)mdco.getValueFromOpenRegister(0x2000, 0x08);
            float motor_gear_ratio;
            std::memcpy(&motor_gear_ratio, &raw_data, sizeof(float));
            u16 motor_torque_bandwidth = mdco.getValueFromOpenRegister(0x2000, 0x05);
            u32 motor_shutdown_temp    = mdco.getValueFromOpenRegister(0x2000, 0x07);

            /*────────────────────────────
              section [limits]
            ────────────────────────────*/
            u16 limits_max_current      = mdco.getValueFromOpenRegister(0x6073, 0x00);
            u32 limits_rated_current    = mdco.getValueFromOpenRegister(0x6075, 0x00);
            u16 limits_max_torque       = mdco.getValueFromOpenRegister(0x6072, 0x00);
            u32 limits_rated_torque     = mdco.getValueFromOpenRegister(0x6076, 0x00);
            u32 limits_max_velocity     = mdco.getValueFromOpenRegister(0x6080, 0x00);
            u32 limits_max_position     = mdco.getValueFromOpenRegister(0x607D, 0x02);
            u32 limits_min_position     = mdco.getValueFromOpenRegister(0x607D, 0x01);
            u32 limits_max_acceleration = mdco.getValueFromOpenRegister(0x60C5, 0x00);
            u32 limits_max_deceleration = mdco.getValueFromOpenRegister(0x60C6, 0x00);

            /*────────────────────────────
              section [profile]
            ────────────────────────────*/
            u32 profile_acceleration = 0.0f;
            raw_data                 = mdco.getValueFromOpenRegister(0x2008, 0x04);
            std::memcpy(&profile_acceleration, &raw_data, sizeof(float));
            u32 profile_deceleration = 0.0f;
            raw_data                 = mdco.getValueFromOpenRegister(0x2008, 0x05);
            std::memcpy(&profile_deceleration, &raw_data, sizeof(float));
            u32 profile_velocity = 0.0f;
            raw_data             = mdco.getValueFromOpenRegister(0x2008, 0x03);
            std::memcpy(&profile_velocity, &raw_data, sizeof(float));

            /*────────────────────────────
              section [output encoder]
            ────────────────────────────*/
            short output_encoder = 0;
            raw_data             = mdco.getValueFromOpenRegister(0x2005, 0x01);
            std::memcpy(&output_encoder, &raw_data, sizeof(short));
            short output_encoder_mode = 0;
            raw_data                  = mdco.getValueFromOpenRegister(0x2005, 0x03);
            std::memcpy(&output_encoder_mode, &raw_data, sizeof(short));

            /*────────────────────────────
              section [position pid]
            ────────────────────────────*/
            float pos_kp = 0.0f;
            raw_data     = mdco.getValueFromOpenRegister(0x2002, 0x01);
            std::memcpy(&pos_kp, &raw_data, sizeof(float));
            float pos_ki = 0.0f;
            raw_data     = mdco.getValueFromOpenRegister(0x2002, 0x02);
            std::memcpy(&pos_ki, &raw_data, sizeof(float));
            float pos_kd = 0.0f;
            raw_data     = mdco.getValueFromOpenRegister(0x2002, 0x03);
            std::memcpy(&pos_kd, &raw_data, sizeof(float));
            float pos_windup = 0.0f;
            raw_data         = mdco.getValueFromOpenRegister(0x2002, 0x04);
            std::memcpy(&pos_windup, &raw_data, sizeof(float));

            /*────────────────────────────
              section [velocity pid]
            ────────────────────────────*/
            float vel_kp = 0.0f;
            raw_data     = mdco.getValueFromOpenRegister(0x2001, 0x01);
            std::memcpy(&vel_kp, &raw_data, sizeof(float));
            float vel_ki = 0.0f;
            raw_data     = mdco.getValueFromOpenRegister(0x2001, 0x02);
            std::memcpy(&vel_ki, &raw_data, sizeof(float));
            float vel_kd = 0.0f;
            raw_data     = mdco.getValueFromOpenRegister(0x2001, 0x03);
            std::memcpy(&vel_kd, &raw_data, sizeof(float));
            float vel_windup = 0.0f;
            raw_data         = mdco.getValueFromOpenRegister(0x2001, 0x04);
            std::memcpy(&vel_windup, &raw_data, sizeof(float));

            /*────────────────────────────
              section [impedance pd]
            ────────────────────────────*/
            float imp_kp = 0.0f;
            raw_data     = mdco.getValueFromOpenRegister(0x200C, 0x01);
            std::memcpy(&imp_kp, &raw_data, sizeof(float));
            float imp_kd = 0.0f;
            raw_data     = mdco.getValueFromOpenRegister(0x200C, 0x02);
            std::memcpy(&imp_kd, &raw_data, sizeof(float));

            // ────────────────────────────────────────────────────────────
            // ÉCRITURE DU FICHIER CONFIG
            // ────────────────────────────────────────────────────────────
            std::ofstream cfg(configName);
            if (!cfg.is_open())
            {
                std::cerr << "Impossible to open" << configName << " in wrting mode.\n";
                return;
            }

            cfg << std::fixed << std::setprecision(5);

            // ----- [motor] -----
            cfg << "[motor]\n";
            cfg << "name = " << std::string(motor_name.begin(), motor_name.end()) << '\n';
            cfg << "pole pairs = " << motor_pole_pairs << '\n';
            cfg << "kv = " << motor_kv << '\n';
            cfg << "torque constant = " << motor_torque_constant << '\n';
            cfg << "gear ratio = " << motor_gear_ratio << '\n';
            cfg << "torque bandwidth = " << motor_torque_bandwidth << '\n';
            cfg << "shutdown temp = " << motor_shutdown_temp << "\n\n";

            // ----- [limits] -----
            cfg << "[limits]\n";
            cfg << "rated current = " << limits_rated_current << '\n';
            cfg << "max current = " << limits_max_current << '\n';
            cfg << "rated torque = " << limits_rated_torque << '\n';
            cfg << "max torque = " << limits_max_torque << '\n';
            cfg << "max velocity = " << limits_max_velocity << '\n';
            cfg << "max position = " << limits_max_position << '\n';
            cfg << "min position = " << limits_min_position << '\n';
            cfg << "max acceleration = " << limits_max_acceleration << '\n';
            cfg << "max deceleration = " << limits_max_deceleration << "\n\n";

            // ----- [profile] -----
            cfg << "[profile]\n";
            cfg << "acceleration = " << profile_acceleration << '\n';
            cfg << "deceleration = " << profile_deceleration << '\n';
            cfg << "velocity = " << profile_velocity << "\n\n";

            // ----- [output encoder] -----
            cfg << "[output encoder]\n";
            cfg << "output encoder = " << output_encoder << '\n';
            cfg << "output encoder mode = " << output_encoder_mode << "\n\n";

            // ----- [position pid] -----
            cfg << "[position pid]\n";
            cfg << "kp = " << pos_kp << '\n';
            cfg << "ki = " << pos_ki << '\n';
            cfg << "kd = " << pos_kd << '\n';
            cfg << "windup = " << pos_windup << "\n\n";

            // ----- [velocity pid] -----
            cfg << "[velocity pid]\n";
            cfg << "kp = " << vel_kp << '\n';
            cfg << "ki = " << vel_ki << '\n';
            cfg << "kd = " << vel_kd << '\n';
            cfg << "windup = " << vel_windup << "\n\n";

            // ----- [impedance pd] -----
            cfg << "[impedance pd]\n";
            cfg << "kp = " << imp_kp << '\n';
            cfg << "kd = " << imp_kd << "\n\n";

            cfg.close();
            m_log.info("Fichier %s généré avec succès.\n ", configName.c_str());

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
            u64  latence_totale = 0;
            bool testOk         = true;

            for (int i = 0; i < 100; ++i)
            {
                auto start = std::chrono::steady_clock::now();

                if (mdco.readOpenRegisters(0x1000, 0) != MDCO::Error_t::OK)
                {
                    testOk = false;
                }

                auto end = std::chrono::steady_clock::now();
                auto duree =
                    std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
                latence_totale += static_cast<u64>(duree);
            }

            if (testOk)
            {
                u64 moyenne = latence_totale / 100;
                m_log.info("---------------Latence---------------\n");
                m_log.info("Total: %lu µs\n", latence_totale);
                m_log.info("Result (average of 100 attempts): %lu µs\n", moyenne);
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

    MovementLimitsOPtions moveAbsParm(testMoveAbs);

    testMoveAbs->callback(
        [this, candleBuilder, mdCanId, moveAbsParm]()
        {
            updateUserChoice();
            auto candle = attachCandle(*(candleBuilder->datarate), *(candleBuilder->busType), true);
            MDCO mdco((*mdCanId), candle);
            MDCO::Error_t err;
            err = mdco.setProfileParameters(*moveAbsParm.param);
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
