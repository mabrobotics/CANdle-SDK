#ifndef MAB_FILE_PARSER_HPP
#define MAB_FILE_PARSER_HPP

#include <array>
#include <string>
#include <memory>
#include "logger.hpp"

#include "mab_types.hpp"

class MabFileParser
{
  public:
    enum class TargetDevice_E : uint8_t
    {
        MD,
        CANDLE,
        PDS,
        INVALID = 0xFF,
    };

    struct FirmwareEntry
    {
        static constexpr u32 MAX_SIZE     = 256 * 1024;  // Maximum size of the firmware binary
        TargetDevice_E       targetDevice = TargetDevice_E::INVALID;  // Target device type
        u8                   version[16]  = {0};                      // Firmware version code
        u32                  size         = 0x0;                      // Size of the firmware binary
        u32                  bootAddress  = 0x0;                      // Start address of the MCU
        u8                   checksum[32] = {0};                      // SHA256 checksum
        u8                   aes_iv[16]   = {0};  // Initialization vector for AES encryption
        std::unique_ptr<std::array<u8, MAX_SIZE>> data =
            std::make_unique<std::array<u8, MAX_SIZE>>();  // Program binary
    };

    MabFileParser() = delete;
    MabFileParser(std::string filePath, TargetDevice_E target);
    MabFileParser(MabFileParser&) = delete;
    FirmwareEntry m_fwEntry;

  private:
    Logger log;
};

#endif /*MAB_FILE_PARSER_HPP*/
