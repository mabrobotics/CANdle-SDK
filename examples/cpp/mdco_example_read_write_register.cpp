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

    if (ids.size() == 0)
    {
        log.error("No driver found with CANopen communication");
        return EXIT_SUCCESS;
    }

    // Get first md that was detected (the one with the lowest id).
    MDCO mdco(ids[0], candle);

    // Display the value contain in the object 0x6064:0 in the terminal
    mdco.readOpenRegisters(0x6064, 0x00);

    // You can write value with SDO, you can use register name or register address
    // You can find all register detail in the object dictionary:
    // https://mabrobotics.github.io/MD80-x-CANdle-Documentation/md_canopen/OD.html
    mdco.writeOpenRegisters("Controlword", 15);
    mdco.writeOpenRegisters(0x6040, 0x00, 15);

    // You can get value from a register
    long returnValue = mdco.getValueFromOpenRegister(0x6064, 0x00);
    if (returnValue > 0)
        log.info("The position of the motor is positive");

    // You can use SDO segmented transfer for register with a size over 4 bytes
    std::vector<u8> MotorNameBefore;
    std::vector<u8> MotorNameAfter;
    mdco.readLongOpenRegisters(0x2000, 0x06, MotorNameBefore);
    mdco.writeLongOpenRegisters(0x2000, 0x06, "My Custom Motor Name");
    mdco.readLongOpenRegisters(0x2000, 0x06, MotorNameAfter);

    // you can also use PDO for faster communication
    std::vector<u8> frame = {0x00, 0x0f};
    mdco.writeOpenPDORegisters(0x200 + ids[0], frame);

    // you can use this method for every other message who don't need answer (NMT,TimeStamp,etc.).
    // e.g. send a NMT reset node message to the motor drive with the lowest id
    std::vector<u8> data = {0x81, (u8)ids[0]};
    mdco.sendCustomData(0x000, data);

    return EXIT_SUCCESS;
}