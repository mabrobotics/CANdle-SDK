#include "candle.hpp"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>

#include "candle_protocol.hpp"
#include "register.hpp"

namespace mab
{
    class mystreambuf : public std::streambuf
    {
    };
    mystreambuf  nostreambuf;
    std::ostream nocout(&nostreambuf);
#define vout ((this->printVerbose) ? std::cout << "[CANDLE] " : nocout)
    std::string statusOK   = "  \033[1;32m[OK]\033[0m";
    std::string statusFAIL = "  \033[1;31m[FAILED]\033[0m";

    uint64_t getTimestamp()
    {
        using namespace std::chrono;
        return duration_cast<microseconds>(system_clock::now().time_since_epoch()).count();
    }

    std::vector<Candle*> Candle::instances = std::vector<Candle*>();

    Candle::Candle(CANdleBaudrate_E  canBaudrate,
                   bool              printVerbose,
                   mab::BusType_E    busType,
                   const std::string device)
        : Candle(canBaudrate, printVerbose, makeBus(busType, device))
    {
        log.m_tag   = "Candle";
        log.m_layer = Logger::ProgramLayer_E::TOP;
    }

    Candle::Candle(CANdleBaudrate_E canBaudrate, bool printVerbose, std::shared_ptr<Bus> bus)
        : printVerbose(printVerbose), bus(bus)
    {
        reset();
        usleep(5000);
        if (sem_init(&received, 0, 0) == -1)
            throw std::runtime_error("Failed to set up receive semaphore");
        for (u32 i = 0; i < 10; i++)
        {
            if (!sendBusFrame(BUS_FRAME_END, 100))
                log.error("Candle not responding");
            else
            {
                log.info("Bus communication functional");
                break;
            }
        }

        if (!configCandleBaudrate(canBaudrate, true))
        {
            log.error("Failed to set up CANdle baudrate @%dMbps", canBaudrate);
            throw std::runtime_error("Failed to set up CANdle baudrate!");
        }
        if (bus->getType() == mab::BusType_E::USB)
            log.info("CANdle 0x%x ready (USB)", getDeviceId());
        else if (bus->getType() == mab::BusType_E::SPI)
            log.info("CANdle ready (SPI)");
        else if (bus->getType() == mab::BusType_E::UART)
            log.info("CANdle ready (UART)");
        md80Register = std::make_shared<Register>(this);
        Candle::instances.push_back(this);
    }

    Candle::~Candle()
    {
        if (inUpdateMode())
            end();
        transmitterThread.request_stop();
        sem_destroy(&received);
    }

    std::shared_ptr<Bus> Candle::makeBus(mab::BusType_E busType, std::string device)
    {
        switch (busType)
        {
            case mab::BusType_E::USB:
            {
                std::vector<u32> idsToIgnore;

                for (Candle* instance : Candle::instances)
                {
                    if (instance->bus->getType() == BusType_E::USB)
                        idsToIgnore.push_back(instance->bus->getId());
                }

                if (idsToIgnore.size() == 0 && searchMultipleDevicesOnUSB(candlePid, candleVid) > 1)
                {
                    log.warn(
                        "Multiple CANdle detected! If ID is unspecified in the constructor, the "
                        "one with the smallest ID will be used by default!");
                }

                std::shared_ptr<UsbDevice> usb =
                    std::make_shared<UsbDevice>(candleVid, candlePid, idsToIgnore, device);

                if (!usb->isConnected())
                {
                    log.warn("Failed to connect to CANdle device! Trying bootloader mode...");
                    usb = std::make_shared<UsbDevice>(candleVid, bootloaderPid);

                    if (!usb->isConnected())
                    {
                        log.error("Unable to connect to Candle device!");
                        exit(1);
                    }

                    log.warn("Connected to CANdle in bootloader mode!");
                }

                return usb;
            }
#ifdef UNIX
            case mab::BusType_E::SPI:
            {
                if (device != "")
                    return std::make_shared<SpiDevice>(device);
                else
                    return std::make_shared<SpiDevice>();
            }
            case mab::BusType_E::UART:
            {
                if (device != "")
                    return std::make_shared<UartDevice>(device);
                else
                    return std::make_shared<UartDevice>();
            }
#endif
            default:
                throw std::runtime_error("Error wrong bus type specified!");
        }
        return nullptr;
    }

