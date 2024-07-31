#ifndef CANLOADER_HPP
#define CANLOADER_HPP

#include "iLoader.hpp"
#include "candle.hpp"
#include "logger.hpp"

class CanLoader : public iLoader
{
  public:
    CanLoader() = delete;
    CanLoader(mab::Candle& candle, mabFileParser& mabFile, uint32_t canId);

    ~CanLoader() = default;
    Error_E resetDevice() override;
    Error_E enterBootloader() override;
    Error_E uploadFirmware() override;
    Error_E sendBootCommand() override;

  private:
    enum BootloaderFrameId_E : uint8_t  // todo: suffix _E
    {
        CMD_TARGET_RESET = 0x13,
        CMD_HOST_INIT    = 0xA1,
        CMD_PAGE_PROG    = 0xA2,
        CMD_BOOT         = 0xA3,
        CMD_WRITE        = 0xA4,
    };

    mab::Candle& m_candle;
    uint32_t     m_canId;
    logger       m_log;
    size_t       m_fileSize;
    size_t       m_bytesToUpload;
    size_t       m_pagesToUpload;
    uint32_t     m_currentPage;

    void sendResetCmd();
    bool sendInitCmd();
    bool sendPageProgCmd();
    bool sendWriteCmd(uint8_t* pPageBuffer, int bufferSize);
    bool sendPage();
    bool sendBootCmd();
};

#endif /* CANLOADER_HPP */