#include <cstdio>
#include <string>
#include <unordered_map>

#include "candle.hpp"
#include "MD.hpp"
#include "MDStatus.hpp"

/// @brief Print one status register the same way as "candletool md info"
template <typename T>
static void printStatus(const char*                                         name,
                        u32                                                 raw,
                        std::unordered_map<T, mab::MDStatus::StatusItem_S>& map)
{
    // Decode raw register value into named flags
    mab::MDStatus::decode(raw, map);

    std::string flags;
    for (const auto& [bit, item] : map)
    {
        if (!item.isSet())
            continue;
        if (!flags.empty())
            flags += ", ";
        flags += item.name + (item.isError ? " [ERROR]" : " [WARNING]");
    }
    printf("- %-22s 0x%08x (%s%s)\n",
           name,
           raw,
           flags.empty() ? "OK" : "Set flags: ",
           flags.c_str());
}

int main()
{
    Logger log(Logger::ProgramLayer_E::TOP, "User Program");

    mab::Candle* candle = mab::attachCandle(mab::CANdleDatarate_E::CAN_DATARATE_1M,
                                            mab::candleTypes::busTypes_t::USB);

    // Look for MAB devices present on the can network, stop at the first one (only one is used)
    auto ids = mab::MD::discoverMDs(candle, 10, 0x7FF, 1);
    if (ids.empty())
    {
        log.error("No MDs found!");
        mab::detachCandle(candle);
        return EXIT_FAILURE;
    }

    mab::MD md(ids[0], candle);
    if (md.init() != mab::MD::Error_t::OK)
    {
        log.error("Could not initialize MD %u!", ids[0]);
        mab::detachCandle(candle);
        return EXIT_FAILURE;
    }
    md.m_timeout = 2 /*ms*/;  // sometimes reading registers can take more than default 1ms timeout

    // Read all status registers in one request (a single request is limited to 64 bytes)
    mab::MDRegisters_S regs;
    mab::MD::Error_t   err = md.readRegisters(regs.mainEncoderStatus,
                                            regs.auxEncoderStatus,
                                            regs.calibrationStatus,
                                            regs.bridgeStatus,
                                            regs.hardwareStatus,
                                            regs.communicationStatus,
                                            regs.motionStatus,
                                            regs.miscStatus,
                                            regs.configStatus);
    if (err == mab::MD::Error_t::OK)
        err = md.readRegisters(regs.auxEncoder);  // aux status is meaningful only with aux encoder
    if (err != mab::MD::Error_t::OK)
    {
        log.error("Error reading status registers: %u", (u8)err);
        mab::detachCandle(candle);
        return EXIT_FAILURE;
    }

    // Each status register has its own set of flags, MDStatus holds their names and severity
    mab::MDStatus s;
    printf("***** MD %u STATUS *****\n", ids[0]);
    printStatus("main encoder status:", regs.mainEncoderStatus.value, s.encoderStatus);
    if (regs.auxEncoder.value != 0)
        printStatus("aux encoder status:", regs.auxEncoderStatus.value, s.encoderStatus);
    printStatus("calibration status:", regs.calibrationStatus.value, s.calibrationStatus);
    printStatus("bridge status:", regs.bridgeStatus.value, s.bridgeStatus);
    printStatus("hardware status:", regs.hardwareStatus.value, s.hardwareStatus);
    printStatus("communication status:", regs.communicationStatus.value, s.communicationStatus);
    printStatus("motion status:", regs.motionStatus.value, s.motionStatus);
    printStatus("misc status:", regs.miscStatus.value, s.miscStatus);
    printStatus("config status:", regs.configStatus.value, s.configStatus);

    mab::detachCandle(candle);
    return EXIT_SUCCESS;
}