    const std::string Candle::getVersion()
    {
        return getVersionString({{CANDLE_VTAG, CANDLE_VREVISION, CANDLE_VMINOR, CANDLE_VMAJOR}});
    }

    int Candle::getActualCommunicationFrequency()
    {
        return static_cast<int>(usbCommsFreq);
    }

    void Candle::setTransmitDelayUs(uint32_t delayUs)
    {
        transmitterDelay = delayUs < 20 ? 20 : delayUs;
    }

    void Candle::transfer(std::stop_token stop_token)
    {
        int      counter        = 0;
        uint64_t freqCheckStart = getTimestamp();
        while (!shouldStopTransmitter || stop_token.stop_requested())
        {
            if (++counter == 250)
            {
                usbCommsFreq   = 250.0 / (float)(getTimestamp() - freqCheckStart) * 1000000.0f;
                freqCheckStart = getTimestamp();
                counter        = 0;
            }
            transmitNewStdFrame();
            msgsSent++;
            if (bus->receive(sizeof(StdMd80ResponseFrame_t) * md80s.size() + 1), 1)
            {
                if (*bus->getRxBuffer() == BUS_FRAME_UPDATE)
                    manageReceivedFrame();
            }
        }
    }

    void Candle::manageReceivedFrame()
    {
        for (size_t i = 0; i < md80s.size(); i++)
            md80s[i].__updateResponseData(
                (StdMd80ResponseFrame_t*)bus->getRxBuffer(1 + i * sizeof(StdMd80ResponseFrame_t)));
    }

    unsigned long Candle::getDeviceId()
    {
        return bus->getId();
    }

    void Candle::updateMd80State(mab::Md80& drive)
    {
        Md80::State state{};

        md80Register->read(drive.getId(),
                           Md80Reg_E::mainEncoderPosition,
                           state.position,
                           Md80Reg_E::mainEncoderVelocity,
                           state.velocity,
                           Md80Reg_E::motorTorque,
                           state.torque,
                           Md80Reg_E::outputEncoderPosition,
                           state.outputEncoderPosition,
                           Md80Reg_E::outputEncoderVelocity,
                           state.outputEncoderVelocity,
                           Md80Reg_E::motorTemperature,
                           state.temperature,
                           Md80Reg_E::quickStatus,
                           state.quickStatus);

        drive.__updateResponseData(state);
    }

    bool Candle::addMd80(uint16_t canId, bool printFailure)
    {
        if (inUpdateMode())
            return false;

        for (auto& md : md80s)
        {
            if (md.getId() == canId)
            {
                log.success("MD80 with ID: %d is already on the update list.", canId);
                return true;
            }
        }

        if (md80s.size() >= maxDevices)
        {
            log.error("Cannot add more than %d MDs!", maxDevices);
            return false;
        }

        char payload[2]{};
        *(uint16_t*)payload = canId;

        if (sendBusFrame(BUS_FRAME_MD80_ADD, 2, payload, 3, 2))
        {
            version_ut firmwareVersion = {{0, 0, 0, 0}};

            if (!md80Register->read(canId, Md80Reg_E::firmwareVersion, firmwareVersion.i))
            {
                log.error(
                    "Unable to read MD80's firmware version! Please check the ID, or update "
                    "the MD80 with MAB_CAN_Flasher.");
                return false;
            }

            if (firmwareVersion.i < md80CompatibleVersion.i)
            {
                log.warn(
                    "MD80 firmware (ID: %d) is outdated. Please see the manual for intructions on "
                    "how to update.",
                    canId);
            }
            else if (firmwareVersion.s.major > md80CompatibleVersion.s.major ||
                     firmwareVersion.s.minor > md80CompatibleVersion.s.minor)
            {
                log.warn("MD80 firmware (ID: %d) is a future version.", canId);
            }
            log.success("Added MD80 (ID: %d)", canId);
            md80s.push_back(Md80(canId));
            mab::Md80& newDrive = md80s.back();
            updateMd80State(newDrive);
            return true;
        }

        if (printFailure)
            log.error("Failed to add MD80 (ID: %d)", canId);
        return false;
    }

