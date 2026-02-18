#include <cstddef>
#include <optional>
#include <sstream>
#include <string>
#include <utility>
#include <stdexcept>
#include <variant>

#include "edsEntry.hpp"

namespace mab
{

    const EDSEntry::EDSEntryMetaData& EDSEntry::getEntryMetaData() const noexcept
    {
        return m_edsEntryMetaData;
    }
    const std::optional<EDSEntry::EDSValueMetaData> EDSEntry::getValueMetaData() const noexcept
    {
        return m_edsEntryMetaData.edsValueMeta;
    }
    const std::optional<EDSEntry::EDSContainerMetaData> EDSEntry::getContainerMetaData()
        const noexcept
    {
        return m_edsEntryMetaData.edsContainerMeta;
    }

    EDSEntry::EDSEntry(EDSEntryMetaData&& edsEntryMetaData) : m_edsEntryMetaData(edsEntryMetaData)
    {
        if (m_edsEntryMetaData.objectType != ObjectType_E::VALUE)
            throw std::runtime_error("Invalid constructor for value entry!");

        if (!m_edsEntryMetaData.edsValueMeta.has_value())
        {
            std::stringstream ss;
            ss << "Invalid EDS entry generation at entry " << m_edsEntryMetaData.parameterName;
            throw std::runtime_error(ss.str());
        }

        m_value = getVariantFromString(m_edsEntryMetaData.edsValueMeta.value().dataType,
                                       m_edsEntryMetaData.edsValueMeta.value().defaultValueStr);
        if (std::holds_alternative<std::monostate>(m_value.value()))
        {
            throw std::runtime_error("Invalid default value or type!");
        }
    }

    EDSEntry::EDSEntry(EDSEntryMetaData&&                        edsEntryMetaData,
                       std::map<u8, std::unique_ptr<EDSEntry>>&& subObjectsMap)
        : m_edsEntryMetaData(std::move(edsEntryMetaData)), m_subObjectsMap(std::move(subObjectsMap))
    {
        if (m_edsEntryMetaData.objectType == ObjectType_E::VALUE)
            throw std::runtime_error("Invalid constructor for container entry!");

        if (!m_edsEntryMetaData.edsContainerMeta.has_value())
        {
            std::stringstream ss;
            ss << "Invalid EDS entry generation at entry " << m_edsEntryMetaData.parameterName;
            throw std::runtime_error(ss.str());
        }

        if (m_subObjectsMap.value().size() !=
            m_edsEntryMetaData.edsContainerMeta.value().numberOfSubindices)
        {
            std::stringstream ss;
            ss << "Invalid subindecies in map generation at entry "
               << m_edsEntryMetaData.parameterName;
            throw std::runtime_error(ss.str());
        }
    }

