
#include "I_communication_interface_mock.hpp"
#include "mab_types.hpp"
#include "logger.hpp"
#include "candle_bootloader.hpp"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

using ::testing::_;
using ::testing::Return;

class CandleBootloaderTest : public ::testing::Test
{
  protected:
    std::unique_ptr<MockBus> mockBus;
    std::vector<u8>          mockData;
    u32                      mockId = 100;

    struct __attribute__((packed)) exampleFrame_t
    {
        u16 mdId    = 0x64;
        u8  padding = 0x0;
        u16 dataOne = 0x3E;
        u16 dataTwo = 0xE7;
    };

    void SetUp() override
    {
        mockData                       = {0xFA, 0x1};
        Logger::g_m_verbosity          = Logger::Verbosity_E::SILENT;
        mockBus                        = std::make_unique<MockBus>();
        ::testing::FLAGS_gmock_verbose = "error";
    }
};

TEST_F(CandleBootloaderTest, buildAndInit)
{
    mab::CandleBootloader bt = mab::CandleBootloader(std::move(mockBus));
}
