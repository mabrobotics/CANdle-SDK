#include "candle_bootloader.hpp"
#include <algorithm>

namespace mab
{
    CandleBootloader::~CandleBootloader()
    {
        enterAppFromBootloader();
        m_usb->disconnect();
        m_log.debug("Destroyed");
    }
    candleTypes::Error_t CandleBootloader::sendCmd(const BootloaderCommand_E cmd,
                                                   const std::vector<u8>     payload = {}) const
    {
        m_log.debug("Sending command: %d", cmd);
        m_log.debug("With payload of size: %d", payload.size());
        constexpr u8  preamble  = 0xAA;
        constexpr u32 timeoutMs = 50;

        // Standard response for candle bootloader protocol
        const std::vector<u8> expectedResponse = {cmd, static_cast<u8>('O'), static_cast<u8>('K')};

        // Standard header for candle protocol
        std::vector<u8> outBuffer = {cmd, preamble, preamble};

        if (payload.size() != 0)
            outBuffer.insert(outBuffer.end(), payload.begin(), payload.end());

        std::pair<std::vector<u8>, I_CommunicationInterface::Error_t> response =
            m_usb->transfer(outBuffer, timeoutMs, expectedResponse.size());

        if (response.second != I_CommunicationInterface::Error_t::OK)
        {
            switch (response.second)
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

        if (response.first.size() < expectedResponse.size() ||
            !std::equal(expectedResponse.begin(), expectedResponse.end(), response.first.begin()))
        {
            m_log.error("Response corrupted!");
            return candleTypes::Error_t::BAD_RESPONSE;
        }

        return candleTypes::Error_t::OK;
    }

    candleTypes::Error_t CandleBootloader::init()
    {
        auto err = m_usb->connect();
        if (err == I_CommunicationInterface::Error_t::OK &&
            sendCmd(BootloaderCommand_E::BOOTLOADER_FRAME_CHECK_ENTERED) ==
                candleTypes::Error_t::OK)
            return candleTypes::Error_t::OK;
        else
            m_log.error("Failed init!");
        return candleTypes::Error_t::DEVICE_NOT_CONNECTED;
    }

    candleTypes::Error_t CandleBootloader::enterAppFromBootloader()
    {
        return sendCmd(BootloaderCommand_E::BOOTLOADER_FRAME_BOOT_TO_APP);
    }

    candleTypes::Error_t CandleBootloader::writePage(const std::array<u8, PAGE_SIZE_STM32G474> page,
                                                     const u32 crc32) const
    {
        m_log.debug("Writing page...");
        constexpr size_t MAX_TRANSFER_SIZE = CANDLE_BOOTLOADER_BUFFER_SIZE - PROTOCOL_HEADER_SIZE;

        constexpr u32 NUMBER_OF_TRANSFERS =
            1 + ((PAGE_SIZE_STM32G474 - 1) / MAX_TRANSFER_SIZE);  // overflow-proof ceiling division

        std::vector<u8> pageV;
        pageV.insert(pageV.end(), page.begin(), page.end());

        for (u32 i = 0; i < NUMBER_OF_TRANSFERS; i++)
        {
            m_log.debug("Transfer %d", i + 1);
            // Transfer either max size that can be transferred because of buffer size inside
            // candle or data left to send size
            size_t transferSize =
                pageV.size() > MAX_TRANSFER_SIZE ? MAX_TRANSFER_SIZE - 1 : pageV.size();

            candleTypes::Error_t err =
                sendCmd(BootloaderCommand_E::BOOTLOADER_FRAME_SEND_PAGE,
                        std::vector<u8>(pageV.begin(), pageV.begin() + transferSize));

            if (err != candleTypes::Error_t::OK)
            {
                m_log.error("Failed to send page to the device");
                return err;
            };
            pageV.erase(pageV.begin(), pageV.begin() + transferSize);
        }

        std::vector<u8> serializedCrc32 = {static_cast<u8>(crc32 & 0xFF),
                                           static_cast<u8>((crc32 >> 8) & 0xFF),
                                           static_cast<u8>((crc32 >> 16) & 0xFF),
                                           static_cast<u8>((crc32 >> 24) & 0xFF)};

        m_log.debug("Sending CRC confirmation");
        return sendCmd(BootloaderCommand_E::BOOTLOADER_FRAME_WRITE_PAGE, serializedCrc32);
    }
}  // namespace mab