#include "mabFileParser.hpp"
#include "mini/ini.h"
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

    // parse
    m_fwEntry.targetDevice = parseTargetDevice(ini.get("firmware").get("tag"));
    m_fwEntry.size         = atoi(ini.get("firmware").get("size").c_str());
    hexStringToBytes(
        m_fwEntry.checksum, sizeof(m_fwEntry.checksum), ini.get("firmware").get("checksum"));
    m_fwEntry.bootAddress = strtol(ini.get("firmware").get("start").c_str(), nullptr, 16);
    strcpy((char*)m_fwEntry.version, ini.get("firmware").get("version").c_str());
    hexStringToBytes(m_fwEntry.aes_iv, sizeof(m_fwEntry.aes_iv), ini.get("firmware").get("iv"));
    hexStringToBytes(
        m_fwEntry.data.get()->data(), m_fwEntry.size, ini.get("firmware").get("binary"));

    // validate
    if (target != m_fwEntry.targetDevice || m_fwEntry.targetDevice == TargetDevice_E::INVALID)
    {
        log.error("Error processing .mab file!");
        log.error("Device target mismatch. Expected: [%s], Read: [%s].",
                  tagFromTargetDevice(target).c_str(),
                  tagFromTargetDevice(m_fwEntry.targetDevice).c_str());
        throw std::runtime_error("Error processing file");
    }
    if (m_fwEntry.bootAddress < 0x8000000 || m_fwEntry.size == 0 ||
        m_fwEntry.size > m_fwEntry.data.get()->size())
    {
        log.error("Error processing .mab file!");
        log.error("Boot address [0x%x] or size of firmware [%d bytes] invalid!",
                  m_fwEntry.bootAddress,
                  m_fwEntry.size);
        throw std::runtime_error("Error processing file");
    }
    // TODO: Validate checksum here

    log.success(".mab file OK");
}

bool hexStringToBytes(u8 buffer[], u32 bufferLen, const std::string& str)
{
    memset(buffer, 0, bufferLen);
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
            return "md";
        case MabFileParser::TargetDevice_E::PDS:
            return "pds";
        case MabFileParser::TargetDevice_E::CANDLE:
            return "candle";
        default:
            return "UNKNOWN";
    }
}
MabFileParser::TargetDevice_E parseTargetDevice(std::string tag)
{
    if (tag == "md")
        return MabFileParser::TargetDevice_E::MD;
    else if (tag == "candle")
        return MabFileParser::TargetDevice_E::CANDLE;
    else if (tag == "pds")
        return MabFileParser::TargetDevice_E::PDS;
    else
        return MabFileParser::TargetDevice_E::INVALID;
}
