#include "canLoader.hpp"
#include "mab_crc.hpp"

CanLoader::CanLoader(mab::Candle& candle, MabFileParser& mabFile, uint32_t canId)
    : I_Loader(mabFile), m_candle(candle), m_canId(canId)
{
    m_log.m_tag   = "CAN LOADER";
    m_log.m_layer = Logger::ProgramLayer_E::LAYER_2;
}

CanLoader::Error_E CanLoader::resetDevice()
{
    sendResetCmd();
    return Error_E::OK;
}

CanLoader::Error_E CanLoader::enterBootloader()
{
    if (!sendSetupCmd())
        return Error_E::ERROR_SETUP;

    return Error_E::OK;
}

CanLoader::Error_E CanLoader::uploadFirmware()
{
    if (!sendEraseCmd())
        return Error_E::ERROR_ERASE;
    if (!sendProgStartCmd())
        return Error_E::ERROR_PROG;

    u32 bytesToSend = m_mabFile.m_fwEntry.size;
    u32 bytesSent   = 0;

    while (bytesSent < bytesToSend)
    {
        m_log.debug("Sending page %d", bytesSent / M_PAGE_SIZE);
        u8* dataChunk = &m_mabFile.m_fwEntry.data[bytesSent];
        if (!sendPage(dataChunk))
            return Error_E::ERROR_PAGE;
        if (!sendWriteCmd(dataChunk))
            return Error_E::ERROR_WRITE;
        bytesSent += M_PAGE_SIZE;
        m_log.progress(std::clamp((f32)bytesSent / bytesToSend, 0.f, 1.f));
    }
    if (!sendSetupCmd()) // to exit from PROG mode
        return Error_E::ERROR_UNKNOWN;
    if (!sendMetaCmd())
        return Error_E::ERROR_META;

    return Error_E::OK;
}

CanLoader::Error_E CanLoader::sendBootCommand()
{
    if (sendBootCmd())
        return Error_E::OK;
    return Error_E::ERROR_BOOT;
}

void CanLoader::sendResetCmd()
{
    uint8_t txBuff[64] = {0};
    char    rxBuff[64] = {0};

    txBuff[0] = (u8)CMD_TARGET_RESET;
    txBuff[1] = 0x00;

    m_log.debug("Entering bootloader mode...");
    if (!m_candle.sendGenericFDCanFrame(m_canId, 2, (const char*)txBuff, rxBuff, nullptr, 1000))
    {
        m_log.warn(
            "Error while sendind bootloader enter command! Checking if not already in "
            "bootloader mode...");
        return;
    }
    usleep(150000);
}

bool CanLoader::sendSetupCmd()
{
    uint8_t txBuff[64] = {0};
    char    rxBuff[64] = {0};

    txBuff[0]         = (u8)CMD_SETUP;
    *(u32*)&txBuff[1] = m_mabFile.m_fwEntry.bootAddress;
    *(u32*)&txBuff[5] = m_mabFile.m_fwEntry.size;

    s32 retries = 10;
    while (retries-- > 0)
    {
        m_candle.sendGenericFDCanFrame(
            m_canId, 9, (const char*)txBuff, (char*)rxBuff, nullptr, 100);
        if (strcmp("OK", (char*)rxBuff) == 0)
            return true;
    }
    return false;
}
bool CanLoader::sendEraseCmd()
{
    uint8_t txBuff[64]            = {0};
    char    rxBuff[64]            = {0};
    u32     maxEraseSize          = 8 * 2048;
    u32     remainingBytesToErase = m_mabFile.m_fwEntry.size;

    txBuff[0]         = (u8)CMD_ERASE;
    *(u32*)&txBuff[1] = m_mabFile.m_fwEntry.bootAddress;

    while (remainingBytesToErase > 0)
    {
        u32 bytesToErase = maxEraseSize;
        if (remainingBytesToErase < maxEraseSize)
            bytesToErase = remainingBytesToErase;
        *(u32*)&txBuff[5] = bytesToErase;
        m_log.debug("ERASE @ %x, %d bytes.", *(u32*)&txBuff[1], *(u32*)&txBuff[5]);
        m_candle.sendGenericFDCanFrame(
            m_canId, 9, (const char*)txBuff, (char*)rxBuff, nullptr, 250);
        if (strncmp(rxBuff, "OK", 2) != 0)
            return false;
        memset(rxBuff, 0, 2);
        *(u32*)&txBuff[1] += bytesToErase;
        remainingBytesToErase -= bytesToErase;
    }
    return true;
}

bool CanLoader::sendProgStartCmd()
{
    m_log.debug("Sending enter page programming mode command... ");

    uint8_t txBuff[64] = {0};
    char    rxBuff[64] = {0};
    bool    useCipher  = strlen((const char*)m_mabFile.m_fwEntry.aes_iv) > 0;

    txBuff[0] = (u8)CMD_PROG;
    txBuff[1] = useCipher;
    if (useCipher)
        memcpy(&txBuff[2], m_mabFile.m_fwEntry.aes_iv, 16);

    m_candle.sendGenericFDCanFrame(m_canId, 18, (const char*)txBuff, rxBuff, nullptr, 100);
    if (strcmp("OK", (char*)rxBuff) == 0)
        return true;
    m_log.error("Sending prog start command FAILED!");
    return false;
}

bool CanLoader::sendPage(u8* data)
{
    char txBuff[64] = {0};
    char rxBuff[64] = {0};

    for (int i = 0; i < 32; i++)
    {
        memcpy(txBuff, &data[i * M_CAN_CHUNK_SIZE], M_CAN_CHUNK_SIZE);
        if (m_candle.sendGenericFDCanFrame(m_canId, M_CAN_CHUNK_SIZE, txBuff, rxBuff, nullptr, 100))
            if (strncmp(rxBuff, "OK", 2) == 0)
                continue;
        return false;
    }
    return true;
}

bool CanLoader::sendWriteCmd(u8* data)
{
    char tx[64] = {}, rx[64] = {};

    tx[0]         = (u8)CMD_WRITE;
    *(u32*)&tx[1] = mab::crc32(data, M_PAGE_SIZE);
    if (m_candle.sendGenericFDCanFrame(m_canId, 5, tx, rx, nullptr, 200))
        return strncmp(rx, "OK", 2) == 0;
    return false;
}

bool CanLoader::sendBootCmd()
{
    char tx[64] = {}, rx[64] = {};

    tx[0]         = (u8)CMD_BOOT;
    *(u32*)&tx[1] = m_mabFile.m_fwEntry.bootAddress;
    if (m_candle.sendGenericFDCanFrame(m_canId, 5, tx, rx, nullptr, 200))
        return strncmp(rx, "OK", 2) == 0;
    return false;
}
bool CanLoader::sendMetaCmd()
{
    char tx[64] = {}, rx[64] = {};
    bool shouldSaveMeta = true;

    tx[0] = (u8)CMD_META;
    tx[1] = shouldSaveMeta;
    memcpy(&tx[2], m_mabFile.m_fwEntry.checksum, 32);
    m_candle.sendGenericFDCanFrame(m_canId, 64, tx, nullptr, nullptr, 10);
    usleep(300000);
    // erase and save takes about 350ms, and this is done
    // due to a bug in candle fw allowing max of 255 ms timeout
    if (m_candle.sendGenericFDCanFrame(m_canId, 0, tx, rx, nullptr, 250))
        return (strncmp(rx, "OK", 2) == 0);
    return false;
}

