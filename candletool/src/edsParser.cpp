#include "edsParser.hpp"
#include "configHelpers.hpp"
#include "mini/ini.h"

using namespace mab;

// Utility
std::string mab::toLower(const std::string& s)
{
    std::string out = s;
    std::transform(out.begin(), out.end(), out.begin(), ::tolower);
    return out;
}

std::string int_to_hex(u32 val, size_t width)
{
    std::ostringstream oss;
    oss << std::uppercase << std::hex << std::setw(width) << std::setfill('0') << val;
    return oss.str();
}

std::string toHex(u32 val, int width = 4)
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
    std::regex         lineRegex(R"((\d+)=0x([0-9A-Fa-f]{4}))");
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
    std::regex         entryRegex(R"((\d+)=0x([0-9A-Fa-f]{4}))");
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
        newSection += std::to_string(i + 1) + "=0x" + int_to_hex(indices[i], 4) + "\n";
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

Error_t edsParser::generateMarkdown(const std::string& path)
{
    this->updateFilePath();

    std::ifstream edsFile(m_edsFilePath);
    if (!edsFile)
    {
        log.error("Error : impossible to open the EDS file for the Markdown generation.\n");
        return INVALID_PATH;
    }

    std::string defaultPath = "./eds/eds_parsed.md";
    if (path != "")
        defaultPath = path;
    else
    {
        // Check and create the ./eds folder if necessary
        namespace fs = std::filesystem;
        if (!fs::exists("./eds"))
        {
            try
            {
                fs::create_directories("./eds");
            }
            catch (const std::exception& e)
            {
                log.error("Error : impossible to create ./eds directory: %s\n", e.what());
                return INVALID_PATH;
            }
        }
    }

    std::ofstream mdFile(defaultPath);
    if (!mdFile)
    {
        log.error("Error : impossible to create Markdown file.\n");
        return INVALID_PATH;
    }

    std::string                        line;
    std::string                        currentSection;
    std::map<std::string, std::string> currentFields;

    auto flushSection = [&]()
    {
        if (currentSection.empty() || currentFields.count("ObjectType") == 0)
            return;

        std::string title = currentSection;
        std::transform(title.begin(), title.end(), title.begin(), ::tolower);
        if (title.find("fileinfo") != std::string::npos)
            return;

        mdFile << "### 0x" << currentSection << "- ";
        mdFile << (currentFields["ParameterName"].empty() ? "???" : currentFields["ParameterName"])
               << "\n";

        mdFile << "| Object Type |\n";
        mdFile << "| ------------ |\n";
        mdFile << "| " << getObjectTypeName(currentFields["ObjectType"]) << "|\n\n";

        mdFile << "| Data Type      | SDO | PDO | SRDO | Default Value |\n";
        mdFile << "| -------------- | --- | --- | ---- | -------------- |\n";
        mdFile << "| " << getDataTypeName(currentFields["DataType"]) << "| "
               << (currentFields["AccessType"].empty() ? "???" : currentFields["AccessType"])
               << "| " << (currentFields["PDOMapping"].empty() ? "no" : currentFields["PDOMapping"])
               << "| "
               << "no | "
               << (currentFields["DefaultValue"].empty() ? "" : currentFields["DefaultValue"])
               << "|\n\n";

        currentFields.clear();
    };

    while (std::getline(edsFile, line))
    {
        if (line.empty())
            continue;

        // Section treatment
        if (line[0] == '[' && line.back() == ']')
        {
            flushSection();
            currentSection = line.substr(1, line.length() - 2);
            continue;
        }

        std::string actualLine = (line[0] == ';') ? line.substr(1) : line;
        actualLine.erase(0, actualLine.find_first_not_of("\t\r\n"));
        actualLine.erase(actualLine.find_last_not_of("\t\r\n") + 1);

        auto eqPos = actualLine.find('=');
        if (eqPos != std::string::npos)
        {
            std::string key   = actualLine.substr(0, eqPos);
            std::string value = actualLine.substr(eqPos + 1);

            key.erase(key.find_last_not_of("\t\r\n") + 1);
            key.erase(0, key.find_first_not_of("\t\r\n"));
            value.erase(value.find_last_not_of("\t\r\n") + 1);
            value.erase(0, value.find_first_not_of("\t\r\n"));

            std::transform(value.begin(), value.end(), value.begin(), ::tolower);

            currentFields[key] = value;
        }
    }

    flushSection();
    edsFile.close();
    mdFile.close();
    log.success("Markdown file generate : %s", defaultPath.c_str());
    return OK;
}

