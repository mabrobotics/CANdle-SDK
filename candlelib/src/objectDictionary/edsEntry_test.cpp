#include <gtest/gtest.h>
#include <gmock/gmock.h>
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
    EDSEntry::EDSEntryMetaData floatMetaData = {
        .parameterName   = std::string("Float"),
        .objectType      = EDSEntry::ObjectType_E::VALUE,
        .storageLocation = EDSEntry::StorageLocation_E::RAM};
    EDSEntryVal::EDSValueMetaData floatValueMetaData{
        .dataType        = EDSEntryVal::DataType_E::REAL32,
        .accessType      = EDSEntryVal::AccessRights_E::READ_WRITE,
        .PDOMapping      = true,
        .defaultValueStr = "1.0f"};

    // EDSEntry::EDSEntryMetaData    uintMetaData = {.parameterName = std::string("UINT"),
    //                                               .objectType    = EDSEntry::ObjectType_E::VALUE,
    //                                               .storageLocation =
    //                                               EDSEntry::StorageLocation_E::RAM};
    // EDSEntryVal::EDSValueMetaData uintValueMetaData{
    //     .dataType        = EDSEntryVal::DataType_E::UNSIGNED16,
    //     .accessType      = EDSEntryVal::AccessRights_E::READ_WRITE,
    //     .PDOMapping      = true,
    //     .defaultValueStr = "12"};

    // EDSEntry::EDSEntryMetaData stringMetaData = {
    //     .parameterName   = std::string("String"),
    //     .objectType      = EDSEntry::ObjectType_E::VALUE,
    //     .storageLocation = EDSEntry::StorageLocation_E::RAM};
    // EDSEntryVal::EDSValueMetaData stringValueMetaData{
    //     .dataType        = EDSEntryVal::DataType_E::VISIBLE_STRING,
    //     .accessType      = EDSEntryVal::AccessRights_E::READ_WRITE,
    //     .PDOMapping      = true,
    //     .defaultValueStr = "Some text"};

    EDSEntryVal floatEntry(std::move(floatMetaData), std::move(floatValueMetaData));
    // EDSEntryVal uintEntry(std::move(uintMetaData), std::move(uintValueMetaData));
    // EDSEntryVal stringEntry(std::move(stringMetaData), std::move(stringValueMetaData));

    UNSIGNED16_t floatValue = floatEntry;
    floatEntry              = floatValue;
    EXPECT_FLOAT_EQ(floatValue, 1.0f);
}
