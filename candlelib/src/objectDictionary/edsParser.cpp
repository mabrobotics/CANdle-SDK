#include "edsParser.hpp"
#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <memory>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include "logger.hpp"
#include "mini/ini.h"

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

std::pair<std::shared_ptr<EDSObjectDictionary>, EDSParser::Error_t> EDSParser::load(
    const std::filesystem::path& edsFilePath)
{
    Logger                                 log(Logger::ProgramLayer_E::TOP, "EDS Parser");
    std::map<u16, EDSEntry>                odMap;
    std::map<std::pair<u16, u8>, EDSEntry> subEntryMap;

    mINI::INIStructure ini;

    if (!std::filesystem::exists(edsFilePath))
    {
        log.error("EDS file not found: %s", edsFilePath.c_str());
        return std::make_pair<std::shared_ptr<EDSObjectDictionary>, Error_t>(
            std::make_shared<EDSObjectDictionary>(EDSObjectDictionary(std::move(odMap))),
            INVALID_PATH);
    }

    mINI::INIFile file(edsFilePath);
    if (!file.read(ini))
    {
        log.error("mINI parser failed to read a file");
    }

    if (!ini.has("DeviceInfo"))
    {
        log.error("%s is not an .eds file!", edsFilePath.c_str());
    }

    for (auto key_val : ini["FileInfo"])
    {
        log.debug("%s - %s", key_val.first.c_str(), key_val.second.c_str());
    }
    log.info("EDS loaded:%s", edsFilePath.c_str());

    for (auto key_val : ini)
    {
        log.debug("Looking at :%s", key_val.first.c_str());
        if (isEntry(key_val.first) || isSubentry(key_val.first))
        {
            EDSEntry::EDSEntryMetaData metaData;
            auto&                      entry = key_val.second;
            metaData.parameterName           = entry["ParameterName"];
            u32 idx    = isEntry(key_val.first) ? std::stoul(key_val.first, nullptr, 16)
                                                : extractIndexAndSubindex(key_val.first).value().first;
            u8  subidx = isSubentry(key_val.first)
                             ? extractIndexAndSubindex(key_val.first).value().second
                             : 0;

            std::optional<u8> subidxOpt;
            if (isSubentry(key_val.first))
            {
                subidxOpt = subidx;
            }
            metaData.address = std::pair<u32, std::optional<u8>>(idx, std::move(subidxOpt));

            // Fill object type
            switch (std::stoul(entry["ObjectType"].c_str(), nullptr, 16))
            {
                case 0x7:
                    metaData.objectType = EDSEntry::ObjectType_E::VALUE;
                    break;
                case 0x8:
                    metaData.objectType = EDSEntry::ObjectType_E::ARRAY;
                    break;
                case 0x9:
                    metaData.objectType = EDSEntry::ObjectType_E::RECORD;
                    break;
                default:
                    std::stringstream ss;
                    ss << entry["ObjectType"] << " is not a valid type!";
                    throw std::runtime_error(ss.str());
                    break;
            }
            // Fill storage location
            if (std::strcmp(entry["StorageLocation"].c_str(), "PERSIST_COMM") == 0)
            {
                metaData.storageLocation = EDSEntry::StorageLocation_E::PERSIST_COMM;
            }
            else
            {
                metaData.storageLocation = EDSEntry::StorageLocation_E::RAM;
            }

            // Create either value or container
            if (metaData.objectType == EDSEntry::ObjectType_E::VALUE)
            {
                auto edsValueMetadata = parseValueMetadata(entry);
                metaData.edsValueMeta = std::move(edsValueMetadata);

                if (isEntry(key_val.first))
                    odMap.emplace(idx, EDSEntry(std::move(metaData)));
                else
                    subEntryMap.emplace(std::pair<u32, u8>(idx, subidx),
                                        EDSEntry(std::move(metaData)));
            }
        }
    }

    for (auto key_val : ini)
    {
        log.debug("Looking at :%s", key_val.first.c_str());
        if (isEntry(key_val.first))
        {
            EDSEntry::EDSEntryMetaData metaData;
            auto&                      entry = key_val.second;
            metaData.parameterName           = entry["ParameterName"];
            u32 idx                          = std::stoul(key_val.first, nullptr, 16);

            // Fill object type
            switch (std::stoul(entry["ObjectType"].c_str(), nullptr, 0))
            {
                case 0x7:
                    metaData.objectType = EDSEntry::ObjectType_E::VALUE;
                    break;
                case 0x8:
                    metaData.objectType = EDSEntry::ObjectType_E::ARRAY;
                    break;
                case 0x9:
                    metaData.objectType = EDSEntry::ObjectType_E::RECORD;
                    break;
                default:
                    std::stringstream ss;
                    ss << entry["ObjectType"] << " is not a valid type!";
                    throw std::runtime_error(ss.str());
                    break;
            }
            // Fill storage location
            if (std::strcmp(entry["StorageLocation"].c_str(), "PERSIST_COMM") == 0)
            {
                metaData.storageLocation = EDSEntry::StorageLocation_E::PERSIST_COMM;
            }
            else
            {
                metaData.storageLocation = EDSEntry::StorageLocation_E::RAM;
            }

            // Create either value or container
            if (metaData.objectType != EDSEntry::ObjectType_E::VALUE)
            {
                EDSEntry::EDSContainerMetaData edsContainerMetadata;
                edsContainerMetadata.numberOfSubindices =
                    std::stoul(entry["SubNumber"], nullptr, 16);
                metaData.edsContainerMeta = std::move(edsContainerMetadata);

                std::map<u8, std::unique_ptr<EDSEntry>> map;

                for (size_t i = 0; i < std::stoul(entry["SubNumber"], nullptr, 16); i++)
                {
                    map.emplace(i,
                                std::make_unique<EDSEntry>(
                                    std::move(subEntryMap.at(std::pair<u32, u8>(idx, i)))));
                }
                odMap.emplace(idx, EDSEntry(std::move(metaData), std::move(map)));
            }
        }
    }

    return std::make_pair<std::shared_ptr<EDSObjectDictionary>, Error_t>(
        std::make_shared<EDSObjectDictionary>(EDSObjectDictionary(std::move(odMap))), OK);
}

