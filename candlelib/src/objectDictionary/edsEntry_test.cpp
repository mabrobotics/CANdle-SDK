#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <span>
#include "edsEntry.hpp"

using namespace mab;
using namespace canopen_types;

class EdsEntry_test : public ::testing::Test
{
  protected:
    void SetUp() override
    {
    }
    void TearDown() override
    {
    }
};

TEST_F(EdsEntry_test, simpleValues)
{
    EDSEntry::EDSValueMetaData floatValueMetaData{
        .dataType        = EDSEntry::DataType_E::REAL32,
        .accessType      = EDSEntry::AccessRights_E::READ_WRITE,
        .PDOMapping      = true,
        .defaultValueStr = "1.0f"};
    EDSEntry::EDSEntryMetaData floatMetaData{.parameterName   = std::string("Float"),
                                             .objectType      = EDSEntry::ObjectType_E::VALUE,
                                             .storageLocation = EDSEntry::StorageLocation_E::RAM,
                                             .edsValueMeta    = floatValueMetaData};

    EDSEntry::EDSValueMetaData uintValueMetaData{.dataType   = EDSEntry::DataType_E::UNSIGNED16,
                                                 .accessType = EDSEntry::AccessRights_E::READ_WRITE,
                                                 .PDOMapping = true,
                                                 .defaultValueStr = "12"};
    EDSEntry::EDSEntryMetaData uintMetaData{.parameterName   = std::string("UINT"),
                                            .objectType      = EDSEntry::ObjectType_E::VALUE,
                                            .storageLocation = EDSEntry::StorageLocation_E::RAM,
                                            .edsValueMeta    = uintValueMetaData};

    EDSEntry::EDSValueMetaData stringValueMetaData{
        .dataType        = EDSEntry::DataType_E::VISIBLE_STRING,
        .accessType      = EDSEntry::AccessRights_E::READ_WRITE,
        .PDOMapping      = true,
        .defaultValueStr = "Some text"};

    EDSEntry::EDSEntryMetaData stringMetaData{.parameterName   = std::string("String"),
                                              .objectType      = EDSEntry::ObjectType_E::VALUE,
                                              .storageLocation = EDSEntry::StorageLocation_E::RAM,
                                              .edsValueMeta    = stringValueMetaData};

    EDSEntry floatEntry(std::move(floatMetaData));
    EDSEntry uintEntry(std::move(uintMetaData));
    EDSEntry stringEntry(std::move(stringMetaData));

    REAL32_t         floatValue = floatEntry;
    UNSIGNED16_t     uintValue  = uintEntry;
    VISIBLE_STRING_t stringVal  = stringEntry;
    floatEntry                  = floatValue;
    uintEntry                   = uintValue;
    stringEntry                 = stringVal;
    EXPECT_FLOAT_EQ(floatEntry, 1.0f);
    EXPECT_EQ((UNSIGNED16_t)uintEntry, uintValue);
    EXPECT_STREQ(((std::string)stringEntry).c_str(), stringVal.c_str());
}