    std::vector<std::byte> EDSEntry::getSerializedValue() const noexcept
    {
        if (!m_edsEntryMetaData.edsValueMeta.has_value() || !m_value.has_value())
            return {};

        if (std::holds_alternative<open_types::INTEGER8_t>(m_value.value()) ||
            std::holds_alternative<open_types::INTEGER16_t>(m_value.value()) ||
            std::holds_alternative<open_types::INTEGER32_t>(m_value.value()) ||
            std::holds_alternative<open_types::INTEGER64_t>(m_value.value()))
        {
            auto v =
                std::as_bytes(std::span(&std::get<open_types::INTEGER64_t>(m_value.value()), 1));
            return std::vector<std::byte>(v.begin(), v.end());
        }
        else if (std::holds_alternative<open_types::BOOLEAN_t>(m_value.value()) ||
                 std::holds_alternative<open_types::UNSIGNED8_t>(m_value.value()) ||
                 std::holds_alternative<open_types::UNSIGNED16_t>(m_value.value()) ||
                 std::holds_alternative<open_types::UNSIGNED32_t>(m_value.value()) ||
                 std::holds_alternative<open_types::UNSIGNED64_t>(m_value.value()))
        {
            auto v =
                std::as_bytes(std::span(&std::get<open_types::UNSIGNED64_t>(m_value.value()), 1));
            return std::vector<std::byte>(v.begin(), v.end());
        }
        else if (std::holds_alternative<open_types::REAL32_t>(m_value.value()))
        {
            auto v = std::as_bytes(std::span(&std::get<open_types::REAL32_t>(m_value.value()), 1));
            return std::vector<std::byte>(v.begin(), v.end());
        }
        else if (std::holds_alternative<open_types::DOMAIN_t>(m_value.value()))
        {
            return std::get<open_types::DOMAIN_t>(m_value.value());
        }
        else if (std::holds_alternative<open_types::VISIBLE_STRING_t>(m_value.value()))
        {
            const std::string&     str = std::get<open_types::VISIBLE_STRING_t>(m_value.value());
            std::vector<std::byte> result;
            for (const char& c : str)
            {
                result.push_back(static_cast<std::byte>(c));
            }
            return result;
        }
        else
            return {};
    }
    EDSEntry::Error_t EDSEntry::setSerializedValue(const std::span<std::byte> bytes)
    {
        if (!m_edsEntryMetaData.edsValueMeta.has_value() || !m_value.has_value())
        {
            return Error_t::INCORRECT_USE;
        }

        switch (m_edsEntryMetaData.edsValueMeta.value().dataType)
        {
            case DataType_E::BOOLEAN:
                m_value = *std::bit_cast<const open_types::BOOLEAN_t*>(bytes.data());
                break;
            case DataType_E::INTEGER8:
                m_value = *std::bit_cast<const open_types::INTEGER8_t*>(bytes.data());
                break;
            case DataType_E::INTEGER16:
                m_value = *std::bit_cast<const open_types::INTEGER16_t*>(bytes.data());
                break;
            case DataType_E::INTEGER32:
                m_value = *std::bit_cast<const open_types::INTEGER32_t*>(bytes.data());
                break;
            case DataType_E::INTEGER64:
                m_value = *std::bit_cast<const open_types::INTEGER64_t*>(bytes.data());
                break;
            case DataType_E::UNSIGNED8:
                m_value = *std::bit_cast<const open_types::UNSIGNED8_t*>(bytes.data());
                break;
            case DataType_E::UNSIGNED16:
                m_value = *std::bit_cast<const open_types::UNSIGNED16_t*>(bytes.data());
                break;
            case DataType_E::UNSIGNED32:
                m_value = *std::bit_cast<const open_types::UNSIGNED32_t*>(bytes.data());
                break;
            case DataType_E::UNSIGNED64:
                m_value = *std::bit_cast<const open_types::UNSIGNED64_t*>(bytes.data());
                break;
            case DataType_E::REAL32:
                m_value = *std::bit_cast<const open_types::REAL32_t*>(bytes.data());
                break;
            case DataType_E::DOMAIN:
                m_value = open_types::DOMAIN_t(bytes.begin(), bytes.end());
                break;
            case DataType_E::VISIBLE_STRING:
            {
                std::string result;
                for (const auto& byte : bytes)
                {
                    result += static_cast<char>(byte);
                }
                m_value = result;
            }
            break;
            default:
                return Error_t::PARSING_FAILED;
        }
        return Error_t::OK;
    }

