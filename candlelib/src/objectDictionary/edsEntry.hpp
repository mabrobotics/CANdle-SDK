// This file contains EDS entry interface as well as all of its implemetations. This class serves to
// represent .eds file entry for each of object types (value, array and record) in a form that is
// easy to iterate through, enables safe access it's acctual value, and holds onto neccesary
// meta-data about the object itself.

#include <cstddef>
#include <functional>
#include <iterator>
#include <span>
#include <sstream>
#include <string>
#include <string_view>
#include <type_traits>
#include <variant>
#include <vector>
#include <map>
#include "logger.hpp"
#include "mab_types.hpp"

namespace mab
{
    namespace open_types
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
        using DOMAIN_t         = std::vector<std::byte>;
    }  // namespace open_types

    class EDSEntry
    {
      public:
        using ValueVariant_t =
            std::variant<i64, u64, f32, std::string, std::vector<std::byte>, std::nullptr_t>;
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
            NO_VALUE
        };

        struct EDSEntryMetaData
        {
            std::string       parameterName;
            ObjectType_E      objectType;
            StorageLocation_E storageLocation;
        };

        EDSEntry(EDSEntryMetaData&& edsEntryMetaData) : m_edsEntryMetaData(edsEntryMetaData)
        {
        }

        const EDSEntryMetaData& getEntryMetaData() const noexcept;

      protected:
        EDSEntryMetaData m_edsEntryMetaData;
    };

    class EDSEntryVal final : public EDSEntry
    {
      public:
        enum class AccessRights_E
        {
            READ_ONLY  = 0x00,
            READ_WRITE = 0x01,
            WRITE_ONLY = 0x02,
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
            DOMAIN         = 0x000F
        };

        struct EDSValueMetaData
        {
            DataType_E     dataType;
            AccessRights_E accessType;
            bool           PDOMapping;
            std::string    defaultValueStr;
        };
        EDSEntryVal(EDSEntryMetaData&& edsEntryMetaData, EDSValueMetaData&& edsValueMetaData);

        template <class T>
        operator T() const
        {
            T result;
            try
            {
                return std::get<T>(m_value);
            }
            catch (std::bad_variant_access& e)
            {
                Logger            log(Logger::ProgramLayer_E::TOP, "ERR_HANDLR");
                std::stringstream ss;
                ValueVariant_t    testVar = T();
                ss << "The member variant type index was: " << m_value.index();
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
            if (std::holds_alternative<T>(m_value))
            {
                m_value = externalValue;
            }
            else
            {
                Logger            log(Logger::ProgramLayer_E::TOP, "ERR_HANDLR");
                std::stringstream ss;
                ValueVariant_t    testVar = T();
                ss << "The member variant type index was: " << m_value.index();
                ss << "; and the input variant index was: " << testVar.index();
                log.error("Bad casting to CANopen entry named %s",
                          m_edsEntryMetaData.parameterName.c_str());
                log.error("%s", ss.str().c_str());
            }
            return externalValue;
        }

        std::vector<std::byte> getSerializedValue() const noexcept;
        Error_t                setSerializedValue(const std::span<std::byte> bytes);

        std::string getAsString() const noexcept;
        Error_t     setFromString(const std::string_view str);

      private:
        static ValueVariant_t getVariantFromString(const DataType_E&      dataType,
                                                   const std::string_view str);

        static std::string getStringFromVariant(const DataType_E&     dataType,
                                                const ValueVariant_t& val);
        EDSValueMetaData   m_edsValueMetaData;
        ValueVariant_t     m_value;
    };

    class EDSEntryContainer final : public EDSEntry
    {
      public:
        struct EDSContainerMetaData
        {
            u8 numberOfSubindecies;
        };

        EDSEntryContainer(EDSEntryMetaData&&                           edsEntryMetaData,
                          EDSContainerMetaData&&                       edsContainerMetadata,
                          std::map<u8, std::unique_ptr<EDSEntryVal>>&& subObjectsMap)
            : EDSEntry(std::move(edsEntryMetaData)),
              m_edsContainerMetaData(std::move(edsContainerMetadata)),
              m_subObjectsMap(std::move(subObjectsMap))
        {
        }

        inline EDSEntryVal& operator[](u8 subIndex)
        {
            return *m_subObjectsMap.at(subIndex);
        }

        const inline EDSContainerMetaData getContainerMetaData() const noexcept
        {
            return m_edsContainerMetaData;
        }

      private:
        EDSContainerMetaData                       m_edsContainerMetaData;
        std::map<u8, std::unique_ptr<EDSEntryVal>> m_subObjectsMap;
    };

    class EDSObjectDictionary
    {
      public:
        EDSObjectDictionary(std::map<u32, EDSEntryContainer>&& map) : m_map(std::move(map))
        {
        }

        EDSEntryContainer& operator[](u32 idx)
        {
            return m_map.at(idx);
        }

      private:
        std::map<u32, EDSEntryContainer> m_map;
    };

}  // namespace mab