    std::vector<uint16_t> Candle::ping(mab::CANdleBaudrate_E baudrate)
    {
        if (!configCandleBaudrate(baudrate))
            return std::vector<uint16_t>();

        log.info("Starting pinging drives at baudrate: %dM", baudrate);
        std::vector<uint16_t> ids{};

        if (sendBusFrame(BUS_FRAME_PING_START, 2000, nullptr, 2, 33))
        {
            uint16_t* idsPointer = (uint16_t*)bus->getRxBuffer(1);
            for (int i = 0; i < maxDevices; i++)
            {
                uint16_t id = idsPointer[i];
                if (id == 0x00)
                    break;
                ids.push_back(id);
            }
            if (ids.size() == 0)
            {
                log.info("No drives found");
                return ids;
            }
            log.info("Found drives.");
            for (size_t i = 0; i < ids.size(); i++)
            {
                if (ids[i] == 0)
                    break;  // No more ids in the message
                log.info("%d: ID = %d (Ox%x)", (i + 1), ids[i], ids[i]);
                if (ids[i] > idMax)
                {
                    log.warn(
                        "ID is invalid! Probably two or more drives share same ID."
                        "Communication will most likely be broken until IDs are unique!");
                    std::vector<uint16_t> empty;
                    return empty;
                }
            }
        }
        return ids;
    }

    std::vector<BusDevice_S> Candle::pingNew(mab::CANdleBaudrate_E baudrate)
    {
        if (!configCandleBaudrate(baudrate))
            return std::vector<BusDevice_S>();

        // Same TX Buffer for all messages...
        const char txBuffer[]   = {FRAME_GET_INFO, 0x00};
        char       rxBuffer[64] = {0};

        for (uint16_t canId = 10; canId < 2075; canId++)
        {
            if (sendGenericFDCanFrame(canId, sizeof(txBuffer), txBuffer, rxBuffer, nullptr, 1))
                log.debug("Pinging ID [ %u ] OK", canId);
        }

        log.debug("Finish!");

        return std::vector<BusDevice_S>();
    }

    std::vector<uint16_t> Candle::ping()
    {
        return ping(m_canBaudrate);
    }
    bool Candle::sendGenericFDCanFrame(uint16_t    canId,
                                       int         msgLen,
                                       const char* txBuffer,
                                       char*       rxBuffer,
                                       size_t*     pRxLength,
                                       int         timeoutMs)
    {
        GenericMd80Frame64 frame;

        size_t rxLength = 0;

        frame.frameId    = mab::BusFrameId_t::BUS_FRAME_MD80_GENERIC_FRAME;
        frame.driveCanId = canId;
        frame.canMsgLen  = msgLen;
        frame.timeoutMs  = timeoutMs < 3 ? 3 : timeoutMs - 3;
        memcpy(frame.canMsg, txBuffer, msgLen);
        char tx[96];
        int  len = sizeof(frame) - sizeof(frame.canMsg) + msgLen;
        memcpy(tx, &frame, len);
        bool waitForResponse = rxBuffer != nullptr;
        if (bus->transmit(tx, len, waitForResponse, timeoutMs, 66, false))  // Got some response
        {
            if (!waitForResponse)
                return true;
            rxLength = bus->getBytesReceived();
            if (*bus->getRxBuffer(0) == tx[0] &&                     // USB Frame ID matches
                *bus->getRxBuffer(1) == true && rxLength <= 64 + 2)  // response can ID matches
            {
                memcpy(rxBuffer, bus->getRxBuffer(2), rxLength - 2);
                if (pRxLength != nullptr)
                    *pRxLength = rxLength;
                return true;
            }
        }
        return false;
    }

    bool Candle::configMd80Can(uint16_t         canId,
                               uint16_t         newId,
                               CANdleBaudrate_E newBaudrateMbps,
                               unsigned int     newTimeout,
                               bool             canTermination)
    {
        if (newId < 10 || newId > idMax)
        {
            log.error("CAN config change failed, ID out of range! Valid ID range: <10-2000>");
            return false;
        }

        if (!md80Register->write(canId,
                                 Md80Reg_E::canId,
                                 (uint32_t)newId,
                                 Md80Reg_E::canBaudrate,
                                 newBaudrateMbps * 1000000,
                                 Md80Reg_E::canWatchdog,
                                 newTimeout,
                                 Md80Reg_E::canTermination,
                                 (uint8_t)canTermination,
                                 Md80Reg_E::runCanReinit,
                                 true))
        {
            log.error("CAN config change failed!");
            return false;
        }
        log.info("Drive ID: %d was changed to ID: %d.", canId, newId);
        log.info("Drive CAN baudrate is now: %dMbps", newBaudrateMbps);
        log.info("Drive CAN timeout is now: %sms",
                 (newTimeout == 0) ? "disabled" : std::to_string(newTimeout).c_str());
        log.info("Drive CAN termination is %s", canTermination ? "enabled" : "disabled");
        log.success("CAN config change successful!");
        return true;
    }