TEST_F(EdsEntry_test, containerLayout)
{
    EDSEntry::EDSValueMetaData floatValueMetaData{
        .dataType        = EDSEntry::DataType_E::REAL32,
        .accessType      = EDSEntry::AccessRights_E::READ_WRITE,
        .PDOMapping      = true,
        .defaultValueStr = "1.0f"};
    EDSEntry::EDSEntryMetaData floatMetaData{.parameterName   = std::string("Float"),
                                             .objectType      = EDSEntry::ObjectType_E::VALUE,
                                             .storageLocation = EDSEntry::StorageLocation_E::RAM,
                                             .edsValueMeta    = floatValueMetaData};

    EDSEntry::EDSValueMetaData uintValueMetaData{.dataType   = EDSEntry::DataType_E::UNSIGNED16,
                                                 .accessType = EDSEntry::AccessRights_E::READ_WRITE,
                                                 .PDOMapping = true,
                                                 .defaultValueStr = "12"};
    EDSEntry::EDSEntryMetaData uintMetaData{.parameterName   = std::string("UINT"),
                                            .objectType      = EDSEntry::ObjectType_E::VALUE,
                                            .storageLocation = EDSEntry::StorageLocation_E::RAM,
                                            .edsValueMeta    = uintValueMetaData};

    EDSEntry::EDSValueMetaData stringValueMetaData{
        .dataType        = EDSEntry::DataType_E::VISIBLE_STRING,
        .accessType      = EDSEntry::AccessRights_E::READ_WRITE,
        .PDOMapping      = true,
        .defaultValueStr = "Some text"};

    EDSEntry::EDSEntryMetaData stringMetaData{.parameterName   = std::string("String"),
                                              .objectType      = EDSEntry::ObjectType_E::VALUE,
                                              .storageLocation = EDSEntry::StorageLocation_E::RAM,
                                              .edsValueMeta    = stringValueMetaData};

    EDSEntry::EDSContainerMetaData containerSpecificMetaData = {.numberOfSubindices = 3};

    EDSEntry::EDSEntryMetaData containerMetaData{
        .parameterName    = std::string("Conatiner"),
        .objectType       = EDSEntry::ObjectType_E::RECORD,
        .storageLocation  = EDSEntry::StorageLocation_E::RAM,
        .edsContainerMeta = containerSpecificMetaData};

    std::map<u8, std::unique_ptr<EDSEntry>> containerMap;
    containerMap.emplace(0, std::make_unique<EDSEntry>(std::move(floatMetaData)));
    containerMap.emplace(1, std::make_unique<EDSEntry>(std::move(uintMetaData)));
    containerMap.emplace(2, std::make_unique<EDSEntry>(std::move(stringMetaData)));

    EDSEntry containerEntry(std::move(containerMetaData), std::move(containerMap));

    std::map<u16, EDSEntry> ODMap;
    ODMap.emplace(0x1000, std::move(containerEntry));

    EDSObjectDictionary od(std::move(ODMap));

    EXPECT_FLOAT_EQ(od[0x1000][0x0], 1.0f);
    EXPECT_EQ((UNSIGNED16_t)od[0x1000][0x1], 12);
    EXPECT_STREQ(((std::string)od[0x1000][0x2]).c_str(), "Some text");
}

TEST_F(EdsEntry_test, serialization)
{
    EDSEntry::EDSValueMetaData floatValueMetaData{
        .dataType        = EDSEntry::DataType_E::REAL32,
        .accessType      = EDSEntry::AccessRights_E::READ_WRITE,
        .PDOMapping      = true,
        .defaultValueStr = "1.0f"};
    EDSEntry::EDSEntryMetaData floatMetaData{.parameterName   = std::string("Float"),
                                             .objectType      = EDSEntry::ObjectType_E::VALUE,
                                             .storageLocation = EDSEntry::StorageLocation_E::RAM,
                                             .edsValueMeta    = floatValueMetaData};

    EDSEntry floatEntry(std::move(floatMetaData));

    floatEntry      = 3.14f;
    auto serialized = floatEntry.getSerializedValue();
    EXPECT_EQ(serialized.size(), sizeof(REAL32_t));
    floatEntry.setSerializedValue(std::span<std::byte>(serialized.data(), serialized.size()));
    EXPECT_FLOAT_EQ(floatEntry, 3.14f);
}

TEST_F(EdsEntry_test, stringSerialization)
{
    EDSEntry::EDSValueMetaData stringValueMetaData{
        .dataType        = EDSEntry::DataType_E::VISIBLE_STRING,
        .accessType      = EDSEntry::AccessRights_E::READ_WRITE,
        .PDOMapping      = true,
        .defaultValueStr = "Some text"};

    EDSEntry::EDSEntryMetaData stringMetaData{.parameterName   = std::string("String"),
                                              .objectType      = EDSEntry::ObjectType_E::VALUE,
                                              .storageLocation = EDSEntry::StorageLocation_E::RAM,
                                              .edsValueMeta    = stringValueMetaData};

    EDSEntry stringEntry(std::move(stringMetaData));

    auto readBytes = stringEntry.getSerializedValue();
    EXPECT_EQ(readBytes.size(), 9);  // "Some text" length
    for (size_t i = 0; i < readBytes.size(); i++)
    {
        EXPECT_EQ(static_cast<char>(readBytes[i]), "Some text"[i]);
    }

    auto spanBytes   = std::as_bytes(std::span<const char>("Hello World", 11));
    auto stringBytes = std::vector<std::byte>(spanBytes.begin(), spanBytes.end());
    stringEntry.setSerializedValue(stringBytes);
    EXPECT_STREQ(stringEntry.getAsString().c_str(), "Hello World");
}