Error_t edsParser::generateHtml(const std::string& path)
{
    this->updateFilePath();

    std::ifstream edsFile(m_edsFilePath);
    if (!edsFile)
    {
        log.error("Error : impossible to open the EDS file for HTML generation.\n");
        return INVALID_PATH;
    }
    std::string defaultPath = "./eds/eds_parsed.html";
    if (path != "")
        defaultPath = path;
    else
    {
        // Check and create the ./eds folder if necessary
        namespace fs = std::filesystem;
        if (!fs::exists("./eds"))
        {
            try
            {
                fs::create_directories("./eds");
            }
            catch (const std::exception& e)
            {
                log.error("Error : impossible to create the ./eds folder: %s\n", e.what());
                return INVALID_PATH;
            }
        }
    }

    std::ofstream htmlFile(defaultPath);

    if (!htmlFile)
    {
        log.error("Error : impossible to create the HTML file.\n");
        return INVALID_PATH;
    }

    // Begin of HTML code
    htmlFile << "<!DOCTYPE html><html><head><meta charset='UTF-8'>\n";
    htmlFile << "<style>body { font-family: sans-serif; } h2 { margin-top: 30px; } "
                "table { border-collapse: collapse; width: 100%; margin-bottom: 30px; } "
                "th, td { border: 1px solid #ccc; padding: 8px; text-align: left; } "
                "th { background-color: #ebf1b5; }</style>\n";
    htmlFile << "<title>EDS Parsed</title></head><body>\n";
    htmlFile << "<h1>EDS File: " << m_edsFilePath << "</h1>\n";

    std::string                        line;
    std::string                        currentSection;
    std::map<std::string, std::string> currentFields;

    auto flushSection = [&]()
    {
        if (currentSection.empty() || currentFields.count("ObjectType") == 0)
            return;

        std::string title = currentSection;
        std::transform(title.begin(), title.end(), title.begin(), ::tolower);
        if (title.find("fileinfo") != std::string::npos)
            return;

        htmlFile << "<h2>0x" << currentSection << "- "
                 << (currentFields["ParameterName"].empty() ? "???"
                                                            : currentFields["ParameterName"])
                 << "</h2>\n";

        htmlFile << "<table>\n<tr><th>Object Type</th></tr>\n<tr><td>"
                 << getObjectTypeName(currentFields["ObjectType"]) << "</td></tr>\n</table>\n";

        htmlFile << "<table>\n";
        htmlFile << "<tr><th>Data Type</th><th>SDO</th><th>PDO</th><th>SRDO</th><th>Default "
                    "Value</th></tr>\n";

        htmlFile << "<tr>"
                 << "<td>" << getDataTypeName(currentFields["DataType"]) << "</td>"
                 << "<td>"
                 << (currentFields["AccessType"].empty() ? "???" : currentFields["AccessType"])
                 << "</td>"
                 << "<td>"
                 << (currentFields["PDOMapping"].empty() ? "no" : currentFields["PDOMapping"])
                 << "</td>"
                 << "<td>no</td>"
                 << "<td>"
                 << (currentFields["DefaultValue"].empty() ? "" : currentFields["DefaultValue"])
                 << "</td>"
                 << "</tr>\n</table>\n";

        currentFields.clear();
    };

    while (std::getline(edsFile, line))
    {
        if (line.empty())
            continue;

        if (line[0] == '[' && line.back() == ']')
        {
            flushSection();
            currentSection = line.substr(1, line.length() - 2);
            continue;
        }

        std::string actualLine = (line[0] == ';') ? line.substr(1) : line;
        actualLine.erase(0, actualLine.find_first_not_of("\t\r\n"));
        actualLine.erase(actualLine.find_last_not_of("\t\r\n") + 1);

        auto eqPos = actualLine.find('=');
        if (eqPos != std::string::npos)
        {
            std::string key   = actualLine.substr(0, eqPos);
            std::string value = actualLine.substr(eqPos + 1);

            key.erase(key.find_last_not_of("\t\r\n") + 1);
            key.erase(0, key.find_first_not_of("\t\r\n"));
            value.erase(value.find_last_not_of("\t\r\n") + 1);
            value.erase(0, value.find_first_not_of("\t\r\n"));

            std::transform(value.begin(), value.end(), value.begin(), ::tolower);

            currentFields[key] = value;
        }
    }

    flushSection();
    htmlFile << "</body></html>\n";

    edsFile.close();
    htmlFile.close();

    log.success("HTML file generate :%s", defaultPath.c_str());
    return OK;
}

