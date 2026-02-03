#include <cstddef>
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

    EDSEntryVal::EDSEntryVal(EDSEntryMetaData&& edsEntryMetaData,
                             EDSValueMetaData&& edsValueMetaData)
        : EDSEntry(std::move(edsEntryMetaData)), m_edsValueMetaData(edsValueMetaData)
    {
        if (m_edsEntryMetaData.objectType != ObjectType_E::VALUE)
        {
            throw std::runtime_error("Array or Record parsed incorrectly!");
        }
        m_value =
            getVariantFromString(m_edsValueMetaData.dataType, m_edsValueMetaData.defaultValueStr);
        if (std::holds_alternative<std::monostate>(m_value))
        {
            throw std::runtime_error("Invalid default value or type!");
        }
    }
    std::vector<std::byte> EDSEntryVal::getSerializedValue() const noexcept
    {
        return {};
    }
    EDSEntry::Error_t EDSEntryVal::setSerializedValue(const std::span<std::byte> bytes)
    {
        return Error_t::OK;
    }

    std::string EDSEntryVal::getAsString() const noexcept
    {
        return getStringFromVariant(m_edsValueMetaData.dataType, m_value);
    }
    EDSEntry::Error_t EDSEntryVal::setFromString(const std::string_view str)
    {
        m_value = getVariantFromString(m_edsValueMetaData.dataType, str);
        if (std::holds_alternative<std::monostate>(m_value))
        {
            return Error_t::PARSING_FAILED;
        }
        return Error_t::OK;
    }

    EDSEntryVal::ValueVariant_t EDSEntryVal::getVariantFromString(const DataType_E&      dataType,
                                                                  const std::string_view str)
    {
        switch (dataType)
        {
            case DataType_E::BOOLEAN:
                return str == "TRUE" ? open_types::BOOLEAN_t(true) : open_types::BOOLEAN_t(false);
            case DataType_E::INTEGER8:
                return open_types::INTEGER8_t(std::stoi(str.data()));
            case DataType_E::INTEGER16:
                return open_types::INTEGER16_t(std::stoi(str.data()));
            case DataType_E::INTEGER32:
                return open_types::INTEGER32_t(std::stoi(str.data()));
            case DataType_E::INTEGER64:
                return open_types::INTEGER64_t(std::stoll(str.data()));
            case DataType_E::UNSIGNED8:
                return open_types::UNSIGNED8_t(std::stoi(str.data()));
            case DataType_E::UNSIGNED16:
                return open_types::UNSIGNED16_t(std::stoi(str.data()));
            case DataType_E::UNSIGNED32:
                return open_types::UNSIGNED32_t(std::stoul(str.data()));
            case DataType_E::UNSIGNED64:
                return open_types::UNSIGNED64_t(std::stoull(str.data()));
            case DataType_E::REAL32:
                return open_types::REAL32_t(std::stof(str.data()));
            case DataType_E::REAL64:
                return {};
            case DataType_E::VISIBLE_STRING:
                return std::string(str);
            case DataType_E::UNICODE_STRING:
                return {};
            case DataType_E::OCTET_STRING:
                return {};
            case DataType_E::DOMAIN:
                std::vector<std::byte> result;
                for (const auto& byte : str)
                {
                    result.push_back(std::byte(byte));
                }
                return result;
        }
        return nullptr;
    }

    std::string EDSEntryVal::getStringFromVariant(const DataType_E&     dataType,
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
                return nullptr;
            case DataType_E::VISIBLE_STRING:
                return std::get<open_types::VISIBLE_STRING_t>(val);
            case DataType_E::UNICODE_STRING:
                return nullptr;
            case DataType_E::OCTET_STRING:
                return nullptr;
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
}  // namespace mab