    bool Candle::configMd80Save(uint16_t canId)
    {
        return executeCommand(canId,
                              Md80Reg_E::runSaveCmd,
                              "Saving in flash failed at ID: ",
                              "Saving in flash successful at ID: ", false);
    }

    bool Candle::configMd80Blink(uint16_t canId)
    {
        return executeCommand(
            canId, Md80Reg_E::runBlink, "Blinking failed at ID: ", "LEDs blining at ID:", false);
    }

    bool Candle::controlMd80SetEncoderZero(uint16_t canId)
    {
        return executeCommand(canId,
                              Md80Reg_E::runZero,
                              "Setting new zero position failed at ID: ",
                              "Setting new zero position successful at ID: ", false);
    }

    bool Candle::configMd80SetCurrentLimit(uint16_t canId, float currentLimit)
    {
        if (inUpdateMode() || !md80Register->write(canId, Md80Reg_E::motorIMax, currentLimit))
        {
            log.error("Setting new current limit failed (ID: %d)", canId);
            return false;
        }
        log.success("Setting new current limit succesfull (ID: %d)", canId);
        return true;
    }

    bool Candle::configCandleBaudrate(CANdleBaudrate_E canBaudrate, bool printVersionInfo)
    {
        this->m_canBaudrate = canBaudrate;

        char payload[1]{};
        payload[0] = static_cast<uint8_t>(m_canBaudrate);

        if (sendBusFrame(BUS_FRAME_CANDLE_CONFIG_BAUDRATE, 50, payload, 2, 6))
        {
            version_ut candleDeviceVersion{};
            candleDeviceVersion.i = *(uint32_t*)bus->getRxBuffer(2);

            if (printVersionInfo)
            {
                if (candleDeviceVersion.i < candleDeviceCompatibleVersion.i)
                {
                    log.warn(
                        "Your CANdle device firmware is outdated. Please see the "
                        "manual for intructions on how to update.");
                }
                log.info("CANdle firmware v%s", mab::getVersionString(candleDeviceVersion).c_str());
            }
            return true;
        }
        return false;
    }

    bool Candle::configMd80TorqueBandwidth(uint16_t canId, uint16_t torqueBandwidth)
    {
        if (inUpdateMode() || !md80Register->write(canId,
                                                   Md80Reg_E::motorTorgueBandwidth,
                                                   torqueBandwidth,
                                                   Md80Reg_E::runCalibratePiGains,
                                                   true))
        {
            log.error("Bandwidth change failed (ID: %d)", canId);
            return false;
        }
        log.success("Bandwidth succesfully %.0dchanged (ID: %d)", canId);
        return true;
    }

