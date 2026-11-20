// Two CANdles (USB), each with its own set of MD drives, driven in parallel from one thread per
// CANdle. Read-only: drives are not enabled, nothing moves.
//
// Threading rules:
//  - a Candle and the MD objects attached to it must be used from one thread only,
//  - configure logging (Logger::g_m_verbosity, setStream) before threads start,
//  - MD::discoverMDs changes the global log verbosity, so do not call it from several threads;
//    this example probes ids itself and silences logging for the whole discovery instead.

#include "candle.hpp"
#include "MD.hpp"

#include <chrono>
#include <cstring>
#include <thread>
#include <vector>

static constexpr int          MAX_CANDLES = 2;
static constexpr int          CYCLES      = 1000;
static constexpr mab::canId_t MIN_ID      = 10;
static constexpr mab::canId_t MAX_ID      = 1000;

struct Branch
{
    mab::Candle*         candle = nullptr;
    std::vector<mab::MD> mds;
    std::string          serial;
    u32                  errors    = 0;
    double               avgCycleUs = 0.0;
};

static void discover(Branch* b)
{
    for (mab::canId_t id = MIN_ID; id <= MAX_ID; id++)
    {
        mab::MD md(id, b->candle);
        if (md.init() == mab::MD::Error_t::OK)
            b->mds.push_back(md);
    }
}

static void feedbackLoop(Branch* b)
{
    mab::MDRegisters_S regs;
    auto               start = std::chrono::steady_clock::now();
    for (int i = 0; i < CYCLES; i++)
    {
        for (auto& md : b->mds)
        {
            if (md.readRegisters(regs.mainEncoderPosition, regs.mainEncoderVelocity) !=
                mab::MD::Error_t::OK)
                b->errors++;
        }
    }
    auto elapsed = std::chrono::steady_clock::now() - start;
    b->avgCycleUs =
        std::chrono::duration<double, std::micro>(elapsed).count() / static_cast<double>(CYCLES);
}

// Usage: md_two_candles [--datarate 1M|2M|5M|8M]   (default 1M)
static bool parseDatarate(int argc, char** argv, mab::CANdleDatarate_E* datarate)
{
    static const char*                 names[] = {"1M", "2M", "5M", "8M"};
    static const mab::CANdleDatarate_E rates[] = {mab::CANdleDatarate_E::CAN_DATARATE_1M,
                                                  mab::CANdleDatarate_E::CAN_DATARATE_2M,
                                                  mab::CANdleDatarate_E::CAN_DATARATE_5M,
                                                  mab::CANdleDatarate_E::CAN_DATARATE_8M};
    *datarate = mab::CANdleDatarate_E::CAN_DATARATE_1M;
    if (argc == 1)
        return true;
    if (argc != 3 || strcmp(argv[1], "--datarate") != 0)
        return false;
    for (int i = 0; i < 4; i++)
    {
        if (strcmp(argv[2], names[i]) == 0)
        {
            *datarate = rates[i];
            return true;
        }
    }
    return false;
}

int main(int argc, char** argv)
{
    Logger log(Logger::ProgramLayer_E::TOP, "TWO_CANDLES");

    mab::CANdleDatarate_E datarate;
    if (!parseDatarate(argc, argv, &datarate))
    {
        log.error("Usage: %s [--datarate 1M|2M|5M|8M]", argv[0]);
        return EXIT_FAILURE;
    }

    std::vector<std::string> serials =
        mab::USB::listSerials(mab::Candle::CANDLE_VID, mab::Candle::CANDLE_PID);
    if (serials.size() < MAX_CANDLES)
    {
        log.error("Need %d CANdles, found %zu", MAX_CANDLES, serials.size());
        return EXIT_FAILURE;
    }

    Branch branches[MAX_CANDLES];
    for (int i = 0; i < MAX_CANDLES; i++)
    {
        Branch& b = branches[i];
        b.serial  = serials[i];
        auto bus  = std::make_unique<mab::USB>(
            mab::Candle::CANDLE_VID, mab::Candle::CANDLE_PID, b.serial);
        b.candle = mab::attachCandle(datarate, std::move(bus));
        if (b.candle == nullptr)
        {
            log.error("Could not attach CANdle %s", b.serial.c_str());
            for (int j = 0; j < i; j++)
                mab::detachCandle(branches[j].candle);
            return EXIT_FAILURE;
        }
    }

    // One thread per CANdle, both buses run at the same time
    std::thread threads[MAX_CANDLES];
    log.info("Scanning ids %u - %u on all CANdles", MIN_ID, MAX_ID);
    // Missing ids produce timeout errors on every layer. Silence logging globally - safe because
    // it is set before the threads start and restored after they finish.
    auto prevVerbosity    = Logger::g_m_verbosity;
    Logger::g_m_verbosity = Logger::Verbosity_E::SILENT;
    for (int i = 0; i < MAX_CANDLES; i++)
        threads[i] = std::thread(discover, &branches[i]);
    for (int i = 0; i < MAX_CANDLES; i++)
        threads[i].join();
    Logger::g_m_verbosity = prevVerbosity;
    for (int i = 0; i < MAX_CANDLES; i++)
    {
        log.info("CANdle %s: %zu MDs", branches[i].serial.c_str(), branches[i].mds.size());
        for (auto& md : branches[i].mds)
            log.info(" - id %u", md.m_canId);
    }

    auto start = std::chrono::steady_clock::now();
    for (int i = 0; i < MAX_CANDLES; i++)
        threads[i] = std::thread(feedbackLoop, &branches[i]);
    for (int i = 0; i < MAX_CANDLES; i++)
        threads[i].join();
    double totalMs =
        std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start)
            .count();

    for (int i = 0; i < MAX_CANDLES; i++)
    {
        Branch& b = branches[i];
        log.info("CANdle %s: %zu MDs | avg cycle %.0f us | errors %u",
                 b.serial.c_str(),
                 b.mds.size(),
                 b.avgCycleUs,
                 b.errors);
    }
    log.info("Total time for %d cycles on both CANdles: %.1f ms", CYCLES, totalMs);

    for (int i = 0; i < MAX_CANDLES; i++)
    {
        branches[i].mds.clear();  // MDs hold a Candle*, drop them before the Candle
        mab::detachCandle(branches[i].candle);
    }
    return EXIT_SUCCESS;
}
