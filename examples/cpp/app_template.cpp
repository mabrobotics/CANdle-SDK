#include "candle.hpp"
#include "pds.hpp"
#include "MD.hpp"
#include "logger.hpp"

#include <vector>
#include <thread>

int main(int argc, char** argv)
{
    Logger log(Logger::ProgramLayer_E::TOP, "User Program");
    // Logger::g_m_verbosity = Logger::Verbosity_E::VERBOSITY_1;

    mab::MDRegisters_S    regs;
    std::vector<mab::Pds> pdsV;
    std::vector<mab::MD>  mdV;

    auto candle = std::unique_ptr<mab::Candle>(
        attachCandle(mab::CANdleDatarate_E::CAN_DATARATE_1M, mab::candleTypes::busTypes_t::USB));

    for (const auto& canIdAndRate : mab::Pds::discoverPDS(mab::candleTypes::busTypes_t::USB))
    {
        log.info("Found PDS with CAN ID: %u", canIdAndRate.id);
        pdsV.emplace_back(canIdAndRate.id, candle.get());
        pdsV.back().init(canIdAndRate.id);
    }

    for (const auto& id : mab::MD::discoverMDs(candle.get()))
    {
        log.info("Found MD with CAN ID: %u", id);
        mdV.emplace_back(id, candle.get());
        mdV.back().init();
    }

    bool run = true;
    while (run)
    {
        for (auto& md : mdV)
        {
            if (md.readRegisters(regs.mainEncoderPosition, regs.mainEncoderVelocity) !=
                mab::MD::Error_t::OK)
            {
                log.error("Error reading encoder data!");
                run = false;
            }
            else
            {
                log.info("MD %u", md.m_canId);
                log.info("Position: %.4f", regs.mainEncoderPosition.value);
                log.info("Velocity: %.4f\n", regs.mainEncoderVelocity.value);
            }
        }
        log.info("----------------------------");
        for (auto& pds : pdsV)
        {
            u32 busVoltage;
            if (pds.getBusVoltage(busVoltage) != mab::Pds::error_E::OK)
            {
                log.error("Error reading bus voltage!");
                run = false;
            }
            else
            {
                log.info("PDS %u", pds.getCanId());
                log.info("Bus voltage: %u mV\n", busVoltage);
            }
        }
        log.info("----------------------------");
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    return EXIT_SUCCESS;
}