    Md80& Candle::getMd80FromList(uint16_t id)
    {
        for (auto& md : md80s)
            if (md.getId() == id)
                return md;
        throw std::runtime_error("getMd80FromList(id): Id not found on the list!");
    }
    bool Candle::controlMd80SetEncoderZero(Md80& drive)
    {
        return controlMd80SetEncoderZero(drive.getId());
    }
    bool Candle::controlMd80Enable(Md80& drive, bool enable)
    {
        return controlMd80Enable(drive.getId(), enable);
    }
    bool Candle::controlMd80Mode(Md80& drive, Md80Mode_E mode)
    {
        return controlMd80Mode(drive.getId(), mode);
    }
    bool Candle::controlMd80Mode(uint16_t canId, Md80Mode_E mode)
    {
        Md80& drive = getMd80FromList(canId);

        if (inUpdateMode() ||
            !md80Register->write(canId, Md80Reg_E::motionModeCommand, static_cast<uint8_t>(mode)))
        {
            log.error("Setting control mode failed (ID: %d)", canId);
            return false;
        }
        log.success("Setting control mode successful (ID: %d)", canId);
        drive.__setControlMode(mode);
        return true;
    }
    bool Candle::controlMd80Enable(uint16_t canId, bool enable)
    {
        uint16_t controlword = enable ? 39 : 64;
        if (inUpdateMode() || !md80Register->write(canId, Md80Reg_E::state, controlword))
        {
            log.error("%s failed (ID: %d)", (enable ? "Enabling" : "Disabling"), canId);
            return false;
        }

        if (enable)
        {
            mab::Md80Mode_E mode{mab::Md80Mode_E::POSITION_PID};
            if (!md80Register->read(canId, Md80Reg_E::motionModeStatus, mode))
                throw std::runtime_error("Could not read motion mode from the driver");
            if (mode == mab::Md80Mode_E::IDLE)
            {
                log.warn("Drive %d has no motion mode set, it will be idle", canId);
            }
        }
        log.success("%s succesfull (ID: %d)", (enable ? "Enabling" : "Disabling"), canId);
        return true;
    }
    bool Candle::begin()
    {
        if (mode == CANdleMode_E::UPDATE)
        {
            log.warn("Cannot call 'begin()', already in update mode.");
            return false;
        }

        if (sendBusFrame(BUS_FRAME_BEGIN, 10))
        {
            log.success("Beginnig auto update loop mode");
            mode                  = CANdleMode_E::UPDATE;
            shouldStopTransmitter = false;
            msgsSent              = 0;
            msgsReceived          = 0;

            log.info("Starting transfer thread...");
            // bind_front used to enable stop_token in jthread, jthread used to avoid unexpected
            // termination
            transmitterThread = std::jthread(std::bind_front(&Candle::transfer, this));

            return true;
        }
        log.error("Failed to begin auto update loop mode");
        return false;
    }
    bool Candle::end()
    {
        if (mode == CANdleMode_E::CONFIG)
            return false;

        shouldStopTransmitter = true;
        sem_post(&received);
        if (transmitterThread.joinable())
            transmitterThread.join();

        bus->flushReceiveBuffer();

        if (sendBusFrame(BUS_FRAME_END, 100))
            mode = CANdleMode_E::CONFIG;

        for (auto& md : md80s)
            controlMd80Enable(md, false);

        if (mode == CANdleMode_E::CONFIG)
        {
            log.success("Ending auto update loop");
            return true;
        }
        log.error("Failed to end auto update loop.");
        return false;
    }
    bool Candle::reset()
    {
        return sendBusFrame(BUS_FRAME_RESET, 100);
    }
    bool Candle::inUpdateMode()
    {
        return mode == CANdleMode_E::UPDATE;
    }
    void Candle::transmitNewStdFrame()
    {
        char tx[1 + sizeof(StdMd80CommandFrame_t) * maxDevices];
        tx[0] = BUS_FRAME_UPDATE;

        for (size_t i = 0; i < md80s.size(); i++)
        {
            md80s[i].__updateCommandFrame();
            *(StdMd80CommandFrame_t*)&tx[1 + i * sizeof(StdMd80CommandFrame_t)] =
                md80s[i].__getCommandFrame();
        }

        uint32_t cmdSize  = 1 + md80s.size() * sizeof(StdMd80CommandFrame_t);
        uint32_t respSize = 1 + md80s.size() * sizeof(StdMd80ResponseFrame_t);

        if (bus->getType() == BusType_E::SPI)
            bus->transfer(tx, cmdSize, respSize);
        else if (!bus->transmit(tx, cmdSize, false, 100, respSize))
            throw std::runtime_error("Failed to transmit to candle!");
    }

    bool Candle::setupMd80Calibration(uint16_t canId)
    {
        return executeCommand(canId,
                              Md80Reg_E::runCalibrateCmd,
                              "Starting calibration failed at ID: ",
                              "Starting calibration at ID: ", true);
    }

    bool Candle::setupMd80CalibrationOutput(uint16_t canId)
    {
        return executeCommand(canId,
                              Md80Reg_E::runCalibrateOutpuEncoderCmd,
                              "Starting output encoder calibration failed at ID: ",
                              "Starting output encoder calibration at ID: ", true);
    }

    bool Candle::setupMd80TestOutputEncoder(uint16_t canId)
    {
        return executeCommand(canId,
                              Md80Reg_E::runTestOutputEncoderCmd,
                              "Output encoder test failed at ID: ",
                              "Output encoder test in progress at ID: ", true);
    }

