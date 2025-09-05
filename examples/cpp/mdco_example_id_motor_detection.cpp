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
    mab::Candle* candle = mab::attachCandle(mab::CANdleDatarate_E::CAN_DATARATE_1M,
                                            mab::candleTypes::busTypes_t::USB);

    // Look for MAB devices present on the can network
    auto ids = mab::MDCO::discoverOpenMDs(candle);

    if (ids.size() == 0)
    {
        log.error("No driver found with CANopen communication");
        return EXIT_SUCCESS;
    }
    for (int i = 0; i < (int)ids.size(); i++)
    {
        MDCO mdco(ids[i], candle);
        log.info("blink motor drive with ID: %d", ids[i]);
        mdco.blinkOpenTest();
        usleep(5000'000);
    }

    return EXIT_SUCCESS;
}