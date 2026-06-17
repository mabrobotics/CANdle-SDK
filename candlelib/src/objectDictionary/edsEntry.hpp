// This file contains EDS entry interface as well as all of its implemetations. This class serves to
// represent .eds file entry for each of object types (value, array and record) in a form that is
// easy to iterate through, enables safe access it's acctual value, and holds onto neccesary
// meta-data about the object itself.
#pragma once
#include <cstddef>
#include <iterator>
#include <span>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <variant>
#include <vector>
#include <map>
#include "logger.hpp"
#include "mab_types.hpp"

namespace mab
{
    /// @namespace canopen_types
    /// @brief this namespace shoudl be used along with EDSEntryVal class for type compatibility
    /// with the variants
    namespace canopen_types
    {
        using BOOLEAN_t        = u64;
        using INTEGER8_t       = i64;
        using INTEGER16_t      = i64;
        using INTEGER32_t      = i64;
        using INTEGER64_t      = i64;
        using UNSIGNED8_t      = u64;
        using UNSIGNED16_t     = u64;
        using UNSIGNED32_t     = u64;
        using UNSIGNED64_t     = u64;
        using REAL32_t         = f32;
        using VISIBLE_STRING_t = std::string;
        using OCTET_STRING_t   = std::vector<std::byte>;
        using UNICODE_STRING_t = std::vector<std::byte>;
        using DOMAIN_t         = std::vector<std::byte>;
    }  // namespace canopen_types

    /// @class EDSEntry
    /// @brief Base class representing an EDS entry
    class EDSEntry
    {
      public:
      public:
        using ValueVariant_t =
            std::variant<i64, u64, f32, std::string, std::vector<std::byte>, std::monostate>;
        enum class StorageLocation_E
        {
            RAM,
            PERSIST_COMM
        };
        enum class ObjectType_E : u8
        {
            VALUE  = 0x7,
            ARRAY  = 0x8,
            RECORD = 0x9
        };

        enum class Error_t
        {
            UNKNOWN,
            OK,
            PARSING_FAILED,
            INCORRECT_USE
        };

        enum class AccessRights_E
        {
            READ_ONLY  = 0x00,
            READ_WRITE = 0x01
        };

        enum class DataType_E
        {
            BOOLEAN        = 0x0001,
            INTEGER8       = 0x0002,
            INTEGER16      = 0x0003,
            INTEGER32      = 0x0004,
            INTEGER64      = 0x0015,
            UNSIGNED8      = 0x0005,
            UNSIGNED16     = 0x0006,
            UNSIGNED32     = 0x0007,
            UNSIGNED64     = 0x001B,
            REAL32         = 0x0008,
            REAL64         = 0x0011,
            VISIBLE_STRING = 0x0009,
            OCTET_STRING   = 0x000A,
            UNICODE_STRING = 0x000B,
            DOMAIN_TYPE    = 0x000F
        };

        struct EDSValueMetaData
        {
            DataType_E     dataType;
            AccessRights_E accessType;
            bool           PDOMapping;
            std::string    defaultValueStr;
        };

        struct EDSContainerMetaData
        {
            u8 numberOfSubindices;
        };

        struct EDSEntryMetaData
        {
            std::pair<u16, std::optional<u8>>   address;
            std::string                         parameterName;
            ObjectType_E                        objectType;
            StorageLocation_E                   storageLocation;
            std::optional<EDSValueMetaData>     edsValueMeta;
            std::optional<EDSContainerMetaData> edsContainerMeta;
        };

        EDSEntry(EDSEntryMetaData&& edsEntryMetaData);
        EDSEntry(EDSEntryMetaData&&                        edsEntryMetaData,
                 std::map<u8, std::unique_ptr<EDSEntry>>&& subObjectsMap);

