#include "edsParser.hpp"

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

std::string edsParser::generateObjectSection(const edsObject& obj)
{
    std::ostringstream oss;
    oss << "[" << std::hex << std::uppercase << std::setfill('0') << std::setw(4) << obj.index;
    if (obj.subIndex != 0)
        oss << "sub" << std::dec << (int)obj.subIndex;
    oss << "]\n";
    oss << "ParameterName=" << obj.ParameterName << "\n";
    oss << "ObjectType=0x" << std::hex << (int)obj.ObjectType << "\n";
    if (!obj.StorageLocation.empty())
        oss << ";StorageLocation=" << obj.StorageLocation << "\n";
    oss << "DataType=0x" << std::hex << obj.DataType << "\n";
    oss << "AccessType=" << obj.accessType << "\n";
    oss << "DefaultValue=0x" << std::hex << obj.defaultValue << "\n";
    oss << "PDOMapping=" << (obj.PDOMapping ? 1 : 0) << "\n";
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

void updateObjectTypeSection(std::map<std::string, std::string>& sectionMap,
                             SectionType_t                       type,
                             u32                                 index)
{
    std::string sectionName;
    switch (type)
    {
        case MandatoryObjects:
            sectionName = "MandatoryObjects";
            break;
        case OptionalObjects:
            sectionName = "OptionalObjects";
            break;
        case ManufacturerObjects:
            sectionName = "ManufacturerObjects";
            break;
    }

    std::string& sectionContent = sectionMap[sectionName];

    // Extract existing index
    std::set<u32>     indexSet;
    std::stringstream ss(sectionContent);
    std::string       line;
    while (std::getline(ss, line))
    {
        std::smatch match;
        if (std::regex_match(line, match, std::regex(R"((\d+)\s*=\s*0x([0-9A-Fa-f]{4}))")))
        {
            indexSet.insert(std::stoul(match[2], nullptr, 16));
        }
    }

    indexSet.insert(index);  // Add the new index

    // rebuild the section properly
    std::ostringstream out;
    out << "[" << sectionName << "]\n";
    out << "SupportedObjects=" << indexSet.size() << "\n";

    int i = 1;
    for (u32 idx : indexSet)
    {
        out << i << "=0x" << std::uppercase << std::hex << std::setw(4) << std::setfill('0') << idx
            << "\n";
        ++i;
    }

    sectionMap[sectionName] = out.str();
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

edsParser::~edsParser()
{
}

Error_t edsParser::load(const std::string& edsFilePath)
{
    mINI::INIFile      file(mab::getCanOpenConfigPath());
    mINI::INIStructure ini;
    file.read(ini);

    namespace fs = std::filesystem;

    if (!fs::exists(edsFilePath))
    {
        log.error("EDS file not found: %s", edsFilePath.c_str());
        return INVALID_PATH;
    }

    std::ifstream in(edsFilePath);
    if (!in.is_open())
    {
        log.error("Impossible to open the file containing the EDS file: %s", edsFilePath.c_str());
        return INVALID_PATH;
    }

    std::string line;
    std::getline(in, line);
    in.close();

    // cleaning (we removed spaces, tabs, and newlines)
    line.erase(0, line.find_first_not_of("\t\r\n"));
    line.erase(line.find_last_not_of("\t\r\n") + 1);

    if (line.empty())
    {
        log.error("eds_path.txt file empty or invalid.");
        return INVALID_PATH;
    }

    // Modify & Rewrite
    ini["eds"]["path"] = edsFilePath;
    file.write(ini);
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

Error_t edsParser::unload()
{
    mINI::INIFile      file(mab::getCanOpenConfigPath());
    mINI::INIStructure ini;
    file.read(ini);

    // Modify & Rewrite
    ini["eds"]["path"] = "";
    file.write(ini);
    log.success("EDS unloaded");
    return OK;
}

Error_t edsParser::display()
{
    std::stringstream ss;

    // update the path since the eds_path.txt file
    Error_t pathResult = this->updateFilePath();
    if (pathResult != OK)
    {
        log.error("Impossible to update the EDS path.\n");
        return pathResult;
    }

    std::ifstream edsFile(m_edsFilePath);
    if (!edsFile.is_open())
    {
        log.error("Impossible to open the EDS file: %s\n", m_edsFilePath.c_str());
        return INVALID_PATH;
    }

    ss << "\n--- Content of " << m_edsFilePath << "---\n";

    std::string line;
    while (std::getline(edsFile, line))
    {
        ss << line << std::endl;
    }

    edsFile.close();
    ss << "--- End of EDS file ---\n";

    log.success("%s", ss.str().c_str());

    return OK;
}

Error_t edsParser::get(u32 index, u8 subindex)
{
    if (updateFilePath() != OK)
    {
        log.error("Impossible to upload the EDS path.\n");
        return INVALID_PATH;
    }

    std::ifstream edsFile(m_edsFilePath);
    if (!edsFile.is_open())
    {
        log.error("Impossible to open the EDS file: %s\n", m_edsFilePath.c_str());
        return INVALID_PATH;
    }

    std::stringstream baseSectionStream;
    baseSectionStream << std::hex << std::uppercase << "[" << std::setw(4) << std::setfill('0')
                      << index << "]";
    std::string baseSection = baseSectionStream.str();

    std::stringstream subSectionStream;
    subSectionStream << std::hex << std::uppercase << "[" << std::setw(4) << std::setfill('0')
                     << index << "sub" << static_cast<int>(subindex) << "]";
    std::string requestedSubSection = subSectionStream.str();

    std::string line;
    bool        inSection    = false;
    bool        sectionFound = false;
    bool        wantAllSubs  = (subindex == 0x00);

    std::stringstream ss;

    ss << "--- Reading for index 0x" << std::hex << index;
    if (!wantAllSubs)
        ss << "subindex 0x" << std::hex << static_cast<int>(subindex);
    ss << "---\n";

    while (std::getline(edsFile, line))
    {
        // cleaning
        line.erase(0, line.find_first_not_of("\t\r\n"));
        line.erase(line.find_last_not_of("\t\r\n") + 1);

        if (line.empty() || line[0] == ';')
            continue;

        // begin of a new section
        if (line.front() == '[' && line.back() == ']')
        {
            inSection = false;

            if (wantAllSubs)
            {
                // We want baseSection or any [indexsubX]
                if (line == baseSection ||
                    (line.find(baseSection.substr(0, baseSection.size() - 1)) == 0 &&
                     line.find("sub") != std::string::npos))
                {
                    inSection    = true;
                    sectionFound = true;
                    ss << line << std::endl;
                }
            }
            else
            {
                // unique case : [indexsubX] only
                if (line == requestedSubSection)
                {
                    inSection    = true;
                    sectionFound = true;
                    ss << line << std::endl;
                }
            }
            continue;
        }

        if (inSection)
        {
            ss << line << std::endl;
        }
    }
    log.info("%s\n", ss.str().c_str());
    edsFile.close();

    if (!sectionFound)
    {
        log.warn("No section found for 0x%04X%s\n",
                 index,
                 (wantAllSubs ? "" : ("sub" + std::to_string(subindex)).c_str()));
        return INVALID_INDEX;
    }
    log.info("--- End of reading ---");
    return OK;
}

Error_t edsParser::find(const std::string& searchTerm)
{
    if (updateFilePath() != OK)
    {
        log.error("Impossible to upload the EDS path.\n");
        return INVALID_PATH;
    }

    std::ifstream edsFile(m_edsFilePath);
    if (!edsFile.is_open())
    {
        log.error("Impossible to open the EDS file: %s\n", m_edsFilePath.c_str());
        return INVALID_PATH;
    }

    std::string              lowerSearch = toLower(searchTerm);
    std::string              line;
    std::string              currentSection;
    std::vector<std::string> sectionLines;
    bool                     matchFound = false;
    std::stringstream        ss;

    while (std::getline(edsFile, line))
    {
        std::string trimmed = line;

        trimmed.erase(0, trimmed.find_first_not_of("\t\r\n"));
        trimmed.erase(trimmed.find_last_not_of("\t\r\n") + 1);

        if (trimmed.empty())
            continue;

        // new section detection
        if (trimmed.front() == '[' && trimmed.back() == ']')
        {
            // Check the previous section
            if (!sectionLines.empty())
            {
                for (const auto& l : sectionLines)
                {
                    if (toLower(l).find(lowerSearch) != std::string::npos)
                    {
                        ss << "--- corresponding Section  : " << currentSection << "---\n";
                        for (const auto& sl : sectionLines)
                            ss << sl << "\n";
                        ss << "--- Fin de section ---\n\n";
                        matchFound = true;
                        break;
                    }
                }
            }
            // Nouvelle section
            currentSection = trimmed;
            sectionLines.clear();
            sectionLines.push_back(trimmed);
        }
        else
        {
            sectionLines.push_back(line);
        }
    }

    // check last section if necessary
    if (!sectionLines.empty())
    {
        for (const auto& l : sectionLines)
        {
            if (toLower(l).find(lowerSearch) != std::string::npos)
            {
                ss << "--- Corresponding section : " << currentSection << "---\n";
                for (const auto& sl : sectionLines)
                    ss << sl << "\n";
                ss << "--- Fin de section ---\n\n";
                matchFound = true;
                break;
            }
        }
    }

    log.info("%s\n", ss.str().c_str());
    edsFile.close();

    if (!matchFound)
    {
        log.warn("No section is containing '%s'.\n", searchTerm.c_str());
        return INVALID_INDEX;
    }

    return OK;
}

Error_t edsParser::updateFilePath()
{
    mINI::INIFile      file(mab::getCanOpenConfigPath());
    mINI::INIStructure ini;
    file.read(ini);

    const std::string pathFile = ini["eds"]["path"];

    std::ifstream in(pathFile);
    if (!in.is_open())
    {
        log.error("Impossible to open the file containing the EDS file: %s\n", pathFile.c_str());
        return INVALID_PATH;
    }

    std::string line;
    std::getline(in, line);
    in.close();

    // cleaning (we removed spaces, tabs, and newlines)
    line.erase(0, line.find_first_not_of("\t\r\n"));
    line.erase(line.find_last_not_of("\t\r\n") + 1);

    if (line.empty())
    {
        log.error("eds_path.txt file empty or invalid.\n");
        return INVALID_PATH;
    }

    m_edsFilePath = pathFile;
    // m_edsFilePath = line;
    log.info("EDS path update from the eds_path.txt file : %s", m_edsFilePath.c_str());

    return OK;
}

bool edsParser::hasFileInfoSection()
{
    std::ifstream file(m_edsFilePath);
    std::string   line;
    while (std::getline(file, line))
    {
        if (line == "[FileInfo]")
            return true;
    }
    log.error("Section [FileInfo] is missing.\n");
    return false;
}

bool edsParser::hasMandatoryObjectsSection()
{
    std::ifstream file(m_edsFilePath);
    std::string   line;
    while (std::getline(file, line))
    {
        if (line == "[MandatoryObjects]")
            return true;
    }
    log.error("Section [MandatoryObjects] is missing.\n");
    return false;
}

bool edsParser::hasSupportedObjects()
{
    std::ifstream file(m_edsFilePath);
    std::string   line;
    bool          inSection = false;
    while (std::getline(file, line))
    {
        if (line == "[MandatoryObjects]")
            inSection = true;
        else if (!line.empty() && line[0] == '[')
            inSection = false;

        if (inSection && line.find("SupportedObjects=") != std::string::npos)
            return true;
    }
    log.error("Key section SupportedObjects is missing in [MandatoryObjects].\n");
    return false;
}

bool edsParser::hasMandatoryIndices()
{
    std::set<u32> required = {0x1000, 0x1001, 0x1018};

    std::ifstream file(m_edsFilePath);
    std::string   line;

    if (!file.is_open())
    {
        log.error("Impossible to open the EDS file.\n");
        return false;
    }

    std::regex sectionRegex(R"(\[\s*([0-9A-Fa-f]{4})\s*\])");  // only [XXXX]

    while (std::getline(file, line))
    {
        std::smatch m;
        if (std::regex_search(line, m, sectionRegex))
        {
            std::string found = m[1].str();
            log.debug("Section found : %s\n", found.c_str());

            try
            {
                u32 index = std::stoul(found, nullptr, 16);
                required.erase(index);
            }
            catch (const std::exception& e)
            {
                log.warn("Error while converting to hexa : %s\n", found.c_str());
            }
        }
    }

    if (!required.empty())
    {
        for (const auto& missingIndex : required)
        {
            std::ostringstream oss;
            oss << "0x" << std::hex << std::setw(4) << std::setfill('0') << std::uppercase
                << missingIndex;
            log.error("Mandatory object is missing: %s\n", oss.str().c_str());
        }
        return false;
    }

    return true;
}

bool edsParser::hasValidAccessTypes()
{
    std::ifstream file(m_edsFilePath);
    std::string   line;
    std::regex    pattern(R"(AccessType\s*=\s*(\w+))");  // Capture the value after AccessType=
    std::set<std::string> validTypes = {"ro", "rw", "wo", "const", "rwr", "rww"};

    while (std::getline(file, line))
    {
        std::smatch m;
        if (std::regex_search(line, m, pattern))
        {
            std::string value = m[1];
            std::transform(value.begin(), value.end(), value.begin(), ::tolower);
            if (validTypes.find(value) == validTypes.end())
            {
                log.error("Invalid AccessType  : %s\n", value.c_str());
                return false;
            }
        }
    }

    return true;
}

bool edsParser::hasValidDataTypes()
{
    std::ifstream file(m_edsFilePath);
    std::string   line;
    std::regex    pattern(
        R"(DataType\s*=\s*0x([0-9A-Fa-f]+))");  // Capture the hex value after DataType=

    while (std::getline(file, line))
    {
        std::smatch m;
        if (std::regex_search(line, m, pattern))
        {
            int value = std::stoi(m[1], nullptr, 16);
            if (value < 0x1 || value > 0x1C)  // Ex : standard CANopen types
            {
                log.warn("DataType suspect : 0x%X\n", value);
            }
        }
    }

    return true;
}
