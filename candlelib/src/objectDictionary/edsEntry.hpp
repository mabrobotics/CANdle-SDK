
#include <string>
#include <string_view>
#include <span>
#include <type_traits>
#include <vector>
#include <any>
#include "edsParser.hpp"
#include "mab_types.hpp"

namespace mab
{

    class I_EDSEntry
    {
      public:
        virtual ~I_EDSEntry() = default;
        enum class StorageLocation_E
        {
            RAM,
            PERSIST_COMM
        };
        enum class accessRights_E
        {
            READ_ONLY  = 0x00,
            READ_WRITE = 0x01,
            WRITE_ONLY = 0x02,
        };
        enum class SectionType_E
        {
            MANDATORY    = 0,
            OPTIONAL     = 1,
            MANUFACTURER = 2,
        };

        enum class ObjectType_E : u8
        {
            VALUE  = 0x7,
            ARRAY  = 0x8,
            RECORD = 0x9
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

        enum class Error_t
        {
            UNKNOWN,
            OK,
            PARSING_FAILED,
            BAD_ANY_CAST
        };

        struct EDSEntryMetaData
        {
            std::string       parameterName;
            ObjectType_E      objectType;
            StorageLocation_E storageLocation;
            DataType_E        dataType;
            accessRights_E    accessType;
            bool              PDOMapping;
            SectionType_E     sectionType;
        };

        virtual std::vector<std::byte> getSerializedValue() const                     = 0;
        virtual Error_t                setSerializedValue(std::span<const std::byte>) = 0;
        virtual std::string            toString() const                               = 0;
        virtual Error_t                fromString(const std::string_view)             = 0;
        virtual std::any               get() const                                    = 0;
        virtual Error_t                set(std::any val)                              = 0;

        virtual EDSEntryMetaData getBasicData() const = 0;
    };

    template <class T>
    class EDSEntryValue : public I_EDSEntry
    {
      public:
        const I_EDSEntry::EDSEntryMetaData m_metaData;

        T m_value;

        constexpr EDSEntryValue(const I_EDSEntry::EDSEntryMetaData& metaData) : m_metaData(metaData)
        {
            static_assert(std::is_nothrow_copy_constructible_v<T>, "Type is not copyable");
            static_assert(std::is_standard_layout_v<T>, "Type is not standard layout");
        }
        std::vector<std::byte> getSerializedValue() const override;
        Error_t                setSerializedValue(std::span<const std::byte> value) override;
        std::string            toString() const override;
        Error_t                fromString(const std::string_view value) override;
        EDSEntryMetaData       getBasicData() const override;

        std::any get() const override;
        Error_t  set(std::any) override;
    };

    template <class T>
    class EDSEntryArray : public I_EDSEntry
    {
      public:
        const EDSEntryMetaData m_metaData;

        std::vector<EDSEntryValue<T>> m_entries;

        constexpr EDSEntryArray(const EDSEntryMetaData& metaData) : m_metaData(metaData)
        {
            constexpr bool isSafe = std::is_standard_layout_v<T> & std::is_trivially_copyable_v<T>;
            static_assert(isSafe, "Type is not safe for array serialization");
        }
        std::vector<std::byte> getSerializedValue() const override;
        Error_t                setSerializedValue(std::span<const std::byte> value) override;
        std::string            toString() const override;
        Error_t                fromString(const std::string_view value) override;
        EDSEntryMetaData       getBasicData() const override;
    };

    class EDSEntryRecord : public I_EDSEntry
    {
      public:
        const EDSEntryMetaData m_metaData;

        std::vector<I_EDSEntry> m_entries;

        constexpr EDSEntryRecord(const EDSEntryMetaData& metaData) : m_metaData(metaData)
        {
        }
        std::vector<std::byte> getSerializedValue() const override;
        Error_t                setSerializedValue(std::span<const std::byte> value) override;
        std::string            toString() const override;
        Error_t                fromString(const std::string_view value) override;
        EDSEntryMetaData       getBasicData() const override;
    };
}  // namespace mab