    bool Candle::setupMd80TestMainEncoder(uint16_t canId)
    {
        return executeCommand(canId,
                              Md80Reg_E::runTestMainEncoderCmd,
                              "Main encoder test failed at ID: ",
                              "Main encoder test in progress at ID: ", true);
    }

    bool Candle::setupMd80PerformHoming(uint16_t canId)
    {
        return executeCommand(canId,
                              Md80Reg_E::runHoming,
                              "Homing test failed at ID: ",
                              "Homing test in progress at ID: ", true);
    }

    bool Candle::setupMd80PerformReset(uint16_t canId)
    {
        return executeCommand(
            canId, Md80Reg_E::runReset, "Reset failed at ID: ", "Reset in progress at ID: ", false);
    }

    bool Candle::setupMd80ClearErrors(uint16_t canId)
    {
        return !inUpdateMode() && md80Register->write(canId, Md80Reg_E::runClearErrors, true);
    }

    bool Candle::setupMd80ClearWarnings(uint16_t canId)
    {
        return !inUpdateMode() && md80Register->write(canId, Md80Reg_E::runClearWarnings, true);
    }

    bool Candle::setupMd80DiagnosticExtended(uint16_t canId)
    {
        regRead_st& regR = getMd80FromList(canId).getReadReg();

        if (inUpdateMode() || !md80Register->read(canId,
                                                  Md80Reg_E::motorName,
                                                  regR.RW.motorName,
                                                  Md80Reg_E::buildDate,
                                                  regR.RO.buildDate,
                                                  Md80Reg_E::commitHash,
                                                  regR.RO.commitHash,
                                                  Md80Reg_E::firmwareVersion,
                                                  regR.RO.firmwareVersion,
                                                  Md80Reg_E::motorResistance,
                                                  regR.RO.resistance,
                                                  Md80Reg_E::motorInductance,
                                                  regR.RO.inductance))
        {
            log.error("Extended diagnostic failed (ID: %d)", canId);
            return false;
        }

        if (!md80Register->read(canId,
                                Md80Reg_E::motorIMax,
                                regR.RW.iMax,
                                Md80Reg_E::motorPolePairs,
                                regR.RW.polePairs,
                                Md80Reg_E::motorKt,
                                regR.RW.motorKt,
                                Md80Reg_E::motorGearRatio,
                                regR.RW.gearRatio,
                                Md80Reg_E::bridgeType,
                                regR.RO.bridgeType,
                                Md80Reg_E::canWatchdog,
                                regR.RW.canWatchdog,
                                Md80Reg_E::motorTorgueBandwidth,
                                regR.RW.torqueBandwidth,
                                Md80Reg_E::canBaudrate,
                                regR.RW.canBaudrate,
                                Md80Reg_E::quickStatus,
                                regR.RO.quickStatus,
                                Md80Reg_E::mosfetTemperature,
                                regR.RO.mosfetTemperature,
                                Md80Reg_E::motorKV,
                                regR.RW.motorKV,
                                Md80Reg_E::legacyHardwareVersion,
                                regR.RO.legacyHardwareVersion,
                                Md80Reg_E::hardwareType,
                                regR.RO.hardwareType))
        {
            log.error("Extended diagnostic failed (ID: %d)", canId);
            return false;
        }

        if (!md80Register->read(canId,
                                Md80Reg_E::motorStiction,
                                regR.RW.stiction,
                                Md80Reg_E::motorFriction,
                                regR.RW.friction,
                                Md80Reg_E::outputEncoder,
                                regR.RW.outputEncoder,
                                Md80Reg_E::outputEncoderDir,
                                regR.RW.outputEncoderDir,
                                Md80Reg_E::outputEncoderDefaultBaud,
                                regR.RW.outputEncoderDefaultBaud,
                                Md80Reg_E::motorTemperature,
                                regR.RO.motorTemperature,
                                Md80Reg_E::motorShutdownTemp,
                                regR.RW.motorShutdownTemp,
                                Md80Reg_E::canTermination,
                                regR.RW.canTermination,
                                Md80Reg_E::outputEncoderPosition,
                                regR.RO.outputEncoderPosition,
                                Md80Reg_E::outputEncoderVelocity,
                                regR.RO.outputEncoderVelocity))
        {
            log.error("Extended diagnostic failed (ID: %d)", canId);
            return false;
        }

        if (!md80Register->read(canId,
                                Md80Reg_E::outputEncoderMode,
                                regR.RW.outputEncoderMode,
                                Md80Reg_E::calOutputEncoderStdDev,
                                regR.RO.calOutputEncoderStdDev,
                                Md80Reg_E::calOutputEncoderMinE,
                                regR.RO.calOutputEncoderMinE,
                                Md80Reg_E::calOutputEncoderMaxE,
                                regR.RO.calOutputEncoderMaxE,
                                Md80Reg_E::calMainEncoderStdDev,
                                regR.RO.calMainEncoderStdDev,
                                Md80Reg_E::calMainEncoderMinE,
                                regR.RO.calMainEncoderMinE,
                                Md80Reg_E::calMainEncoderMaxE,
                                regR.RO.calMainEncoderMaxE))
        {
            log.error("Extended diagnostic failed (ID: %d)", canId);
            return false;
        }

        if (!md80Register->read(canId,
                                Md80Reg_E::mainEncoderErrors,
                                regR.RO.mainEncoderErrors,
                                Md80Reg_E::outputEncoderErrors,
                                regR.RO.outputEncoderErrors,
                                Md80Reg_E::calibrationErrors,
                                regR.RO.calibrationErrors,
                                Md80Reg_E::bridgeErrors,
                                regR.RO.bridgeErrors,
                                Md80Reg_E::hardwareErrors,
                                regR.RO.hardwareErrors,
                                Md80Reg_E::communicationErrors,
                                regR.RO.communicationErrors,
                                Md80Reg_E::motionErrors,
                                regR.RO.motionErrors))
        {
            log.error("Extended diagnostic failed (ID: %d)", canId);
            return false;
        }
        if (!md80Register->read(canId,
                                Md80Reg_E::mainEncoderErrors,
                                regR.RO.mainEncoderErrors,
                                Md80Reg_E::outputEncoderErrors,
                                regR.RO.outputEncoderErrors,
                                Md80Reg_E::calibrationErrors,
                                regR.RO.calibrationErrors,
                                Md80Reg_E::bridgeErrors,
                                regR.RO.bridgeErrors,
                                Md80Reg_E::hardwareErrors,
                                regR.RO.hardwareErrors,
                                Md80Reg_E::communicationErrors,
                                regR.RO.communicationErrors,
                                Md80Reg_E::motionErrors,
                                regR.RO.motionErrors,
                                Md80Reg_E::miscStatus,
                                regR.RO.miscStatus))
        {
            log.error("Extended diagnostic failed (ID: %d)", canId);
            return false;
        }

        if (!md80Register->read(canId,
                                Md80Reg_E::outputEncoderCalibrationMode,
                                regR.RW.outputEncoderCalibrationMode))
        {
            log.error("Extended diagnostic failed (ID: %d)", canId);
            return false;
        }
        if (!md80Register->read(
                canId, Md80Reg_E::motorCalibrationMode, regR.RW.motorCalibrationMode))
        {
            log.error("Extended diagnostic failed (ID: %d)", canId);
            return false;
        }
        if (!md80Register->read(canId, Md80Reg_E::shuntResistance, regR.RO.shuntResistance))
        {
            log.error("Extended diagnostic failed (ID: %d)", canId);
            return false;
        }

        if (!md80Register->read(canId,
                                Md80Reg_E::homingMode,
                                regR.RW.homingMode,
                                Md80Reg_E::homingMaxTravel,
                                regR.RW.homingMaxTravel,
                                Md80Reg_E::homingTorque,
                                regR.RW.homingTorque,
                                Md80Reg_E::homingVelocity,
                                regR.RW.homingVelocity,
                                Md80Reg_E::homingErrors,
                                regR.RO.homingErrors))
        {
            log.error("Extended diagnostic failed (ID: %d)", canId);
            return false;
        }

        if (!md80Register->read(canId,
                                Md80Reg_E::positionLimitMin,
                                regR.RW.positionLimitMin,
                                Md80Reg_E::positionLimitMax,
                                regR.RW.positionLimitMax))
        {
            log.error("Extended diagnostic failed (ID: %d)", canId);
            return false;
        }

        if (!md80Register->read(canId,
                                Md80Reg_E::maxAcceleration,
                                regR.RW.maxAcceleration,
                                Md80Reg_E::maxDeceleration,
                                regR.RW.maxDeceleration,
                                Md80Reg_E::maxTorque,
                                regR.RW.maxTorque,
                                Md80Reg_E::maxVelocity,
                                regR.RW.maxVelocity))
        {
            log.error("Extended diagnostic failed (ID: %d)", canId);
            return false;
        }

        if (!md80Register->read(canId,
                                Md80Reg_E::profileAcceleration,
                                regR.RW.profileAcceleration,
                                Md80Reg_E::profileDeceleration,
                                regR.RW.profileDeceleration,
                                Md80Reg_E::quickStopDeceleration,
                                regR.RW.quickStopDeceleration,
                                Md80Reg_E::profileVelocity,
                                regR.RW.profileVelocity))
        {
            log.error("Extended diagnostic failed (ID: %d)", canId);
            return false;
        }

        return true;
    }
    mab::CANdleBaudrate_E Candle::getCurrentBaudrate()
    {
        return m_canBaudrate;
    }
    bool Candle::checkMd80ForBaudrate(uint16_t canId)
    {
        uint16_t status;
        return md80Register->read(canId, Md80Reg_E::quickStatus, status);
    }

