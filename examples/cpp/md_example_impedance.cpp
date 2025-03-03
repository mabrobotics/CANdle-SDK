
#include "MD.hpp"
#include "candle_v2.hpp"
#include "USB_v2.hpp"
#include "I_communication_interface.hpp"
#include <thread>

// initialize USB interface

int main()
{
    Logger log(Logger::ProgramLayer_E::TOP, "User Program");
    Logger::g_m_verbosity = Logger::Verbosity_E::VERBOSITY_1;

    std::unique_ptr<mab::I_CommunicationInterface> usb =
        std::make_unique<mab::USBv2>(mab::CandleV2::CANDLE_VID, mab::CandleV2::CANDLE_PID);

    if (usb->connect() != mab::I_CommunicationInterface::Error_t::OK)
    {
        log.error("Unable to connect to Candle device!");
        exit(1);
    }

    auto candle = mab::attachCandle(mab::CANdleBaudrate_E::CAN_BAUD_1M, std::move(usb));

    candle->discoverDevices();

    auto mdMap = candle->getMDmapHandle();

    if (mdMap->size() == 0)
    {
        log.error("No MDs found!");
        return EXIT_FAILURE;
    }

    mab::MD& md = mdMap->begin()->second;

    md.m_mdRegisters.runZero           = 0x1;
    md.m_mdRegisters.motionModeCommand = mab::Md80Mode_E::IMPEDANCE;
    mab::MD::Error_t mdResult          = md.readRegisters(md.m_mdRegisters.motorName).second;
    if (mdResult != mab::MD::Error_t::OK)
    {
        log.error("Reading name failed!");
        return EXIT_FAILURE;
    }

    std::string motorName = md.m_mdRegisters.motorName.value;

    log.info("First motor is %s", motorName.c_str());

    mdResult = md.writeRegisters(md.m_mdRegisters.runZero, md.m_mdRegisters.motionModeCommand);

    if (mdResult != mab::MD::Error_t::OK)
    {
        log.error("Configuration failed!");
        return EXIT_FAILURE;
    }

    if (mdResult != mab::MD::Error_t::OK)
    {
        log.error("Shutdown failed!");
        return EXIT_FAILURE;
    }

    md.disable();
    md.m_mdRegisters.motionModeCommand = mab::Md80Mode_E::IMPEDANCE;
    mdResult                           = md.writeRegisters(md.m_mdRegisters.motionModeCommand);
    if (mdResult != mab::MD::Error_t::OK)
    {
        log.error("Enable operation failed!");
        return EXIT_FAILURE;
    }
    md.readRegisters(md.m_mdRegisters.motionModeStatus);
    md.enable();

    log.debug("Mode after write: %d", md.m_mdRegisters.motionModeStatus.value);

    constexpr float stepSize        = 0.001f;
    md.m_mdRegisters.targetPosition = 0.0f;

    for (u16 i = 0; i < 1000; i++)
    {
        md.m_mdRegisters.targetPosition.value += stepSize;
        md.writeRegisters(md.m_mdRegisters.targetPosition);
        md.readRegisters(md.m_mdRegisters.motionModeStatus, md.m_mdRegisters.mainEncoderPosition);
        log.info("Target position: %.3f | Current position: %.3f",
                 md.m_mdRegisters.targetPosition.value,
                 md.m_mdRegisters.mainEncoderPosition.value);
        // usleep(5'000);
    }

    md.m_mdRegisters.state = 6 /*shutdown cmd to reset to default state*/;
    mdResult               = md.writeRegisters(md.m_mdRegisters.state);
}