    std::string EDSEntry::getAsString() const noexcept
    {
        if (!m_edsEntryMetaData.edsValueMeta.has_value() || !m_value.has_value())
            return "INCORRECT USAGE";
        return getStringFromVariant(m_edsEntryMetaData.edsValueMeta.value().dataType,
                                    m_value.value());
    }
    EDSEntry::Error_t EDSEntry::setFromString(const std::string_view str)
    {
        if (!m_edsEntryMetaData.edsValueMeta.has_value() || !m_value.has_value())
            return Error_t::INCORRECT_USE;
        m_value = getVariantFromString(m_edsEntryMetaData.edsValueMeta.value().dataType, str);
        if (std::holds_alternative<std::monostate>(m_value.value()))
        {
            return Error_t::PARSING_FAILED;
        }
        return Error_t::OK;
    }
    size_t EDSEntry::valueSize() const noexcept
    {
        if (!m_value.has_value() || !m_edsEntryMetaData.edsValueMeta.has_value())
            return 0;
        size_t size = 0;
        switch (m_edsEntryMetaData.edsValueMeta.value().dataType)
        {
            case mab::EDSEntry::DataType_E::BOOLEAN:
            case mab::EDSEntry::DataType_E::INTEGER8:
            case mab::EDSEntry::DataType_E::UNSIGNED8:
                size = 1;
                break;
            case mab::EDSEntry::DataType_E::INTEGER16:
            case mab::EDSEntry::DataType_E::UNSIGNED16:
                size = 2;
                break;
            case mab::EDSEntry::DataType_E::INTEGER32:
            case mab::EDSEntry::DataType_E::UNSIGNED32:
            case mab::EDSEntry::DataType_E::REAL32:
                size = 4;
                break;
            case mab::EDSEntry::DataType_E::INTEGER64:
            case mab::EDSEntry::DataType_E::UNSIGNED64:
            case mab::EDSEntry::DataType_E::REAL64:
                size = 8;
                break;
            case mab::EDSEntry::DataType_E::VISIBLE_STRING:
                size = std::get<open_types::VISIBLE_STRING_t>(m_value.value()).size();
                break;
            case mab::EDSEntry::DataType_E::OCTET_STRING:
                size = std::get<open_types::OCTET_STRING_t>(m_value.value()).size();
                break;
            case mab::EDSEntry::DataType_E::UNICODE_STRING:
                size = std::get<open_types::UNICODE_STRING_t>(m_value.value()).size();
                break;
            case mab::EDSEntry::DataType_E::DOMAIN:
                size = std::get<open_types::DOMAIN_t>(m_value.value()).size();
                break;
        }
        return size;
    }
    EDSEntry::ValueVariant_t EDSEntry::getVariantFromString(const DataType_E&      dataType,
                                                            const std::string_view str)
    {
        std::string            val(str);
        std::vector<std::byte> byteResult;
        switch (dataType)
        {
            case DataType_E::BOOLEAN:
                if (val.empty())
                    return {};
                return str == "TRUE" ? open_types::BOOLEAN_t(true) : open_types::BOOLEAN_t(false);
            case DataType_E::INTEGER8:
                if (val.empty())
                    return open_types::INTEGER8_t(0);
                return open_types::INTEGER8_t(std::stoi(std::string(str).c_str(), nullptr, 0));
            case DataType_E::INTEGER16:
                if (val.empty())
                    return open_types::INTEGER16_t(0);
                return open_types::INTEGER16_t(std::stoi(std::string(str).c_str(), nullptr, 0));
            case DataType_E::INTEGER32:
                if (val.empty())
                    return open_types::INTEGER32_t(0);
                return open_types::INTEGER32_t(std::stoll(std::string(str).c_str(), nullptr, 0));
            case DataType_E::INTEGER64:
                if (val.empty())
                    return open_types::INTEGER64_t(0);
                return open_types::INTEGER64_t(std::stoll(std::string(str).c_str(), nullptr, 0));
            case DataType_E::UNSIGNED8:
                if (val.empty())
                    return open_types::UNSIGNED8_t(0);
                return open_types::UNSIGNED8_t(std::stoul(std::string(str).c_str(), nullptr, 0));
            case DataType_E::UNSIGNED16:
                if (val.empty())
                    return open_types::UNSIGNED16_t(0);
                return open_types::UNSIGNED16_t(std::stoul(std::string(str).c_str(), nullptr, 0));
            case DataType_E::UNSIGNED32:
                if (val.empty())
                    return open_types::UNSIGNED32_t(0);
                return open_types::UNSIGNED32_t(std::stoul(std::string(str).c_str(), nullptr, 0));
            case DataType_E::UNSIGNED64:
                if (val.empty())
                    return open_types::UNSIGNED64_t(0);
                return open_types::UNSIGNED64_t(std::stoull(std::string(str).c_str(), nullptr, 0));
            case DataType_E::REAL32:
                if (val.empty())
                    return open_types::REAL32_t(0);
                return open_types::REAL32_t(std::stof(std::string(str).c_str()));
            case DataType_E::REAL64:
                if (val.empty())
                    return {};
                return {};
            case DataType_E::VISIBLE_STRING:
                return std::string(str);
            case DataType_E::UNICODE_STRING:
                return {};
            case DataType_E::OCTET_STRING:
                for (const auto& byte : str)
                {
                    byteResult.push_back(std::byte(byte));
                }
                return byteResult;
            case DataType_E::DOMAIN:
                for (const auto& byte : str)
                {
                    byteResult.push_back(std::byte(byte));
                }
                return byteResult;
        }
        return nullptr;
    }