    std::string getVersionString(const version_ut& ver)
    {
        if (ver.s.tag == 'r' || ver.s.tag == 'R')
            return std::string(std::to_string(ver.s.major) + '.' + std::to_string(ver.s.minor) +
                               '.' + std::to_string(ver.s.revision));
        else
            return std::string(std::to_string(ver.s.major) + '.' + std::to_string(ver.s.minor) +
                               '.' + std::to_string(ver.s.revision) + '.' + ver.s.tag);
    }

    bool Candle::executeCommand(uint16_t    canId,
                                Md80Reg_E   reg,
                                const char* failMsg,
                                const char* successMsg,
                                bool        waitToFinish)
    {
        if (inUpdateMode())
        {
            log.error("Cannot execute commands while CANdle is in UPDATE mode!");
            return false;
        }
        bool commandInProgress;
        md80Register->read(canId, reg, commandInProgress);
        if (commandInProgress)
            log.warn("Command in progress!");
        else if (!md80Register->write(canId, reg, true))
        {
            log.error("%s %d", failMsg, canId);
            return false;
        }
        log.success("%s %d", successMsg, canId);
        if (!waitToFinish)
            return true;
        log.info("Waiting for command to finish");
        do
        {
            usleep(500000);
            md80Register->read(canId, reg, commandInProgress);
            fprintf(stderr, ".");
        } while (commandInProgress);
        printf("\n");
        return true;
    }

