#ifndef MAB_FILE_PARSER_HPP
#define MAB_FILE_PARSER_HPP

// #include <openssl/evp.h>

#include <fstream>
#include <string>
#include <vector>
#include "logger.hpp"

#include "mab_types.hpp"
#include "mini/ini.h"

class MabFileParser
{
  public:
    enum class TargetDevice_E : uint8_t
    {
        NONE    = 0,
        MD      = 1,
        CANDLE  = 2,
        PDS     = 3,
        BOOT    = 4,
        INVALID = 0xFF,
    };

    struct FirmwareEntry
    {
        static constexpr u32 maxSize       = 256 * 1024;
        TargetDevice_E       targetDevice  = TargetDevice_E::NONE;
        u8                   version[16]   = {};
        u32                  size          = 0x0;
        u32                  bootAddress   = 0x8000000;
        u8                   checksum[32]  = {};
        u8                   aes_iv[16]    = {};
        u8                   data[maxSize] = {};
    };

    MabFileParser() = delete;
    MabFileParser(std::string filePath, TargetDevice_E target);
    MabFileParser(MabFileParser&) = default;
    FirmwareEntry m_fwEntry;

  private:
    Logger log;
};

#endif /*MAB_FILE_PARSER_HPP*/
