#include "candle_bootloader.hpp"

namespace mab
{
    CandleBootloader::~CandleBootloader()
    {
        enterAppFromBootloader();
        m_usb->disconnect();
    }
    candleTypes::Error_t CandleBootloader::sendCmd(const BootloaderCommand_E cmd,
                                                   const std::vector<u8>     payload = {}) const
    {
        constexpr u8  preamble  = 0xAA;
        constexpr u32 timeoutMs = 2;

        I_CommunicationInterface::Error_t err =
            I_CommunicationInterface::Error_t::INITIALIZATION_ERROR;

        std::vector<u8> outBuffer = {cmd, preamble, preamble};
        if (payload.size() == 0)
            err = m_usb->transfer(outBuffer, timeoutMs);
        else
        {
            outBuffer.insert(outBuffer.end(), payload.begin(), payload.end());
            err = m_usb->transfer(outBuffer, timeoutMs);
        }
        if (err != I_CommunicationInterface::Error_t::OK)
        {
            switch (err)
            {
                case I_CommunicationInterface::Error_t::TRANSMITTER_ERROR:
                    m_log.error("Error while transfering data to USB bootloader");
                    return candleTypes::Error_t::UNKNOWN_ERROR;
                case I_CommunicationInterface::Error_t::RECEIVER_ERROR:
                    m_log.error("Error while receiving data to USB bootloader");
                    return candleTypes::Error_t::UNKNOWN_ERROR;
                default:
                    m_log.error("USB bootloader device failed");
                    return candleTypes::Error_t::UNKNOWN_ERROR;
            }
        }
        return candleTypes::Error_t::OK;
    }

    candleTypes::Error_t CandleBootloader::init()
    {
        return candleTypes::Error_t::OK;
    }

    candleTypes::Error_t CandleBootloader::enterAppFromBootloader()
    {
        return sendCmd(BootloaderCommand_E::BOOTLOADER_FRAME_BOOT_TO_APP);
    }
}  // namespace mab