    bool Candle::sendBusFrame(
        BusFrameId_t id, uint32_t timeout, char* payload, uint32_t cmdLen, uint32_t respLen)
    {
        char tx[128]{};
        tx[0] = id;
        tx[1] = 0x00;

        if (payload)
            memcpy(&tx[1], payload, cmdLen - 1);

        char* rx = bus->getRxBuffer(0);

        if (bus->transmit(tx, cmdLen, true, timeout, respLen))
            return ((rx[0] == id && rx[1] == true) || (rx[0] == BUS_FRAME_PING_START));
        return false;
    }

    bool Candle::sendBootloaderBusFrame(BootloaderBusFrameId_E id,
                                        uint32_t               timeout,
                                        char*                  payload,
                                        uint32_t               payloadLength,
                                        uint32_t               respLen)
    {
        char tx[2048]{};
        tx[0] = id;
        tx[1] = (char)0xAA;  // Preambles ???
        tx[2] = (char)0xAA;  // Preambles ???

        if (payload)
            memcpy(&tx[3], payload, payloadLength);

        char* rx = bus->getRxBuffer(0);

        if (bus->transmit(tx, payloadLength + 3, true, timeout, respLen))
        {
            if (strcmp("OK", &rx[1]) != 0)
            {
                log.error("Bootloader bad response: %s", &rx[1]);
                return false;
            }
            return true;
        }

        return false;
    }

    bool Candle::reconnectToCandleBootloader()
    {
        bool result = false;
        log.info("Reconnecting to CANdle bootloader...");
        result = static_cast<UsbDevice*>(bus.get())->reconnect(candleVid, bootloaderPid);
        return result;
    }

    bool Candle::reconnectToCandleApp()
    {
        log.info("Reconnecting to CANdle application...");
        usleep(5000000);

        return static_cast<UsbDevice*>(bus.get())->reconnect(candleVid, candlePid);
    }

}  // namespace mab
