#ifndef AE13E62E_1F4C_49FF_93E9_B23DF7432112
#define AE13E62E_1F4C_49FF_93E9_B23DF7432112
#include "iLoader.hpp"
#include "candle.hpp"
#include "logger.hpp"

class UsbLoader : public iLoader
{
  public:
    UsbLoader() = delete;
    UsbLoader(mab::Candle& candle, mabFileParser& mabFile);

    ~UsbLoader() = default;
    Error_E resetDevice() override;
    Error_E enterBootloader() override;
    Error_E uploadFirmware() override;
    Error_E sendBootCommand() override;

  private:
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

#endif /* AE13E62E_1F4C_49FF_93E9_B23DF7432112 */
