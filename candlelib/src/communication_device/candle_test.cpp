
#include <I_communication_interface_mock.hpp>
#include <candle.hpp>
#include <mab_types.hpp>

#include <bit>
#include <memory>
#include <variant>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

using ::testing::_;
using ::testing::Return;

class CandleTest : public ::testing::Test
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
        mockData                       = {mab::Candle::CandleCommands_t::RESET, 0x1};
        Logger::g_m_verbosity          = Logger::Verbosity_E::SILENT;
        mockBus                        = std::make_unique<MockBus>();
        ::testing::FLAGS_gmock_verbose = "error";
    }
};

TEST_F(CandleTest, failAttach)
{
    EXPECT_CALL(*mockBus, connect())
        .Times(1)
        .WillOnce(Return(mab::I_CommunicationInterface::Error_t::NOT_CONNECTED));
    testing::Mock::AllowLeak(mockBus.get());
    auto candle = mab::attachCandle(mab::CAN_BAUD_1M, std::move(mockBus));
    EXPECT_EQ(candle, nullptr);
    mab::detachCandle(candle);
}

TEST_F(CandleTest, passAttach)
{
    EXPECT_CALL(*mockBus, connect())
        .Times(1)
        .WillOnce(Return(mab::I_CommunicationInterface::Error_t::OK));
    EXPECT_CALL(*mockBus, transfer(_, _, _))
        .Times(1)
        .WillOnce(Return(std::pair(mockData, mab::I_CommunicationInterface::Error_t::OK)));
    auto candle = mab::attachCandle(mab::CAN_BAUD_1M, std::move(mockBus));
    EXPECT_NE(candle, nullptr);
    mab::detachCandle(candle);
}

TEST_F(CandleTest, failAfterInit)
{
    EXPECT_CALL(*mockBus, connect())
        .Times(1)
        .WillOnce(Return(mab::I_CommunicationInterface::Error_t::OK));
    EXPECT_CALL(*mockBus, transfer(_, _, _))
        .Times(2)
        .WillOnce(Return(std::pair(mockData, mab::I_CommunicationInterface::Error_t::OK)))
        .WillOnce(
            Return(std::pair(mockData, mab::I_CommunicationInterface::Error_t::UNKNOWN_ERROR)));
    auto candle = mab::attachCandle(mab::CAN_BAUD_1M, std::move(mockBus));
    auto result = candle->transferCANFrame(mockId, mockData, 0);
    ASSERT_NE(result.second, mab::candleTypes::Error_t::OK);
    mab::detachCandle(candle);
}

TEST_F(CandleTest, successAfterInit)
{
    EXPECT_CALL(*mockBus, connect())
        .Times(1)
        .WillOnce(Return(mab::I_CommunicationInterface::Error_t::OK));
    EXPECT_CALL(*mockBus, transfer(_, _, _))
        .Times(2)
        .WillOnce(Return(std::pair(mockData, mab::I_CommunicationInterface::Error_t::OK)))
        .WillOnce(Return(std::pair(mockData, mab::I_CommunicationInterface::Error_t::OK)));
    auto candle = mab::attachCandle(mab::CAN_BAUD_1M, std::move(mockBus));
    auto result = candle->transferCANFrame(mockId, mockData, mockData.size());
    ASSERT_EQ(result.second, mab::candleTypes::Error_t::OK);
    mab::detachCandle(candle);
}
