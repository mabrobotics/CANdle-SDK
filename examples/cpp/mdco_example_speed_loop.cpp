#include <unistd.h>
#include <chrono>
#include <cstdlib>
#include <thread>
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

    mab::Candle* candle = mab::attachCandle(
        mab::CANdleDatarate_E::CAN_DATARATE_1M, mab::candleTypes::busTypes_t::USB, true);

    MDCO mdco(10, candle, od);
    if (mdco.init() != MDCO::Error_t::OK)
    {
        log.error("MDCO exited with %d", mdco.init());
    }

    if (mdco.enable() != MDCO::Error_t::OK)
    {
        return EXIT_FAILURE;
    }
    if (mdco.setOperationMode(mab::ModesOfOperation::CyclicSyncVelocity) != MDCO::Error_t::OK)
    {
        return EXIT_FAILURE;
    }
    auto start = std::chrono::steady_clock::now();
    while (start + std::chrono::milliseconds(10'000) > std::chrono::steady_clock::now())
    {
        mdco.setTargetVelocity(2.0f);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        auto pos = mdco.getPosition().first;
        log << "Pos: " << pos << '\n';
        mdco.setTargetVelocity(2.1f);  // get driver unstuck from quickstop
    }

    if (mdco.disable() != MDCO::Error_t::OK)
    {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
