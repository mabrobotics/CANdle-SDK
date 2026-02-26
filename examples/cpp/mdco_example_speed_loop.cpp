#include <unistd.h>
#include "candle.hpp"
#include "MDCO.hpp"
#include "edsEntry.hpp"
#include "edsParser.hpp"

using namespace mab;

int main()
{
    // Logger is a standalone builtin class for handling output from different modules
    Logger log(Logger::ProgramLayer_E::TOP, "User Program");

    // This parameters sets global internal logging level
    Logger::g_m_verbosity = Logger::Verbosity_E::VERBOSITY_1;

    auto od = EDSParser::load("/etc/candletool/config/eds/MDv1.0.0.eds").first;

    std::cout << od->size() << '\n';
    // for (const auto& entry : *od)
    // {
    //     if (entry.second.getEntryMetaData().objectType == mab::EDSEntry::ObjectType_E::VALUE)
    //         std::cout << entry.first << " - " << entry.second.getAsString() << '\n';
    //     else
    //     {
    //         for (u8 i = 0; i < entry.second.getContainerMetaData().value().numberOfSubindices;
    //         i++)
    //         {
    //             std::cout << entry.first << "[" << (int)i << "]" << " - "
    //                       << entry.second[i].getAsString() << '\n';
    //         }
    //     }
    // }

    mab::Candle* candle = mab::attachCandle(
        mab::CANdleDatarate_E::CAN_DATARATE_1M, mab::candleTypes::busTypes_t::USB, true);

    // Logger::g_m_verbosity = Logger::Verbosity_E::VERBOSITY_2;

    MDCO mdco(10, candle, od);
    if (mdco.init() != MDCO::Error_t::OK)
    {
        log.error("MDCO exited with %d", mdco.init());
    }

    // mdco.readSDO((*od)[0x6064]);
    // log.info("%s: %d",
    //          (*od)[0x6064].getEntryMetaData().parameterName.c_str(),
    //          (open_types::INTEGER32_t)(*od)[0x6064]);

    // mdco.blink();
    // (*od)[0x6076] = (open_types::UNSIGNED16_t)1000;
    // (*od)[0x6075] = (open_types::UNSIGNED16_t)10000;

    // (*od)[0x6072] = (open_types::UNSIGNED16_t)10000;
    // (*od)[0x6073] = (open_types::UNSIGNED16_t)10000;

    // mdco.writeSDO((*od)[0x6076]);
    // mdco.writeSDO((*od)[0x6075]);
    // mdco.writeSDO((*od)[0x6072]);
    // mdco.writeSDO((*od)[0x6073]);
    // mdco.enterConfigMode();
    // (*od)[0x2003][0x3] = (open_types::BOOLEAN_t)1;
    // mdco.writeSDO((*od)[0x2003][0x3]);
    // mdco.save();
    // mdco.zero();
    (*od)[0x6060] = (open_types::INTEGER8_t)9;
    mdco.writeSDO((*od)[0x6060]);
    mdco.enable();
    while (true)
    {
        (*od)[0x60FF] = (open_types::INTEGER32_t)30;
        mdco.writeSDO((*od)[0x60FF]);
        usleep(50'000);
        std::cout << mdco.getVelocity().first << '\n';
    }

    return EXIT_SUCCESS;
}
