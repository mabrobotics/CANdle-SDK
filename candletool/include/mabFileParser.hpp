#ifndef MAB_FILE_PARSER_HPP
#define MAB_FILE_PARSER_HPP

// #include <openssl/evp.h>

#include <fstream>
#include <string>
#include <vector>
#include "logger.hpp"

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

    enum class Status_E : uint8_t
    {
        OK             = 0,
        ERROR_FILE     = 1,
        ERROR_TAG      = 2,
        ERROR_CHECKSUM = 3,
    };

    struct FirmwareEntry
    {
        std::string          mabFileVersion;
        TargetDevice_E       targetDevice;
        size_t               size;
        std::string          checksum;
        std::vector<uint8_t> binary;
        Status_E             status = Status_E::OK;
    };

    MabFileParser() = delete;
    MabFileParser(std::string filePath);
    MabFileParser(MabFileParser&) = default;

    FirmwareEntry m_firmwareEntry1;
    FirmwareEntry m_firmwareEntry2;

  private:
    Logger log;

    Status_E             processFile(std::string filePath);
    TargetDevice_E       parseTargetDevice(std::string tag);
    FirmwareEntry        parseFirmwareEntry(mINI::INIStructure& ini, std::string&& header);
    std::vector<uint8_t> hexStringToBytes(std::string str);
    std::string          fileType2String(TargetDevice_E type);

    // TODO: Code left for future implementation
    // static bool validateChecksum(std::vector<uint8_t>& data, std::string& expectedChecksum);
};

#endif /*MAB_FILE_PARSER_HPP*/