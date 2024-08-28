#include "mabFileParser.hpp"

MabFileParser::MabFileParser(std::string filePath)
{
    log.m_tag   = "MAB FILE";
    log.m_layer = Logger::ProgramLayer_E::MIDDLE;
    if (Status_E::OK != processFile(filePath))
    {
        log.error("Error processing file\n\r[ %s ]\n\rCheck file path and format.",
                  filePath.c_str());
        throw std::runtime_error("Error processing file");
    }
}

// MabFileParser::MabFileParser(MabFileParser& MabFileParser)
// {
// }

MabFileParser::Status_E MabFileParser::processFile(std::string filePath)
{
    MabFileParser::log.info("Processing file: %s", filePath.c_str());

    mINI::INIFile      file(filePath);
    mINI::INIStructure ini;

    if (!file.read(ini))
        return Status_E::ERROR_FILE;

    m_firmwareEntry1 = parseFirmwareEntry(ini, std::string("header1"));

    if (m_firmwareEntry1.status == Status_E::ERROR_CHECKSUM ||
        m_firmwareEntry2.status == Status_E::ERROR_CHECKSUM)
    {
        log.error("Checksum validation failed");
        return Status_E::ERROR_CHECKSUM;
    }

    log.success("File processed successfully");
    log.info("Target device: [ %s ]", fileType2String(m_firmwareEntry1.targetDevice).c_str());
    log.info("Primary firmware size: [ %d bytes ]", m_firmwareEntry1.size);

    if ((m_firmwareEntry1.targetDevice == TargetDevice_E::BOOT) && (ini.has("header2")))
    {
        m_firmwareEntry2 = parseFirmwareEntry(ini, std::string("header2"));
        log.info("Secondary firmware size: [ %d bytes ]", m_firmwareEntry2.size);
    }

    return Status_E::OK;
}

MabFileParser::TargetDevice_E MabFileParser::parseTargetDevice(std::string tag)
{
    if (tag == "md")
        return TargetDevice_E::MD;

    else if (tag == "candle")
        return TargetDevice_E::CANDLE;

    else if (tag == "boot")
        return TargetDevice_E::BOOT;

    else if (tag == "pds")
        return TargetDevice_E::PDS;
    else
        return TargetDevice_E::INVALID;
}

MabFileParser::FirmwareEntry MabFileParser::parseFirmwareEntry(mINI::INIStructure& ini,
                                                               std::string&&       header)
{
    FirmwareEntry temp{};
    temp.targetDevice = parseTargetDevice(ini.get(header).get("tag"));
    temp.size         = stoi(ini.get(header).get("size"));
    temp.checksum     = ini.get(header).get("checksum");

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

std::vector<uint8_t> MabFileParser::hexStringToBytes(std::string str)
{
    std::vector<uint8_t> result;

    for (size_t i = 0; i < str.length(); i += 2)
    {
        std::string byteString = str.substr(i, 2);
        result.push_back(std::stoi(byteString, nullptr, 16));
    }

    return result;
}

std::string MabFileParser::fileType2String(TargetDevice_E type)
{
    switch (type)
    {
        case TargetDevice_E::MD:
            return "MD";
        case TargetDevice_E::CANDLE:
            return "CANDLE";
        case TargetDevice_E::BOOT:
            return "BOOT";
        default:
            return "UNKNOWN";
    }
}