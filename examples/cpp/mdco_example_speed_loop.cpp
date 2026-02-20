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

    auto od = EDSParser::load(
                  "/home/pawel/mab-github/CANdle-SDK/candletool/template_package/etc/candletool/"
                  "config/eds/"
                  "MDv1.0.0.eds")
                  .first;

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

    mdco.readSDO((*od)[0x6064]);
    log.info("%s: %d",
             (*od)[0x6064].getEntryMetaData().parameterName.c_str(),
             (open_types::INTEGER32_t)(*od)[0x6064]);

    // mdco.blink();
    // mdco.save();
    // mdco.enable();
    return EXIT_SUCCESS;
}
