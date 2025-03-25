#ifndef CANLOADER_HPP
#define CANLOADER_HPP

#include "iLoader.hpp"
#include "candle_v2.hpp"
#include "logger.hpp"
#include "mab_types.hpp"

class CanLoader : public I_Loader
{
  public:
    CanLoader() = delete;
    CanLoader(const mab::CandleV2& candle, MabFileParser& mabFile, uint32_t canId);

    ~CanLoader() = default;
    Error_E resetDevice() override;
    Error_E enterBootloader() override;
    Error_E uploadFirmware() override;
    Error_E sendBootCommand() override;

  private:
    static constexpr size_t                                 STANDARD_RESPONSE_SIZE = 2;
    static constexpr std::array<u8, STANDARD_RESPONSE_SIZE> STANDARD_RESPONSE      = {'O', 'K'};

    enum BootloaderFrameId_E : u8
    {
        CMD_TARGET_RESET = 0x13,
        CMD_SETUP        = 0xB1,  // Set Bootloder into SETUP state
        CMD_ERASE        = 0xB2,  // Erase FLASH memory
        CMD_PROG         = 0xB3,  // Init Firmware Data transfer
        CMD_WRITE        = 0xB4,  // Write Page to FLASH
        CMD_BOOT         = 0xB5,  // Boot to app
        CMD_META         = 0xB6,  // Set metadata, checksum etc. and save config in FLASH
    };

    MabFileParser&       m_mabFile;
    const mab::CandleV2& m_candle;
    u32                  m_canId;
    Logger               m_log;
    size_t               m_fileSize;
    size_t               m_bytesToUpload;
    size_t               m_pagesToUpload;
    u32                  m_currentPage;

    void sendResetCmd();
    bool sendSetupCmd();
    bool sendEraseCmd();
    bool sendProgTransferStartCmd();
    bool sendPage(u8* data);
    bool sendWriteCmd(u8* data);
    bool sendBootCmd();
    bool sendMetaCmd();

    inline std::pair<std::vector<u8>, mab::candleTypes::Error_t> transferCanFrame(
        std::vector<u8> frameToSend,
        u32             timeoutMs    = 1U,
        size_t          responseSize = STANDARD_RESPONSE_SIZE) const
    {
        auto result = m_candle.transferCANFrame(m_canId, frameToSend, responseSize, timeoutMs);

        if (result.second != mab::candleTypes::Error_t::OK)
        {
            m_log.error("Error while transfering CAN frame!");
        }
        return result;
    }
};

#endif /* CANLOADER_HPP */
