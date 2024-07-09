#include "BinaryParser.hpp"

BinaryParser::BinaryParser()
{
    log.tag = "MAB FILE";
}

BinaryParser::Status BinaryParser::processFile(std::string filePath)
{
    BinaryParser::log.info("Processing file: %s", filePath.c_str());

    mINI::INIFile      file(filePath);
    mINI::INIStructure ini;

    if (!file.read(ini))
        return Status::ERROR_FILE;

    m_firmwareEntry1 = parseFirmwareEntry(ini, std::string("header1"));

    if (m_firmwareEntry1.tag == "md")
        m_fileType = Type::MD;

    else if (m_firmwareEntry1.tag == "candle")
        m_fileType = Type::CANDLE;

    else if (ini.has("header2"))
    {
        m_firmwareEntry2 = parseFirmwareEntry(ini, std::string("header2"));
        if (m_firmwareEntry1.tag == "boot_primary" && m_firmwareEntry2.tag == "boot_secondary")
            m_fileType = Type::BOOT;
        else
        {
            log.error("Invalid tag in header2. Expected 'boot_secondary' but got '%s'",
                      m_firmwareEntry2.tag.c_str());
            return Status::ERROR_TAG;
        }
    }
    else
    {
        log.error("Invalid tag in header1. Expected 'md' or 'candle' but got '%s'",
                  m_firmwareEntry1.tag.c_str());
        return Status::ERROR_TAG;
    }

    if (m_firmwareEntry1.status == Status::ERROR_CHECKSUM ||
        m_firmwareEntry2.status == Status::ERROR_CHECKSUM)
    {
        log.error("Checksum validation failed");
        return Status::ERROR_CHECKSUM;
    }

    log.success("File processed successfully");
    log.info("File type: [ %s ]", fileType2String(m_fileType).c_str());
    log.info("Primary firmware size: [ %d bytes ]", m_firmwareEntry1.size);
    if (m_fileType == Type::BOOT)
        log.info("Secondary firmware size: [ %d bytes ]", m_firmwareEntry2.size);

    return Status::OK;
}

std::vector<uint8_t> BinaryParser::getPrimaryFirmwareFile()
{
    return m_firmwareEntry1.binary;
}

std::vector<uint8_t> BinaryParser::getSecondaryFirmwareFile()
{
    return m_firmwareEntry2.binary;
}

BinaryParser::Type BinaryParser::getFirmwareFileType()
{
    return m_fileType;
}

BinaryParser::FirmwareEntry BinaryParser::parseFirmwareEntry(mINI::INIStructure& ini,
                                                             std::string&&       header)
{
    FirmwareEntry temp{};
    temp.tag      = ini.get(header).get("tag");
    temp.size     = stoi(ini.get(header).get("size"));
    temp.checksum = ini.get(header).get("checksum");

    temp.binary = hexStringToBytes(ini.get(header).get("binary"));

    // if (!validateChecksum(temp.binary, temp.checksum))
    // 	temp.status = Status::ERROR_CHECKSUM;

    return temp;
}

// static bool validateChecksum(std::vector<uint8_t>& data, std::string& expectedChecksum)
// {
// 	constexpr size_t sha256DigestLength = 32;
// 	EVP_MD_CTX*		 mdctx				= EVP_MD_CTX_new();

// 	if (mdctx == NULL)
// 		return false;

// 	EVP_DigestInit(mdctx, EVP_sha256());
// 	EVP_DigestUpdate(mdctx, data.data(), data.size());
// 	uint32_t calculateChecksumLength;
// 	uint8_t	 calculateChecksum[sha256DigestLength];
// 	if (EVP_DigestFinal(mdctx, calculateChecksum, &calculateChecksumLength) != 1)
// 	{
// 		EVP_MD_CTX_free(mdctx);
// 		return false;
// 	}

// 	auto expectedChecksumBytes = hexStringToBytes(expectedChecksum);

// 	for (size_t i = 0; i < sha256DigestLength; i++)
// 	{
// 		if (expectedChecksumBytes[i] != calculateChecksum[i])
// 			return false;
// 	}

// 	return true;
// }

std::vector<uint8_t> BinaryParser::hexStringToBytes(std::string str)
{
    std::vector<uint8_t> result;

    for (size_t i = 0; i < str.length(); i += 2)
    {
        std::string byteString = str.substr(i, 2);
        result.push_back(std::stoi(byteString, nullptr, 16));
    }

    return result;
}

std::string BinaryParser::fileType2String(Type type)
{
    switch (type)
    {
        case Type::MD:
            return "MD";
        case Type::CANDLE:
            return "CANDLE";
        case Type::BOOT:
            return "BOOT";
        default:
            return "UNKNOWN";
    }
}