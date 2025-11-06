#include "candlelib.hpp"

#include <array>
#include <type_traits>
#include <chrono>
#include <cmath>

using namespace mab;

int main()
{
    Logger log(Logger::ProgramLayer_E::TOP, "User Program");

    // Create and open communication with candle (using unique_ptr releases the user from the need
    // for detach candle at the end)
    auto candle = mab::attachCandle(mab::CAN_DATARATE_1M, mab::candleTypes::busTypes_t::USB);
    std::unique_ptr<mab::Candle> uniqueCandle = std::unique_ptr<mab::Candle>(std::move(candle));

    // Enter your MDs here
    auto mds = std::to_array({MD(101, uniqueCandle.get()), MD(102, uniqueCandle.get())});

    // Check communication with all of the MDs
    for (auto& md : mds)
    {
        if (md.init() != mab::MD::Error_t::OK)
        {
            log.error("Failed to init MD%u", md.m_canId);
            return 1;
        }
    }

    constexpr size_t DRIVE_NO = mds.size();

    MDRegisters_S regArr[DRIVE_NO];

    // When using high performance loops it is recommended to disable std output as it adds an
    // additional latency layer. This can be accomplished with global verbosity override.

    // Logger::g_m_verbosity = Logger::Verbosity_E::SILENT;

    for (size_t i = 0; i < 1000; i++)
    {
        std::future<mab::MD::Error_t> promises[DRIVE_NO];
        // Send read/write requests to all the drives
        for (size_t driveNo = 0; driveNo < DRIVE_NO; driveNo++)
        {
            promises[driveNo] = mds[driveNo].writeRegistersAsync(regArr[driveNo].runBlink = 1);
            promises[driveNo] = mds[driveNo].readRegistersAsync(regArr[driveNo].mainEncoderPosition,
                                                                regArr[driveNo].mosfetTemperature);
        }
        // Insert requested data into the provided register buffers
        for (size_t driveNo = 0; driveNo < DRIVE_NO; driveNo++)
        {
            auto err = promises[driveNo].get();
            log.info("Pos %u: %.6f rad; Temp: %f deg C; Err: %u",
                     driveNo,
                     regArr[driveNo].mainEncoderPosition.value,
                     regArr[driveNo].mosfetTemperature.value,
                     err);
        }
    }

    // Logger::g_m_verbosity = Logger::Verbosity_E::DEFAULT;

    return 0;
}