EDSParser::Error_t EDSParser::isValid()
{
    return INVALID_FILE;
}

EDSParser::Error_t EDSParser::display()
{
    // TODO: see later if that makes sense here
    return Error_t::OK;
}

EDSParser::Error_t EDSParser::get(u32 index, u8 subindex)
{
    return EDSParser::Error_t::UNKNOWN_ERROR;
}

EDSParser::Error_t EDSParser::find(const std::string& searchTerm)
{
    return UNKNOWN_ERROR;
}

EDSEntry::EDSValueMetaData EDSParser::parseValueMetadata(mINI::INIMap<std::string>& entry)
{
    EDSEntry::EDSValueMetaData edsValueMetadata;
    edsValueMetadata.defaultValueStr = entry["DefaultValue"];
    switch (std::stoul(entry["DataType"].c_str(), nullptr, 16))
    {
        case 0x0001:
            edsValueMetadata.dataType = EDSEntry::DataType_E::BOOLEAN;
            break;

        case 0x0002:
            edsValueMetadata.dataType = EDSEntry::DataType_E::INTEGER8;
            break;

        case 0x0003:
            edsValueMetadata.dataType = EDSEntry::DataType_E::INTEGER16;
            break;

        case 0x0004:
            edsValueMetadata.dataType = EDSEntry::DataType_E::INTEGER32;
            break;

        case 0x0015:
            edsValueMetadata.dataType = EDSEntry::DataType_E::INTEGER64;
            break;

        case 0x0005:
            edsValueMetadata.dataType = EDSEntry::DataType_E::UNSIGNED8;
            break;

        case 0x0006:
            edsValueMetadata.dataType = EDSEntry::DataType_E::UNSIGNED16;
            break;

        case 0x0007:
            edsValueMetadata.dataType = EDSEntry::DataType_E::UNSIGNED32;
            break;

        case 0x001B:
            edsValueMetadata.dataType = EDSEntry::DataType_E::UNSIGNED64;
            break;

        case 0x0008:
            edsValueMetadata.dataType = EDSEntry::DataType_E::REAL32;
            break;

        case 0x0011:
            edsValueMetadata.dataType = EDSEntry::DataType_E::REAL64;
            break;

        case 0x0009:
            edsValueMetadata.dataType = EDSEntry::DataType_E::VISIBLE_STRING;
            break;

        case 0x000A:
            edsValueMetadata.dataType = EDSEntry::DataType_E::OCTET_STRING;
            break;

        case 0x000F:
            edsValueMetadata.dataType = EDSEntry::DataType_E::DOMAIN;
            break;

        default:
        {
            std::stringstream ss;
            ss << std::stoul(entry["DataType"].c_str(), nullptr, 16)
               << " is not a valid data type!";
            throw std::runtime_error(ss.str());
        }
    }
    edsValueMetadata.PDOMapping =
        std::stoul(entry["PDOMapping"].c_str(), nullptr, 16) ? true : false;
    if (std::strcmp(entry["AccessType"].c_str(), "ro") == 0)
    {
        edsValueMetadata.accessType = EDSEntry::AccessRights_E::READ_ONLY;
    }
    else
    {
        edsValueMetadata.accessType = EDSEntry::AccessRights_E::READ_WRITE;
    }
    return edsValueMetadata;
}
