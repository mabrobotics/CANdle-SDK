#include "edsParser.hpp"
#include <filesystem>

using namespace mab;

// Utility
std::string mab::toLower(const std::string& s)
{
    std::string out = s;
    std::transform(out.begin(), out.end(), out.begin(), ::tolower);
    return out;
}

std::string intToHex(u32 val, size_t width)
{
    std::ostringstream oss;
    oss << std::uppercase << std::hex << std::setw(width) << std::setfill('0') << val;
    return oss.str();
}

bool indexExistsInSection(const std::string& section, u32 index)
{
    std::smatch        match;
    std::regex         lineRegex(R"((\d+)=0x([0-9A-Fa-f]{4}))");  // Matches lines like "1=0x1000"
    std::string        line;
    std::istringstream stream(section);

    while (std::getline(stream, line))
    {
        if (std::regex_match(line, match, lineRegex))
        {
            u32 idx = std::stoul(match[2], nullptr, 16);
            if (idx == index)
                return true;
        }
    }
    return false;
}

void updateObjectListSection(std::string& section, u32 index)
{
    std::vector<u32>   indices;
    std::smatch        match;
    std::regex         entryRegex(R"((\d+)=0x([0-9A-Fa-f]{4}))");  // Matches lines like "1=0x1000"
    std::string        line;
    std::istringstream stream(section);
    std::string        newSection;
    int                maxKey = 0;

    while (std::getline(stream, line))
    {
        if (std::regex_match(line, match, entryRegex))
        {
            u32 value = std::stoul(match[2], nullptr, 16);
            indices.push_back(value);
            maxKey = std::max(maxKey, std::stoi(match[1]));
        }
        else
        {
            newSection += line + "\n";
        }
    }

    if (std::find(indices.begin(), indices.end(), index) == indices.end())
    {
        indices.push_back(index);
        std::sort(indices.begin(), indices.end());
    }

    newSection += "SupportedObjects=" + std::to_string(indices.size()) + "\n";
    for (size_t i = 0; i < indices.size(); ++i)
    {
        newSection += std::to_string(i + 1) + "=0x" + intToHex(indices[i], 4) + "\n";
    }

    section = newSection;
}

std::string mab::getObjectTypeName(const std::string& hex)
{
    if (hex == "0x7")
        return "VAR";
    if (hex == "0x8")
        return "ARRAY";
    if (hex == "0x9")
        return "RECORD";
    return hex;  // fallback
}

std::string mab::getDataTypeName(const std::string& hex)
{
    if (hex == "0x0001")
        return "BOOLEAN";
    if (hex == "0x0002")
        return "INTEGER8";
    if (hex == "0x0003")
        return "INTEGER16";
    if (hex == "0x0004")
        return "INTEGER32";
    if (hex == "0x0015")
        return "INTEGER64";
    if (hex == "0x0005")
        return "UNSIGNED8";
    if (hex == "0x0006")
        return "UNSIGNED16";
    if (hex == "0x0007")
        return "UNSIGNED32";
    if (hex == "0x001B")
        return "UNSIGNED64";
    if (hex == "0x0008")
        return "REAL32";
    if (hex == "0x0011")
        return "REAL64";
    if (hex == "0x0009")
        return "VISIBLE_STRING";
    if (hex == "0x000A")
        return "OCTET_STRING";
    if (hex == "0x000B")
        return "UNICODE_STRING";
    if (hex == "0x000F")
        return "DOMAIN";
    return hex;  // fallback
}

Error_t edsParser::load(const std::filesystem::path& edsFilePath)
{
    mINI::INIFile file(edsFilePath);
    file.read(m_edsIni);

    if (!std::filesystem::exists(edsFilePath))
    {
        log.error("EDS file not found: %s", edsFilePath.c_str());
        return INVALID_PATH;
    }

    if (!m_edsIni.has("FileInfo"))
    {
        log.error("%s is not an .eds file!", edsFilePath.c_str());
    }

    for (auto key_val : m_edsIni["FileInfo"])
    {
        log.info("%s - %s", key_val.first.c_str(), key_val.second.c_str());
    }
    log.success("EDS loaded:%s", edsFilePath.c_str());
    return OK;
}

Error_t edsParser::isValid()
{
    if (!hasFileInfoSection())
        return INVALID_FILE;
    if (!hasMandatoryObjectsSection())
        return INVALID_FILE;
    if (!hasSupportedObjects())
        return INVALID_FILE;
    if (!hasMandatoryIndices())
        return INVALID_FILE;
    if (!hasValidAccessTypes())
        return INVALID_FILE;
    if (!hasValidDataTypes())
        return INVALID_FILE;
    log.success("The EDS file is correct.");
    return OK;
}

Error_t edsParser::display()
{
    // TODO: see later if that makes sense here
    return Error_t::OK;
}

Error_t edsParser::get(u32 index, u8 subindex)
{
    return Error_t::UNKNOWN_ERROR;
}

Error_t edsParser::find(const std::string& searchTerm)
{
    return UNKNOWN_ERROR;
}

bool edsParser::hasFileInfoSection()
{
    return false;
}

bool edsParser::hasMandatoryObjectsSection()
{
    return false;
}

bool edsParser::hasSupportedObjects()
{
    return false;
}

bool edsParser::hasMandatoryIndices()
{
    return false;
}

bool edsParser::hasValidAccessTypes()
{
    return false;
}

bool edsParser::hasValidDataTypes()
{
    return false;
}
