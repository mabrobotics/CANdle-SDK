#include "usbLoader.hpp"
#include "mab_crc.hpp"

UsbLoader::UsbLoader(mab::Candle& candle, MabFileParser& mabFile)
    : I_Loader(mabFile), m_candle(candle)
{
    m_log.m_tag   = "UsbLoader";
    m_log.m_layer = Logger::ProgramLayer_E::LAYER_2;

    // TODO: This stuff might be moved to the iLoader interface
    m_fileSize             = m_mabFile.m_firmwareEntry1.size;
    float flashPagesNeeded = ceilf((float)m_fileSize / (float)M_PAGE_SIZE);
    m_pagesToUpload        = (int)flashPagesNeeded;
    m_currentPage          = 0;
}

UsbLoader::Error_E UsbLoader::resetDevice()
{
    sendResetCmd();
    return Error_E::OK;
}

UsbLoader::Error_E UsbLoader::enterBootloader()
{
    if (!sendInitCmd())
        return Error_E::ERROR_UNKNOWN;

    return Error_E::OK;
}

UsbLoader::Error_E UsbLoader::uploadFirmware()
{
    // m_log.level = logger::LogLevel_E::DEBUG;
    /* write data page per page */
    while (m_currentPage < m_pagesToUpload)
    {
        m_log.debug("Uploading page [ %u ]", m_currentPage);
        if (!sendPage())
        {
            std::cout << std::endl;
            return Error_E::ERROR_UNKNOWN;
        }
        m_log.progress((double)m_currentPage / m_pagesToUpload);
    }

    m_log.m_layer = Logger::ProgramLayer_E::LAYER_2;
    m_log.success("Firmware upload complete!");

    return Error_E::OK;
}

UsbLoader::Error_E UsbLoader::sendBootCommand()
{
    m_log.debug("Sending boot command...");
    if (!m_candle.sendBootloaderBusFrame(mab::BootloaderBusFrameId_E::BOOTLOADER_FRAME_BOOT_TO_APP,
                                         100))
    {
        m_log.error("Error while sending boot command!");
        return Error_E::ERROR_UNKNOWN;
    }

    // if (!m_candle.reconnectToCandleApp())
    // return Error_E::ERROR_UNKNOWN;

    return Error_E::OK;
}

void UsbLoader::sendResetCmd()
{
    m_log.info("Sending RESET command...");

    if (!m_candle.sendBusFrame(mab::BusFrameId_t::USB_FRAME_ENTER_BOOTLOADER, 100))
    {
        m_log.error(
            "Error while sendind bootloader enter command! Checking if not already in "
            "bootloader mode...");
    }

    m_log.success("RESET command sent!");

    usleep(900000);
}

bool UsbLoader::sendInitCmd()
{
    if (!m_candle.reconnectToCandleBootloader())
    {
        m_log.error("Error while reconnecting to bootloader!");
        return false;
    }

    m_log.success("Connected to bootloader!");

    if (!m_candle.sendBootloaderBusFrame(
            mab::BootloaderBusFrameId_E::BOOTLOADER_FRAME_CHECK_ENTERED, 100))
    {
        m_log.error("Error while sending init command!");
        return false;
    }

    return true;
}

bool UsbLoader::sendPage()
{
    m_log.debug("Sending page [ %u ]", m_currentPage);

    bool     result                  = false;
    uint32_t bytesSent               = 0;
    uint32_t framesPerPage           = M_PAGE_SIZE / M_USB_CHUNK_SIZE;
    uint8_t  pageBuffer[M_PAGE_SIZE] = {0};
    int      pageBufferReadSize      = M_PAGE_SIZE;

    size_t binaryLength = m_mabFile.m_firmwareEntry1.size;

    if (binaryLength - ((m_currentPage)*M_PAGE_SIZE) < M_PAGE_SIZE)
        pageBufferReadSize = binaryLength - ((m_currentPage)*M_PAGE_SIZE);

    memcpy(pageBuffer,
           &m_mabFile.m_firmwareEntry1.binary[m_currentPage * M_PAGE_SIZE],
           pageBufferReadSize);

    /* framesPerPage + 1 is for the rest of data that is not a whole chunk in size */
    for (uint32_t i = 0; i < framesPerPage + 1; i++)
    {
        uint32_t frameSize = M_USB_CHUNK_SIZE;
        uint8_t  txBuff[M_USB_CHUNK_SIZE];
        // char    rxBuff[M_USB_CHUNK_SIZE];
        memset(txBuff, 0, M_USB_CHUNK_SIZE);
        // memset(rxBuff, 0, M_USB_CHUNK_SIZE);

        if (pageBufferReadSize - bytesSent < M_USB_CHUNK_SIZE)
            frameSize = (pageBufferReadSize - bytesSent);

        memcpy(txBuff, &pageBuffer[i * M_USB_CHUNK_SIZE], frameSize);
        result =
            m_candle.sendBootloaderBusFrame(mab::BootloaderBusFrameId_E::BOOTLOADER_FRAME_SEND_PAGE,
                                            2000,
                                            (char*)txBuff,
                                            frameSize,
                                            4);

        if (!result)
        {
            m_log.error("Sending Page %u FAIL", m_currentPage);
            return false;
        }

        bytesSent += frameSize;
    }

    result = sendWriteCmd(pageBuffer, bytesSent);
    if (!result)
    {
        m_log.error("Sending write command failed on page [ %u ]!", m_currentPage);
        return false;
    }

    m_currentPage++;

    return result;
}

bool UsbLoader::sendWriteCmd(uint8_t* pPageBuffer, int bufferSize)
{
    uint32_t pageCrc = mab::CalcCRC(pPageBuffer, (uint32_t)bufferSize);

    if (!m_candle.sendBootloaderBusFrame(mab::BootloaderBusFrameId_E::BOOTLOADER_FRAME_WRITE_PAGE,
                                         100,
                                         (char*)&pageCrc,
                                         sizeof(pageCrc),
                                         3))
    {
        m_log.error("Error while sending write command!");
        return false;
    }

    return true;
}