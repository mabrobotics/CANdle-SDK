#include "candle.hpp"
#include "MDCO.hpp"
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

    std::cout << od.size() << '\n';
    for (const auto& entry : od)
    {
        if (entry.second.getEntryMetaData().objectType == mab::EDSEntry::ObjectType_E::VALUE)
            std::cout << entry.first << " - " << entry.second.getAsString() << '\n';
        else
        {
            for (u8 i = 0; i < entry.second.getContainerMetaData().value().numberOfSubindices; i++)
            {
                std::cout << entry.first << "[" << (int)i << "]" << " - "
                          << entry.second[i].getAsString() << '\n';
            }
        }
    }

    return EXIT_SUCCESS;
}
