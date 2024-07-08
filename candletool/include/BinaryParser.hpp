#ifndef BINARY_PARSER_HPP
#define BINARY_PARSER_HPP

#include <openssl/evp.h>

#include <fstream>
#include <string>
#include <vector>
#include "logger.hpp"

#include "mini/ini.h"

class BinaryParser
{
  public:
    enum class Type : uint8_t
    {
        NONE   = 0,
        MD     = 1,
        CANDLE = 2,
        BOOT   = 3,
    };

    enum class Status : uint8_t
    {
        OK             = 0,
        ERROR_FILE     = 1,
        ERROR_TAG      = 2,
        ERROR_CHECKSUM = 3,
    };

    // static constexpr std::string MD_HEADER   = "MD";

  public:
    BinaryParser();
    Status               processFile(std::string filePath);
    std::vector<uint8_t> getPrimaryFirmwareFile();
    std::vector<uint8_t> getSecondaryFirmwareFile();
    BinaryParser::Type   getFirmwareFileType();

  private:
    struct FirmwareEntry
    {
        std::string          tag;
        size_t               size;
        std::string          checksum;
        std::vector<uint8_t> binary;
        Status               status = Status::OK;
        FirmwareEntry()
        {
        } /* this is to fix GCC bug with default initializers */
    };

    logger log;

    FirmwareEntry m_firmwareEntry1;
    FirmwareEntry m_firmwareEntry2;
    Type          m_fileType = Type::MD;

  private:
    FirmwareEntry        parseFirmwareEntry(mINI::INIStructure& ini, std::string&& header);
    std::vector<uint8_t> hexStringToBytes(std::string str);
    std::string          fileType2String(Type type);

    // TODO: Code left for future implementation
    // static bool validateChecksum(std::vector<uint8_t>& data, std::string& expectedChecksum);
};

#endif