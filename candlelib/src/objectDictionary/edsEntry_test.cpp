#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <memory>
#include <utility>
#include "edsParser.hpp"
#include "mab_types.hpp"

#include "edsEntry.hpp"

using namespace mab;

class EdsEntry_test : public ::testing::Test
{
  protected:
    std::map<std::pair<u32, u8>, std::unique_ptr<mab::I_EDSEntry>> m_map;

    void SetUp() override
    {
        m_map.clear();
    }
    void TearDown() override
    {
        m_map.clear();
    }
};

TEST_F(EdsEntry_test, simpleValues)
{
    I_EDSEntry::EDSEntryMetaData u8Meta = {
        .parameterName   = "u8",
        .objectType      = I_EDSEntry::ObjectType_E::VALUE,
        .storageLocation = I_EDSEntry::StorageLocation_E::RAM,
        .dataType        = I_EDSEntry::DataType_E::UNSIGNED8,
        .accessType      = I_EDSEntry::accessRights_E::READ_WRITE,
        .PDOMapping      = true,
        .sectionType     = I_EDSEntry::SectionType_E::MANUFACTURER,
    };
    I_EDSEntry::EDSEntryMetaData floatMeta = {
        .parameterName   = "float",
        .objectType      = I_EDSEntry::ObjectType_E::VALUE,
        .storageLocation = I_EDSEntry::StorageLocation_E::RAM,
        .dataType        = I_EDSEntry::DataType_E::REAL64,
        .accessType      = I_EDSEntry::accessRights_E::READ_WRITE,
        .PDOMapping      = true,
        .sectionType     = I_EDSEntry::SectionType_E::MANUFACTURER,
    };
    // Create EDS object
    std::pair<u32, u8>   u8EntryAddress(0x100, 0x1);
    EDSEntryValue<u8>    u8Entry = (std::move(u8Meta));
    std::pair<u32, u8>   floatEntryAddress(0x100, 0x2);
    EDSEntryValue<float> floatEntry = (std::move(floatMeta));

    // Add to map
    m_map.emplace(u8EntryAddress, std::make_unique<EDSEntryValue<u8>>(std::move(u8Entry)));
    m_map.emplace(floatEntryAddress, std::make_unique<EDSEntryValue<float>>(std::move(floatEntry)));

    EXPECT_EQ(m_map[u8EntryAddress]->set(std::any((u8)12)), I_EDSEntry::Error_t::OK);
    EXPECT_EQ(m_map[floatEntryAddress]->set(std::any(0.5f)), I_EDSEntry::Error_t::OK);

    u8    valueU8    = std::any_cast<u8>(m_map[u8EntryAddress]->get());
    float valueFloat = std::any_cast<float>(m_map[floatEntryAddress]->get());

    EXPECT_FLOAT_EQ(valueFloat, 0.5f);
    EXPECT_EQ(valueU8, 12);
}
