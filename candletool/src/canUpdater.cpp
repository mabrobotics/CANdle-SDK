#include "canUpdater.hpp"
#include "mab_crc.hpp"

#include "mini/ini.h"
#include <string>

namespace canUpdater
{
    bool hexStringToBytes(u8 buffer[], u32 bufferLen, const std::string& str)
    {
        if (bufferLen < (str.length() + 1) / 2 || str.length() % 2 == 1)
            return false;
        for (size_t i = 0; i < str.length() / 2; i++)
        {
            std::string byteString = str.substr(2 * i, 2);
            buffer[i]              = std::stoi(byteString.c_str(), nullptr, 16);
        }
        return true;
    }

    bool parseMabFile(const char* pathToMabFile, const char* tag, mabData& mabData)
    {
        mINI::INIFile      file(pathToMabFile);
        mINI::INIStructure ini;
        if (!file.read(ini))
            return false;
        strncpy(mabData.tag, ini.get("firmware").get("tag").c_str(), 16);
        mabData.fwSize = atoi(ini.get("firmware").get("size").c_str());
        hexStringToBytes(
            mabData.checksum, sizeof(mabData.checksum), ini.get("firmware").get("checksum"));
        mabData.fwStartAddress = strtol(ini.get("firmware").get("start").c_str(), nullptr, 16);
        strcpy(mabData.fwVersion, ini.get("firmware").get("version").c_str());
        hexStringToBytes(mabData.iv, sizeof(mabData.iv), ini.get("firmware").get("iv"));
        if (mabData.fwSize == 0 || strcmp(mabData.tag, tag) != 0)
            return false;
        memset(mabData.fwData, 0, sizeof(mabData.fwData));
        hexStringToBytes(mabData.fwData, mabData.fwSize, ini.get("firmware").get("binary"));
        return true;
    }
    bool sendHostInit(mab::Candle& candle, Logger& log, u16 id, u32 fwStartAdr, u32 fwSize)
    {
        s32  retries = 0;
        s32  retriesMax = 50;
        bool success = false;
        do
        {
            char tx[64] = {}, rx[64] = {};
            tx[0]         = (u8)0xb1;
            *(u32*)&tx[1] = fwStartAdr;
            *(u32*)&tx[5] = fwSize;
            success =
                (candle.sendGenericFDCanFrame(id, 9, tx, rx, 100) && strncmp(rx, "OK", 2) == 0); 
            if (!success && retries % 10 == 0)
                log.warn("Could not connect to bootloader. Retry %d/%d", retries, retriesMax);
        } while (!success && retries++ < retriesMax);
        return success;
    }
    bool sendErase(mab::Candle& candle, Logger& log, u16 id, u32 eraseStart, u32 eraseSize)
    {
        char tx[64] = {}, rx[64] = {};
        s32  maxEraseSize          = 8 * 2048;
        s32  remainingBytesToErase = eraseSize;

        tx[0]         = (u8)0xb2;
        *(u32*)&tx[1] = eraseStart;
        while (remainingBytesToErase > 0)
        {
            u32 bytesToErase = maxEraseSize;
            if (remainingBytesToErase < maxEraseSize)
                bytesToErase = remainingBytesToErase;
            *(u32*)&tx[5] = bytesToErase;
            log.debug("ERASE @ %x, %d bytes.", *(u32*)&tx[1], *(u32*)&tx[5]);

            if (!(candle.sendGenericFDCanFrame(id, 9, tx, rx, 250) && strncmp(rx, "OK", 2) == 0))
                return false;
            memset(rx, 0, 2);
            *(u32*)&tx[1] += bytesToErase;
            remainingBytesToErase -= bytesToErase;
        }
        return true;
    }
    bool sendProgStart(mab::Candle& candle, u16 id, bool cipher, u8* iv)
    {
        char tx[64] = {}, rx[64] = {};
        tx[0] = (u8)0xb3;
        tx[1] = (u8)cipher;
        memcpy(&tx[2], iv, 16);
        return (candle.sendGenericFDCanFrame(id, 18, tx, rx, 100) && strncmp(rx, "OK", 2) == 0);
    }
    bool sendWrite(mab::Candle& candle, u16 id, u8* pagePtr, u32 dataSize)
    {
        char tx[64] = {}, rx[64] = {};
        tx[0]         = (u8)0xb4;  // Send Write
        *(u32*)&tx[1] = mab::crc32(pagePtr, dataSize);
        return (candle.sendGenericFDCanFrame(id, 5, tx, rx, 200) && (strncmp(rx, "OK", 2) == 0));
    }
    bool sendSendFirmware(mab::Candle& candle, Logger& log, u16 id, u32 fwSize, u8* fwBuffer)
    {
        char tx[64] = {}, rx[64] = {};
        u32  page         = 0;
        u32  bytesWritten = 0;
        while (bytesWritten < fwSize)
        {
            log.debug("Sending Page %d", page);
            for (int i = 0; i < 32; i++)
            {
                memcpy(tx, &fwBuffer[page * 2048 + i * 64], 64);
                bool success = (candle.sendGenericFDCanFrame(id, 64, tx, rx, 200) &&
                                strncmp(rx, "OK", 2) == 0);
                if (!success)
                {
                    log.error("Page %d at %d failed!", page, i * 64);
                    return false;
                }
                log.debug("WRITE %d/32 OK", i+1);
            }
            if (!sendWrite(candle, id, &fwBuffer[page * 2048], 2048))
                return false;
            log.debug("WRITE OK");
            bytesWritten += 2048;
            page++;
            log.progress(std::clamp((float)bytesWritten  / fwSize, 0.f, 1.f));
        }
        return true;
    }
    bool sendBoot(mab::Candle& candle, u16 id, u32 fwStart)
    {
        char tx[64] = {}, rx[64] = {};
        tx[0]         = (u8)0xb5;
        *(u32*)&tx[1] = fwStart;
        return (candle.sendGenericFDCanFrame(id, 5, tx, rx, 200) && (strncmp(rx, "OK", 2) == 0));
    }
    bool sendMeta(mab::Candle& candle, u16 id, u8* checksum)
    {
        char tx[64] = {}, rx[64] = {};
        tx[0] = (u8)0xb6;
        tx[1] = (u8) true;
        memcpy(&tx[2], checksum, 32);
        candle.sendGenericFDCanFrame(id, 64, tx, nullptr, 10);
        usleep(300000); 
        // erase and save takes about 350ms, and this is done
        // due to a bug in candle fw allowing max of 255 ms timeout
        candle.sendGenericFDCanFrame(id, 0, tx, rx, 250);
        return (strncmp(rx, "OK", 2) == 0);
    }
}  // namespace canUpdater