Error_t edsParser::generateCpp(const std::string& path)
{
    this->updateFilePath();

    std::ifstream in(m_edsFilePath);
    if (!in.is_open())
    {
        log.error("Impossible to open EDS file.\n");
        return INVALID_PATH;
    }

    std::string defaultPath = "../../candletool/objectDictionary/";
    if (path != "")
        defaultPath = path;

    std::vector<edsObject> objects;
    edsObject              current;
    std::string            line;
    std::string            currentSection;

    std::regex sectionRegex(R"(\[([0-9A-Fa-f]{4})(?:sub([0-9A-Fa-f]+))?\])");

    while (std::getline(in, line))
    {
        std::smatch match;

        if (std::regex_match(line, match, sectionRegex))
        {
            if (!current.ParameterName.empty())
                objects.push_back(current);

            current          = edsObject();  // reset
            current.index    = std::stoul(match[1], nullptr, 16);
            current.subIndex = match[2].matched ? std::stoul(match[2], nullptr, 16) : 0;
            continue;
        }

        if (line.find("ParameterName=") == 0)
            current.ParameterName = line.substr(14);
        else if (line.find("ObjectType=0x") == 0)
            current.ObjectType = std::stoul(line.substr(13), nullptr, 16);
        else if (line.find(";StorageLocation=") == 0)
            current.StorageLocation = line.substr(17);
        else if (line.find("DataType=0x") == 0)
            current.DataType = std::stoul(line.substr(11), nullptr, 16);
        else if (line.find("AccessType=") == 0)
            current.accessType = line.substr(11);
        else if (line.find("DefaultValue=0x") == 0)
            current.defaultValue = std::stoul(line.substr(15), nullptr, 16);
        else if (line.find("PDOMapping=") == 0)
            current.PDOMapping = (line.substr(11) == "1");
    }

    if (!current.ParameterName.empty())
        objects.push_back(current);

    in.close();

    // Sort by index then subIndex
    std::sort(objects.begin(),
              objects.end(),
              [](const edsObject& a, const edsObject& b)
              { return (a.index < b.index) || (a.index == b.index && a.subIndex < b.subIndex); });

    // Write header file
    std::ofstream hpp((defaultPath + "OD.hpp"));
    hpp << "#pragma once\n";
    hpp << "#include <vector>\n";
    hpp << "#include \"edsParser.hpp\"\n\n";
    hpp << "namespace mab\n";
    hpp << "{\n";
    hpp << "std::vector<edsObject> generateObjectDictionary();\n";
    hpp << "}\n";
    hpp << "// This file is auto-generated from EDS file.\n";
    hpp.close();

    // Write cpp file
    std::ofstream cpp(defaultPath + "OD.cpp");
    cpp << "#include \"OD.hpp\"\n\n";
    cpp << "using namespace mab;\n\n";
    cpp << "std::vector<edsObject> mab::generateObjectDictionary() {\n";
    cpp << "   std::vector<edsObject> list;\n";

    for (const auto& obj : objects)
    {
        cpp << "   list.push_back(edsObject{\n";
        cpp << "       0x" << std::hex << obj.index << ", ";
        cpp << "0x" << std::hex << static_cast<int>(obj.subIndex) << ", ";
        cpp << "\"" << obj.ParameterName << "\", ";
        cpp << "0x" << std::hex << static_cast<int>(obj.ObjectType) << ", ";
        cpp << "\"" << obj.StorageLocation << "\", ";
        cpp << "0x" << std::hex << obj.DataType << ", ";
        cpp << "\"" << obj.accessType << "\", ";
        cpp << (obj.PDOMapping ? "true" : "false") << ", ";
        cpp << "0x" << std::hex << obj.defaultValue << "\n";
        cpp << "   });\n";
    }

    cpp << "   return list;\n";
    cpp << "}\n";
    cpp.close();

    log.success("OD.hpp et OD.cpp File generate.");
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

Error_t edsParser::addObject(const edsObject& obj)
{
    if (this->updateFilePath() != OK)
    {
        log.error("Impossible to upload the EDS path.\n");
        return INVALID_PATH;
    }

    std::ifstream in(m_edsFilePath);
    if (!in)
    {
        log.error("Error : impossible to open the EDS file\n");
        return INVALID_PATH;
    }

    std::vector<std::string> lines;
    std::string              line;
    while (std::getline(in, line))
    {
        lines.push_back(line);
    }
    in.close();

    std::string targetSection = (obj.subIndex > 0)
                                    ? (toHex(obj.index) + "sub" + std::to_string(obj.subIndex))
                                    : toHex(obj.index);

    // Delete the existing object if it's already there
    bool   objectExists = false;
    size_t sectionStart = std::string::npos;
    size_t sectionEnd   = std::string::npos;

    for (size_t i = 0; i < lines.size(); ++i)
    {
        if (lines[i] == "[" + targetSection + "]")
        {
            objectExists = true;
            sectionStart = i;
            for (size_t j = i + 1; j < lines.size(); ++j)
            {
                if (!lines[j].empty() && lines[j][0] == '[')
                {
                    sectionEnd = j;
                    break;
                }
            }
            if (sectionEnd == std::string::npos)
                sectionEnd = lines.size();
            break;
        }
    }

    if (objectExists)
    {
        std::string       response;
        std::stringstream ss;
        ss << "The object [" << targetSection
           << "] is already existing. Do you want to overwrite it ? (y/n): ";
        log.info("%s\n", ss.str().c_str());
        std::getline(std::cin, response);
        if (response != "y" && response != "Y")
            return UNKNOWN_ERROR;
        lines.erase(lines.begin() + sectionStart, lines.begin() + sectionEnd);
    }

    // Create the new object section
    std::ostringstream newSection;
    newSection << "[" << targetSection << "]\n";
    newSection << "ParameterName=" << obj.ParameterName << "\n";
    newSection << "ObjectType=0x" << std::hex << (int)obj.ObjectType << "\n";
    if (!obj.StorageLocation.empty())
        newSection << ";StorageLocation=" << obj.StorageLocation << "\n";
    if (obj.DataType > 0x0 && obj.DataType < 0x10)
        newSection << "DataType=0x000" << std::hex << obj.DataType << "\n";
    else if (obj.DataType > 0xF && obj.DataType < 0x100)
        newSection << "DataType=0x00" << std::hex << obj.DataType << "\n";
    else if (obj.DataType > 0xFF && obj.DataType < 0x1000)
        newSection << "DataType=0x0" << std::hex << obj.DataType << "\n";
    else
        newSection << "DataType=0x" << std::hex << obj.DataType << "\n";
    newSection << "AccessType=" << obj.accessType << "\n";
    newSection << "DefaultValue=0x" << std::hex << obj.defaultValue << "\n";
    newSection << "PDOMapping=" << (obj.PDOMapping ? "1" : "0") << "\n\n";

    std::string typeSection;
    switch (obj.sectionType)
    {
        case MandatoryObjects:
            typeSection = "MandatoryObjects";
            break;
        case OptionalObjects:
            typeSection = "OptionalObjects";
            break;
        case ManufacturerObjects:
            typeSection = "ManufacturerObjects";
            break;
    }

    // Update the [xxxObjects] section with correct index
    size_t                     typeStart = std::string::npos, typeEnd = lines.size();
    std::map<u32, std::string> supported;

    for (size_t i = 0; i < lines.size(); ++i)
    {
        if (lines[i] == "[" + typeSection + "]")
        {
            typeStart = i;
            for (size_t j = i + 1; j < lines.size(); ++j)
            {
                if (!lines[j].empty() && lines[j][0] == '[')
                {
                    typeEnd = j;
                    break;
                }
                std::smatch m;
                if (std::regex_match(lines[j], m, std::regex(R"((\d+)\s*=\s*0x([0-9A-Fa-f]{4}))")))
                {
                    supported[std::stoul(m[2], nullptr, 16)] = lines[j];
                }
            }
            break;
        }
    }

    supported[obj.index] = "";  // add or keep

    std::vector<std::string> newTypeSection;
    newTypeSection.push_back("[" + typeSection + "]");
    newTypeSection.push_back("SupportedObjects=" + std::to_string(supported.size()));
    int counter = 1;
    for (const auto& kv : supported)
    {
        std::ostringstream oss;
        oss << counter << "=0x" << std::uppercase << std::hex << std::setw(4) << std::setfill('0')
            << kv.first;
        newTypeSection.push_back(oss.str());
        ++counter;
    }

    if (typeStart != std::string::npos)
    {
        lines.erase(lines.begin() + typeStart, lines.begin() + typeEnd);
        lines.insert(lines.begin() + typeStart, newTypeSection.begin(), newTypeSection.end());
    }

    // Find the right place to insert the object in the sort order of the section
    size_t insertPos      = lines.size();
    bool   inCorrectBlock = false;
    for (size_t i = 0; i < lines.size(); ++i)
    {
        if (lines[i] == "[" + typeSection + "]")
        {
            inCorrectBlock = true;
            continue;
        }
        if (inCorrectBlock && lines[i].starts_with("["))
        {
            std::string sec    = lines[i].substr(1, lines[i].length() - 2);
            size_t      subPos = sec.find("sub");
            u32         cmpIdx = 0;
            u8          cmpSub = 0;
            try
            {
                if (subPos != std::string::npos)
                {
                    cmpIdx = std::stoul(sec.substr(0, subPos), nullptr, 16);
                    cmpSub = std::stoul(sec.substr(subPos + 3));
                }
                else
                {
                    cmpIdx = std::stoul(sec, nullptr, 16);
                }
                if (cmpIdx > obj.index || (cmpIdx == obj.index && cmpSub > obj.subIndex))
                {
                    insertPos = i;
                    break;
                }
            }
            catch (...)
            {
                continue;
            }
        }
    }

    // Insert the new section at the right place
    std::istringstream       objStream(newSection.str());
    std::vector<std::string> objLines;
    std::string              l;
    while (std::getline(objStream, l))
        objLines.push_back(l);

    lines.insert(lines.begin() + insertPos, objLines.begin(), objLines.end());

    std::ofstream out(m_edsFilePath);
    for (const std::string& l : lines)
        out << l << "\n";

    log.success("Object add/modify success : [%s]\n", targetSection.c_str());
    return OK;
}

Error_t edsParser::deleteObject(u32 index, u8 subindex)
{
    this->updateFilePath();

    std::ifstream inFile(m_edsFilePath);
    if (!inFile)
    {
        log.error("Error : impossible to open the EDS file.");
        return INVALID_PATH;
    }

    std::stringstream buffer;
    buffer << inFile.rdbuf();
    std::string content = buffer.str();
    inFile.close();

    // Id format of the section to delete
    char sectionID[20];
    if (subindex == 0)
        snprintf(sectionID, sizeof(sectionID), "%04X", index);
    else
        snprintf(sectionID, sizeof(sectionID), "%04Xsub%u", index, subindex);

    std::string pattern = "\\[" + std::string(sectionID) + "\\][^\\[]*";
    std::regex  sectionRegex(pattern, std::regex::icase);
    std::smatch match;

    if (!std::regex_search(content, match, sectionRegex))
    {
        log.warn("No section[%s] found in the EDS file.", sectionID);
        return UNKNOWN_ERROR;
    }

    content = std::regex_replace(content, sectionRegex, "");

    std::ofstream outFile(m_edsFilePath);
    if (!outFile)
    {
        log.error("Error : impossible to write in the EDS file.");
        return INVALID_PATH;
    }

    outFile << content;
    outFile.close();
    log.success("Section [%s] delete success.", sectionID);
    return OK;
}

Error_t edsParser::modifyObject(const edsObject& obj, u32 index, u8 subindex)
{
    this->updateFilePath();

    std::stringstream ss;

    // Delete the old section if existing
    Error_t delStatus = this->deleteObject(index, subindex);
    if (delStatus != OK)
    {
        log.error("Failed to delete the object [%04Xsub%u]\n", index, subindex);
        return delStatus;
    }

    // if it's RPDO we need verify if the new oject (fit) into the PDO (using predefined index
    // in the CiA 301)
    if ((obj.index == 0x1600 || obj.index == 0x1601 || obj.index == 0x1602 ||
         obj.index == 0x1603) &&
        (obj.subIndex <= 8))
    {
        int totalBits  = (obj.defaultValue & 0xFF);
        int entryCount = 1;

        std::ifstream edsFile(m_edsFilePath);
        if (!edsFile.is_open())
        {
            log.error("Impossible to open the EDS file: %s\n", m_edsFilePath.c_str());
            return INVALID_PATH;
        }

        std::string line, currentSection;
        bool        inSection = false;

        // while there is line to read into the eds file
        while (std::getline(edsFile, line))
        {
            std::string trimmed = line;
            trimmed.erase(0, trimmed.find_first_not_of(" \t\r\n"));
            trimmed.erase(trimmed.find_last_not_of(" \t\r\n") + 1);
            // if the actual line is empty
            if (trimmed.empty())
                continue;
            // if the actual line is containing []
            if (trimmed.front() == '[' && trimmed.back() == ']')
            {
                inSection = false;
            }
            // if the line containing the section in which we need to modify the PDO
            if (trimmed.front() == '[' && trimmed.back() == ']' &&
                trimmed.find("[" + int_to_hex(obj.index, 4)) != std::string::npos)
            {
                currentSection   = trimmed;
                std::string base = "[" + int_to_hex(obj.index, 4);
                inSection        = (currentSection.rfind(base, 0) == 0);
                continue;
            }
            // if the line containing the DefaultValue
            if (inSection && trimmed.find("DefaultValue=") != std::string::npos)
            {
                auto pos = trimmed.find('=');

                std::string value = trimmed.substr(pos);
                if ((int)value.size() >= 8)
                {
                    try
                    {
                        // We add the size of the object to the count
                        std::string sizeStr  = value.substr(9, 2);  // Last byte = bit size
                        int         sizeBits = std::stoi(sizeStr, nullptr, 16);
                        totalBits += sizeBits;
                        entryCount++;
                    }
                    catch (...)
                    {
                    }
                }
            }
        }
        edsFile.close();
        // if we have more than 8 object into 1 PDO
        if (entryCount > 8)
        {
            log.error("Mapping invalid : more than 8 entries in [%04X]\n", obj.index);
            return INVALID_FILE;
        }
        // if the new object is to big for our PDO
        if (totalBits > 64)
        {
            log.error("Mapping invalid : %d bits inside [%04X], the maximum is 64\n",
                      totalBits,
                      obj.index);
            return INVALID_FILE;
        }
    }

    // else if everything is ok, we can add the new section through addObject()
    Error_t addStatus = this->addObject(obj);
    if (addStatus != OK)
    {
        log.error("Failed to add the new object [%04Xsub%u]\n", obj.index, obj.subIndex);
        return addStatus;
    }

    ss << "object [0x" << std::hex << obj.index;
    if (obj.subIndex != 0)
        ss << "::" << std::hex << (int)obj.subIndex;
    ss << "] modified success.\n";

    log.success("%s", ss.str().c_str());

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
    std::ifstream         file(m_edsFilePath);
    std::string           line;
    std::regex            pattern(R"(AccessType\s*=\s*(\w+))");
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
    std::regex    pattern(R"(DataType\s*=\s*0x([0-9A-Fa-f]+))");

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
