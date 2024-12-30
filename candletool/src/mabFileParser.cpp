#include "mabFileParser.hpp"
#include <cstring>

bool                          hexStringToBytes(u8 buffer[], u32 bufferLen, const std::string& str);
MabFileParser::TargetDevice_E parseTargetDevice(std::string tag);
std::string                   tagFromTargetDevice(MabFileParser::TargetDevice_E type);

MabFileParser::MabFileParser(std::string filePath, TargetDevice_E target)
{
    log.m_tag   = "MAB FILE";
    log.m_layer = Logger::ProgramLayer_E::LAYER_2;

    MabFileParser::log.info("Processing file: %s", filePath.c_str());

    mINI::INIFile      file(filePath);
    mINI::INIStructure ini;
    if (!file.read(ini))
    {
        log.error("Error processing file\n\r[ %s ]\n\rCheck file path and format.",
                  filePath.c_str());
        throw std::runtime_error("Error processing file");
    }
    
    m_fwEntry.targetDevice = parseTargetDevice(ini.get("firmware").get("tag"));
    m_fwEntry.size         = atoi(ini.get("firmware").get("size").c_str());
    hexStringToBytes(
        m_fwEntry.checksum, sizeof(m_fwEntry.checksum), ini.get("firmware").get("checksum"));
    m_fwEntry.bootAddress = strtol(ini.get("firmware").get("start").c_str(), nullptr, 16);
    strcpy((char*)m_fwEntry.version, ini.get("firmware").get("version").c_str());
    hexStringToBytes(m_fwEntry.aes_iv, sizeof(m_fwEntry.aes_iv), ini.get("firmware").get("iv"));
    memset(m_fwEntry.data, 0, sizeof(m_fwEntry.data));
    hexStringToBytes(m_fwEntry.data, m_fwEntry.size, ini.get("firmware").get("binary"));

    if (target != m_fwEntry.targetDevice)
    {
        log.error("Error processing .mab file!");
        log.error("Device target mismatch. Expected: [%s], Read: [%s].",
                  tagFromTargetDevice(target).c_str(),
                  tagFromTargetDevice(m_fwEntry.targetDevice).c_str());
        throw std::runtime_error("Error processing file");
    }
    if (m_fwEntry.bootAddress < 0x8000000 || m_fwEntry.size == 0 ||
        m_fwEntry.size > sizeof(m_fwEntry.data))
    {
        log.error("Error processing .mab file!");
        log.error("Boot address [0x%x] or size of firmware [%d bytes] invalid!",
                  m_fwEntry.bootAddress,
                  m_fwEntry.size);
        throw std::runtime_error("Error processing file");
    }
    //TODO: Validate checksum here
}

MabFileParser::TargetDevice_E parseTargetDevice(std::string tag)
{
    if (tag == "md")
        return MabFileParser::TargetDevice_E::MD;

    else if (tag == "candle")
        return MabFileParser::TargetDevice_E::CANDLE;

    else if (tag == "boot")
        return MabFileParser::TargetDevice_E::BOOT;

    else if (tag == "pds")
        return MabFileParser::TargetDevice_E::PDS;
    else
        return MabFileParser::TargetDevice_E::INVALID;
}

bool hexStringToBytes(u8 buffer[], u32 bufferLen, const std::string& str)
{
    if (bufferLen < (str.length() + 1) / 2 || str.length() % 2 == 1)
        return false;
    for (size_t i = 0; i < str.length() / 2; i++)
    {
        std::string byteString = str.substr(2 * i, 2);
        buffer[i]              = std::stoi(byteString.c_str(), nullptr, 16);
    }
    return true;
}
std::string tagFromTargetDevice(MabFileParser::TargetDevice_E type)
{
    switch (type)
    {
        case MabFileParser::TargetDevice_E::MD:
            return "MD";

        case MabFileParser::TargetDevice_E::PDS:
            return "PDS";

        case MabFileParser::TargetDevice_E::CANDLE:
            return "CANDLE";

        case MabFileParser::TargetDevice_E::BOOT:
            return "BOOT";

        default:
            return "UNKNOWN";
    }
}
