#include "canLoader.hpp"
#include "mab_crc.hpp"
#include <unistd.h>

CanLoader::CanLoader(mab::Candle& candle, MabFileParser& mabFile, uint32_t canId)
    : I_Loader(mabFile), m_candle(candle), m_canId(canId)
{
    m_log.m_tag   = "CanLoader";
    m_log.m_layer = Logger::ProgramLayer_E::LAYER_2;

    m_fileSize             = m_mabFile.m_firmwareEntry1.size;
    float flashPagesNeeded = ceilf((float)m_fileSize / (float)M_PAGE_SIZE);
    m_pagesToUpload        = (int)flashPagesNeeded;

    m_currentPage = 0;
}

CanLoader::Error_E CanLoader::resetDevice()
{
    sendResetCmd();
    return Error_E::OK;
}

CanLoader::Error_E CanLoader::enterBootloader()
{
    if (!sendInitCmd())
        return Error_E::ERROR_UNKNOWN;

    if (!sendPageProgCmd())
        return Error_E::ERROR_UNKNOWN;

    return Error_E::OK;
}

CanLoader::Error_E CanLoader::uploadFirmware()
{
    /* write data page per page */
    while (m_currentPage < m_pagesToUpload)
    {
        if (!sendPage())
        {
            std::cout << std::endl;
            return Error_E::ERROR_UNKNOWN;
        }
        m_log.progress((double)m_currentPage / m_pagesToUpload);
    }
    m_log.success("Firmware upload complete!");

    return Error_E::OK;
}

CanLoader::Error_E CanLoader::sendBootCommand()
{
    sendBootCmd();
    return Error_E::OK;
}

void CanLoader::sendResetCmd()
{
    uint8_t txBuff[64] = {0};
    char    rxBuff[64] = {0};

    txBuff[0] = CMD_TARGET_RESET;
    txBuff[1] = 0x00;

    m_log.info("Entering bootloader mode...");

    if (!m_candle.sendGenericFDCanFrame(m_canId, 2, (const char*)txBuff, rxBuff, nullptr, 1000))
    {
        m_log.error(
            "Error while sendind bootloader enter command! Checking if not already in "
            "bootloader mode...");
    }

    usleep(500000);
}

bool CanLoader::sendInitCmd()
{
    uint8_t txBuff[64] = {0};
    char    rxBuff[64] = {0};

    txBuff[0]              = CMD_HOST_INIT;
    *(uint32_t*)&txBuff[1] = M_BOOT_ADDRESS;

    m_log.info("Detecting bootloader mode...");

    m_candle.sendGenericFDCanFrame(m_canId, 5, (const char*)txBuff, (char*)rxBuff, nullptr, 50);

    if (strcmp("OK", (char*)rxBuff) == 0)
        m_log.success("Bootloader detected!");
    else
    {
        m_log.error("Bootloader mode could not be entered!");
        return false;
    }
    return true;
}

bool CanLoader::sendPageProgCmd()
{
    m_log.debug("Sending enter page programming mode command... ");

    uint8_t txBuff[64] = {0};
    char    rxBuff[64] = {0};

    txBuff[0] = CMD_PAGE_PROG;
    txBuff[1] = 0x00;

    m_candle.sendGenericFDCanFrame(m_canId, 5, (const char*)txBuff, rxBuff, nullptr, 500);
    if (strcmp("OK", (char*)rxBuff) == 0)
        return true;

    m_log.error("Sending page program command FAILED!");
    return false;
}

bool CanLoader::sendPage()
{
    m_log.debug("Sending page [ %u ]", m_currentPage);

    int     framesPerPage           = M_PAGE_SIZE / M_CAN_CHUNK_SIZE;
    uint8_t pageBuffer[M_PAGE_SIZE] = {0};
    int     pageBufferReadSize      = M_PAGE_SIZE;

    size_t binaryLength = m_mabFile.m_firmwareEntry1.size;

    if (binaryLength - ((m_currentPage)*M_PAGE_SIZE) < M_PAGE_SIZE)
        pageBufferReadSize = binaryLength - ((m_currentPage)*M_PAGE_SIZE);

    memcpy(pageBuffer,
           &m_mabFile.m_firmwareEntry1.binary[m_currentPage * M_PAGE_SIZE],
           pageBufferReadSize);

    for (int i = 0; i < framesPerPage; i++)
    {
        uint8_t txBuff[M_CAN_CHUNK_SIZE];
        char    rxBuff[M_CAN_CHUNK_SIZE];
        memset(txBuff, 0, M_CAN_CHUNK_SIZE);
        memset(rxBuff, 0, M_CAN_CHUNK_SIZE);
        memcpy(txBuff, &pageBuffer[i * M_CAN_CHUNK_SIZE], M_CAN_CHUNK_SIZE);

        m_candle.sendGenericFDCanFrame(
            m_canId, M_CAN_CHUNK_SIZE, (const char*)txBuff, rxBuff, nullptr, 500);
        if (strcmp("OK", rxBuff) != 0)
        {
            m_log.error("Sending Page %u FAIL", m_currentPage);
            return false;
        }
    }
    m_currentPage++;
    bool result = sendWriteCmd(pageBuffer, M_PAGE_SIZE);
    return result;
}

bool CanLoader::sendWriteCmd(uint8_t* pPageBuffer, int bufferSize)
{
    m_log.debug("Sending write command");

    uint8_t txBuff[64] = {0};
    char    rxBuff[64] = {0};

    txBuff[0]              = CMD_WRITE;
    *(uint32_t*)&txBuff[1] = mab::CalcCRC(pPageBuffer, (uint32_t)bufferSize);

    m_candle.sendGenericFDCanFrame(m_canId, 5, (const char*)txBuff, rxBuff, nullptr, 500);
    usleep(50);
    if (strcmp("OK", (char*)rxBuff) == 0)
        return true;

    m_log.error("CRC fail!");
    return false;
}

bool CanLoader::sendBootCmd()
{
    m_log.debug("Sending boot command...");

    uint8_t txBuff[64];
    char    rxBuff[64];

    txBuff[0] = CMD_BOOT;
    txBuff[1] = 0x00;

    m_candle.sendGenericFDCanFrame(m_canId, 5, (const char*)txBuff, rxBuff, nullptr, 500);
    if (strcmp("OK", rxBuff) == 0)
        return true;

    m_log.error("Error while sending boot command!");
    return false;
}