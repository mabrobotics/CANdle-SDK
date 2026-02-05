#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <map>
#include <memory>
#include <utility>
#include <span>
#include "edsEntry.hpp"

using namespace mab;
using namespace open_types;

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
    EDSEntry::EDSEntryMetaData    floatMetaData{.parameterName = std::string("Float"),
                                                .objectType    = EDSEntry::ObjectType_E::VALUE,
                                                .storageLocation = EDSEntry::StorageLocation_E::RAM};
    EDSEntryVal::EDSValueMetaData floatValueMetaData{
        .dataType        = EDSEntryVal::DataType_E::REAL32,
        .accessType      = EDSEntryVal::AccessRights_E::READ_WRITE,
        .PDOMapping      = true,
        .defaultValueStr = "1.0f"};

    EDSEntry::EDSEntryMetaData    uintMetaData{.parameterName   = std::string("UINT"),
                                               .objectType      = EDSEntry::ObjectType_E::VALUE,
                                               .storageLocation = EDSEntry::StorageLocation_E::RAM};
    EDSEntryVal::EDSValueMetaData uintValueMetaData{
        .dataType        = EDSEntryVal::DataType_E::UNSIGNED16,
        .accessType      = EDSEntryVal::AccessRights_E::READ_WRITE,
        .PDOMapping      = true,
        .defaultValueStr = "12"};

    EDSEntry::EDSEntryMetaData    stringMetaData{.parameterName = std::string("String"),
                                                 .objectType    = EDSEntry::ObjectType_E::VALUE,
                                                 .storageLocation = EDSEntry::StorageLocation_E::RAM};
    EDSEntryVal::EDSValueMetaData stringValueMetaData{
        .dataType        = EDSEntryVal::DataType_E::VISIBLE_STRING,
        .accessType      = EDSEntryVal::AccessRights_E::READ_WRITE,
        .PDOMapping      = true,
        .defaultValueStr = "Some text"};

    EDSEntryVal floatEntry(std::move(floatMetaData), std::move(floatValueMetaData));
    EDSEntryVal uintEntry(std::move(uintMetaData), std::move(uintValueMetaData));
    EDSEntryVal stringEntry(std::move(stringMetaData), std::move(stringValueMetaData));

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
    EDSEntry::EDSEntryMetaData    floatMetaData{.parameterName = std::string("Float"),
                                                .objectType    = EDSEntry::ObjectType_E::VALUE,
                                                .storageLocation = EDSEntry::StorageLocation_E::RAM};
    EDSEntryVal::EDSValueMetaData floatValueMetaData{
        .dataType        = EDSEntryVal::DataType_E::REAL32,
        .accessType      = EDSEntryVal::AccessRights_E::READ_WRITE,
        .PDOMapping      = true,
        .defaultValueStr = "1.0f"};

    EDSEntry::EDSEntryMetaData    uintMetaData{.parameterName   = std::string("UINT"),
                                               .objectType      = EDSEntry::ObjectType_E::VALUE,
                                               .storageLocation = EDSEntry::StorageLocation_E::RAM};
    EDSEntryVal::EDSValueMetaData uintValueMetaData{
        .dataType        = EDSEntryVal::DataType_E::UNSIGNED16,
        .accessType      = EDSEntryVal::AccessRights_E::READ_WRITE,
        .PDOMapping      = true,
        .defaultValueStr = "12"};

    EDSEntry::EDSEntryMetaData    stringMetaData{.parameterName = std::string("String"),
                                                 .objectType    = EDSEntry::ObjectType_E::VALUE,
                                                 .storageLocation = EDSEntry::StorageLocation_E::RAM};
    EDSEntryVal::EDSValueMetaData stringValueMetaData{
        .dataType        = EDSEntryVal::DataType_E::VISIBLE_STRING,
        .accessType      = EDSEntryVal::AccessRights_E::READ_WRITE,
        .PDOMapping      = true,
        .defaultValueStr = "Some text"};

    EDSEntry::EDSEntryMetaData containerMetaData = {
        .parameterName   = std::string("Container"),
        .objectType      = EDSEntry::ObjectType_E::RECORD,
        .storageLocation = EDSEntry::StorageLocation_E::RAM};
    EDSEntryContainer::EDSContainerMetaData containerSpecificMetaData = {.numberOfSubindices = 3};

    std::map<u8, std::unique_ptr<EDSEntryVal>> conteinerMap;
    conteinerMap.emplace(
        0, std::make_unique<EDSEntryVal>(std::move(floatMetaData), std::move(floatValueMetaData)));
    conteinerMap.emplace(
        1, std::make_unique<EDSEntryVal>(std::move(uintMetaData), std::move(uintValueMetaData)));
    conteinerMap.emplace(
        2,
        std::make_unique<EDSEntryVal>(std::move(stringMetaData), std::move(stringValueMetaData)));

    EDSEntryContainer containerEntry(std::move(containerMetaData),
                                     std::move(containerSpecificMetaData),
                                     std::move(conteinerMap));

    std::map<u32, EDSEntryContainer> ODMap;
    ODMap.emplace(0x1000, std::move(containerEntry));

    EDSObjectDictionary od(std::move(ODMap));

    EXPECT_FLOAT_EQ(od[0x1000][0x0], 1.0f);
    EXPECT_EQ((UNSIGNED16_t)od[0x1000][0x1], 12);
    EXPECT_STREQ(((std::string)od[0x1000][0x2]).c_str(), "Some text");
}

TEST_F(EdsEntry_test, serialization)
{
    EDSEntry::EDSEntryMetaData    floatMetaData{.parameterName = std::string("Float"),
                                                .objectType    = EDSEntry::ObjectType_E::VALUE,
                                                .storageLocation = EDSEntry::StorageLocation_E::RAM};
    EDSEntryVal::EDSValueMetaData floatValueMetaData{
        .dataType        = EDSEntryVal::DataType_E::REAL32,
        .accessType      = EDSEntryVal::AccessRights_E::READ_WRITE,
        .PDOMapping      = true,
        .defaultValueStr = "1.0f"};

    EDSEntryVal floatEntry(std::move(floatMetaData), std::move(floatValueMetaData));

    floatEntry      = 3.14f;
    auto serialized = floatEntry.getSerializedValue();
    EXPECT_EQ(serialized.size(), sizeof(REAL32_t));
    floatEntry.setSerializedValue(std::span<std::byte>(serialized.data(), serialized.size()));
    EXPECT_FLOAT_EQ(floatEntry, 3.14f);
}

TEST_F(EdsEntry_test, stringSerialization)
{
    EDSEntry::EDSEntryMetaData    stringMetaData{.parameterName = std::string("String"),
                                                 .objectType    = EDSEntry::ObjectType_E::VALUE,
                                                 .storageLocation = EDSEntry::StorageLocation_E::RAM};
    EDSEntryVal::EDSValueMetaData stringValueMetaData{
        .dataType        = EDSEntryVal::DataType_E::VISIBLE_STRING,
        .accessType      = EDSEntryVal::AccessRights_E::READ_WRITE,
        .PDOMapping      = true,
        .defaultValueStr = "Some text"};

    EDSEntryVal stringEntry(std::move(stringMetaData), std::move(stringValueMetaData));

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
