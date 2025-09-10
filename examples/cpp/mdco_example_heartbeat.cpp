#include "candle.hpp"
#include "MDCO.hpp"

using namespace mab;

int main()
{
    // Logger is a standalone builtin class for handling output from different modules
    Logger log(Logger::ProgramLayer_E::TOP, "User Program");

    // This parameters sets global internal logging level
    Logger::g_m_verbosity = Logger::Verbosity_E::VERBOSITY_1;

    // Attach Candle is an AIO method to get ready to use candle handle that corresponds to the real
    // CANdle USB-CAN converter. Its a main object so should have the longest lifetime of all
    // objects from the library.
    mab::Candle* candle = mab::attachCandle(
        mab::CANdleDatarate_E::CAN_DATARATE_1M, mab::candleTypes::busTypes_t::USB, true);

    // Look for MAB devices present on the can network
    auto ids = mab::MDCO::discoverOpenMDs(candle);

    // if no motor drive found with CANopen communication
    if (ids.size() == 0)
    {
        log.error("No driver found with CANopen communication");
        return EXIT_SUCCESS;
    }

    // Get first md that was detected (the one with the lowest id).
    MDCO mdcoListener(ids[0], candle);

    // display on terminal heartbeat information send by the motor drive
    mdcoListener.testHeartbeat();

    // Now we will test the heartbeat listener function.
    // The candle will first send heartbeat messages, then stop.
    // When the candle stops sending heartbeats, the motor drive
    // should switch to stop mode (Cf. NMT state machine, CiA 301)
    MDCO mdcoProducer(15, candle);

    std::vector<u8> frame;
    frame.push_back(0x05);
    long DataSlave;
    u8   bytes1 = 0x00;  //(Producer ID >> 8)
    u8   bytes2 = 0x0E;  //(Producer ID)
    u8   bytes3 = 0x03;  // (HeartbeatTimeout [ms] >> 8)
    u8   bytes4 = 0xE8;  // (HeartbeatTimeout [ms])
    DataSlave   = bytes4 + (bytes3 << 8) + (bytes2 << 16) + (bytes1 << 24);

    mdcoProducer.sendCustomData((0x700 + 0x0E), frame);  // send the heartbeat message
    if (mdcoListener.getValueFromOpenRegister(0x1003, 0x00) != 00)
    {
        log.error("The Driver is in stop state before testing the heartbeat");
        return EXIT_SUCCESS;
    }
    // give the order to the motor drive to listen the heartbeat with id 0x0E and enter stop mode
    // if no heartbeat received after 1s
    mdcoListener.writeOpenRegisters(0x1016, 0x01, DataSlave, 4);

    // testing multiple time if the motor is entering stop mode before timeout
    for (int i = 1; i <= 6; i++)
    {
        // send the heartbeat message
        mdcoProducer.sendCustomData((0x700 + 0x0E), frame);
        auto start   = std::chrono::steady_clock::now();
        auto timeout = std::chrono::milliseconds(900);
        while (std::chrono::steady_clock::now() - start < timeout)
        {
        }
        // verify if stop state before Heartbeat timeout
        if (mdcoListener.getValueFromOpenRegister(0x1001, 0) != 0)
        {
            log.error("The driver enter stop mode before the Heartbeat timeout");
            return EXIT_SUCCESS;
        }
        else
        {
            log.success("The motor is not in stop state after %d heartbeat message send", i);
        }
    }

    // testing if the motor enter stop mode after timeout
    auto start   = std::chrono::steady_clock::now();
    auto timeout = std::chrono::milliseconds(2000);
    while (std::chrono::steady_clock::now() - start < timeout)
    {
    }
    // verify stop state after Heartbeat timeout
    if (mdcoListener.getValueFromOpenRegister(0x1001, 0) != 0)
    {
        log.success("The driver enter stop mode after the Heartbeat timeout");
        return EXIT_SUCCESS;
    }
    else
    {
        log.error("The motor never enter stop mode");
        return EXIT_SUCCESS;
    }
}