        template <class T>
        operator T() const
        {
            T result;
            if (!m_value.has_value())
            {
                Logger log(Logger::ProgramLayer_E::TOP, "ERR_HANDLR");
                log.error("EDS entry named %s has no value! It can not be accessed this way.",
                          m_edsEntryMetaData.parameterName.c_str());
                return {};
            }
            try
            {
                return std::get<T>(m_value.value());
            }
            catch (std::bad_variant_access& e)
            {
                Logger            log(Logger::ProgramLayer_E::TOP, "ERR_HANDLR");
                std::stringstream ss;
                ValueVariant_t    testVar = T();
                ss << "The member variant type index was: " << m_value.value().index();
                ss << "; and the input variant index was: " << testVar.index();
                log.error("Bad casting from CANopen entry named %s",
                          m_edsEntryMetaData.parameterName.c_str());
                log.error("%s", ss.str().c_str());
                log.error("%s", e.what());
                throw e;
            }
            return result;
        }

        template <class T>
        T operator=(const T& externalValue)
        {
            if (!m_value.has_value())
            {
                Logger log(Logger::ProgramLayer_E::TOP, "ERR_HANDLR");
                log.error("EDS entry named %s has no value! It can not be accessed this way.",
                          m_edsEntryMetaData.parameterName.c_str());
                return {};
            }
            if (std::holds_alternative<T>(m_value.value()))
            {
                m_value = externalValue;
            }
            else
            {
                Logger            log(Logger::ProgramLayer_E::TOP, "ERR_HANDLR");
                std::stringstream ss;
                ValueVariant_t    testVar = T();
                ss << "The member variant type index was: " << m_value.value().index();
                ss << "; and the input variant index was: " << testVar.index();
                log.error("Bad casting to CANopen entry named %s",
                          m_edsEntryMetaData.parameterName.c_str());
                log.error("%s", ss.str().c_str());
            }
            return externalValue;
        }

        EDSEntry& operator[](u8 subIndex);

        const EDSEntry& operator[](u8 subIndex) const;

        std::map<u8, std::unique_ptr<EDSEntry>>::const_iterator begin() const;
        std::map<u8, std::unique_ptr<EDSEntry>>::const_iterator end() const;
        std::map<u8, std::unique_ptr<EDSEntry>>::iterator       begin();
        std::map<u8, std::unique_ptr<EDSEntry>>::iterator       end();

        /// @brief get EDS entry general parameters
        /// @return EDSEntryMetaData for this entry
        const EDSEntryMetaData& getEntryMetaData() const noexcept;

        /// @brief Get EDS value specific parameters
        /// @return EDSValueMetaData for this entry
        const std::optional<EDSValueMetaData> getValueMetaData() const noexcept;

        /// @brief Get EDS container specific parameters
        /// @return EDSContainerMetaData for this entry
        const std::optional<EDSContainerMetaData> getContainerMetaData() const noexcept;

        std::vector<std::byte> getSerializedValue() const noexcept;
        Error_t                setSerializedValue(const std::span<std::byte> bytes);

        std::string getAsString() const noexcept;
        Error_t     setFromString(const std::string_view str);

        size_t valueSize() const noexcept;

      private:
        static ValueVariant_t getVariantFromString(const DataType_E&      dataType,
                                                   const std::string_view str);

        static std::string getStringFromVariant(const DataType_E&     dataType,
                                                const ValueVariant_t& val);

        EDSEntryMetaData                                       m_edsEntryMetaData;
        std::optional<ValueVariant_t>                          m_value;
        std::optional<std::map<u8, std::unique_ptr<EDSEntry>>> m_subObjectsMap;
    };

    class EDSObjectDictionary
    {
      public:
        EDSObjectDictionary(std::map<u16, EDSEntry>&& map) : m_map(std::move(map))
        {
        }

        EDSEntry& operator[](u16 idx);

        std::map<u16, EDSEntry>::iterator       begin();
        std::map<u16, EDSEntry>::iterator       end();
        std::map<u16, EDSEntry>::const_iterator begin() const;
        std::map<u16, EDSEntry>::const_iterator end() const;

        size_t size() const;

        std::optional<std::pair<u16, std::optional<u8>>> getAdressByName(
            std::string_view name) noexcept;
        std::optional<std::reference_wrapper<EDSEntry>> getEntryByName(
            std::string_view name) noexcept;

      private:
        std::map<u16, EDSEntry> m_map;
    };

}  // namespace mab