    std::string EDSEntry::getStringFromVariant(const DataType_E&     dataType,
                                               const ValueVariant_t& val)
    {
        switch (dataType)
        {
            case DataType_E::BOOLEAN:
                return std::to_string(std::get<open_types::BOOLEAN_t>(val));
            case DataType_E::INTEGER8:
                return std::to_string(std::get<open_types::INTEGER8_t>(val));
            case DataType_E::INTEGER16:
                return std::to_string(std::get<open_types::INTEGER16_t>(val));
            case DataType_E::INTEGER32:
                return std::to_string(std::get<open_types::INTEGER32_t>(val));
            case DataType_E::INTEGER64:
                return std::to_string(std::get<open_types::INTEGER64_t>(val));
            case DataType_E::UNSIGNED8:
                return std::to_string(std::get<open_types::UNSIGNED8_t>(val));
            case DataType_E::UNSIGNED16:
                return std::to_string(std::get<open_types::UNSIGNED16_t>(val));
            case DataType_E::UNSIGNED32:
                return std::to_string(std::get<open_types::UNSIGNED32_t>(val));
            case DataType_E::UNSIGNED64:
                return std::to_string(std::get<open_types::UNSIGNED64_t>(val));
            case DataType_E::REAL32:
                return std::to_string(std::get<open_types::REAL32_t>(val));
            case DataType_E::REAL64:
                return {};
            case DataType_E::VISIBLE_STRING:
                return std::get<open_types::VISIBLE_STRING_t>(val);
            case DataType_E::UNICODE_STRING:
                return {};
            case DataType_E::OCTET_STRING:
                return {};  // todo: case on its own can not support it
            case DataType_E::DOMAIN:
                std::vector<std::byte> bytes = std::get<open_types::DOMAIN_t>(val);
                std::string            result;
                for (auto byte : bytes)
                {
                    result += std::to_string(static_cast<int>(byte));
                    result += " ";
                }
                return result;
        }
        return nullptr;
    }
    EDSEntry& EDSEntry::operator[](u8 subIndex)
    {
        if (!m_subObjectsMap.has_value())
        {
            Logger log(Logger::ProgramLayer_E::TOP, "ERR_HANDLR");
            log.error("EDS entry named %s has no subindicies! It can not be accessed this way.",
                      m_edsEntryMetaData.parameterName.c_str());
            throw std::runtime_error("No map in the EDS entry");
        }
        return *(m_subObjectsMap.value()).at(subIndex);
    }
    const EDSEntry& EDSEntry::operator[](u8 subIndex) const
    {
        if (!m_subObjectsMap.has_value())
        {
            Logger log(Logger::ProgramLayer_E::TOP, "ERR_HANDLR");
            log.error("EDS entry named %s has no subindicies! It can not be accessed this way.",
                      m_edsEntryMetaData.parameterName.c_str());
            throw std::runtime_error("No map in the EDS entry");
        }
        return *(m_subObjectsMap.value()).at(subIndex);
    }
    std::map<u8, std::unique_ptr<EDSEntry>>::const_iterator EDSEntry::begin() const
    {
        if (!m_subObjectsMap.has_value())
            throw std::runtime_error("No subindicies in EDS entry");
        return m_subObjectsMap.value().begin();
    }
    std::map<u8, std::unique_ptr<EDSEntry>>::const_iterator EDSEntry::end() const
    {
        if (!m_subObjectsMap.has_value())
            throw std::runtime_error("No subindicies in EDS entry");
        return m_subObjectsMap.value().end();
    }
    std::map<u8, std::unique_ptr<EDSEntry>>::iterator EDSEntry::begin()
    {
        if (!m_subObjectsMap.has_value())
            throw std::runtime_error("No subindicies in EDS entry");
        return m_subObjectsMap.value().begin();
    }
    std::map<u8, std::unique_ptr<EDSEntry>>::iterator EDSEntry::end()
    {
        if (!m_subObjectsMap.has_value())
            throw std::runtime_error("No subindicies in EDS entry");
        return m_subObjectsMap.value().end();
    }
    EDSEntry& EDSObjectDictionary::operator[](u16 idx)
    {
        return m_map.at(idx);
    }
    std::map<u16, EDSEntry>::iterator EDSObjectDictionary::begin()
    {
        return m_map.begin();
    }
    std::map<u16, EDSEntry>::iterator EDSObjectDictionary::end()
    {
        return m_map.end();
    }
    std::map<u16, EDSEntry>::const_iterator EDSObjectDictionary::begin() const
    {
        return m_map.begin();
    }
    std::map<u16, EDSEntry>::const_iterator EDSObjectDictionary::end() const
    {
        return m_map.end();
    }

    std::optional<std::pair<u16, std::optional<u8>>> EDSObjectDictionary::getAdressByName(
        std::string_view name) const noexcept
    {
        for (auto& entryPair : m_map)
        {
            if (name == entryPair.second.getEntryMetaData().parameterName)
                return std::make_pair(entryPair.first, std::nullopt);
            if (entryPair.second.getContainerMetaData().has_value())
            {
                for (auto& subentryPair : entryPair.second)
                {
                    if (name == subentryPair.second->getEntryMetaData().parameterName)
                        return std::make_pair(entryPair.first, subentryPair.first);
                }
            }
        }
        return std::nullopt;
    }
    size_t EDSObjectDictionary::size() const
    {
        return m_map.size();
    }
}  // namespace mab
