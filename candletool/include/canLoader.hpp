#ifndef CANLOADER_HPP
#define CANLOADER_HPP

#include "iLoader.hpp"
#include "candle.hpp"
#include "logger.hpp"

class CanLoader : public I_Loader
{
  public:
    CanLoader() = delete;
    CanLoader(mab::Candle& candle, MabFileParser& mabFile, uint32_t canId);

    ~CanLoader() = default;
    Error_E resetDevice() override;
    Error_E enterBootloader() override;
    Error_E uploadFirmware() override;
    Error_E sendBootCommand() override;

  private:
    enum BootloaderFrameId_E : u8
    {
        CMD_TARGET_RESET = 0x13,
        CMD_SETUP        = 0xB1,
        CMD_ERASE        = 0xB2,
        CMD_PROG         = 0xB3,
        CMD_WRITE        = 0xB4,
        CMD_BOOT         = 0xB5,
        CMD_META         = 0xB6,
    };

    mab::Candle& m_candle;
    uint32_t     m_canId;
    Logger       m_log;
    size_t       m_fileSize;
    size_t       m_bytesToUpload;
    size_t       m_pagesToUpload;
    uint32_t     m_currentPage;

    void sendResetCmd();
    bool sendSetupCmd();
    bool sendEraseCmd();
    bool sendProgStartCmd();
    bool sendPage(u8* data);
    bool sendWriteCmd(u8* data);
    bool sendBootCmd();
    bool sendMetaCmd();
};

#endif /* CANLOADER_HPP */
