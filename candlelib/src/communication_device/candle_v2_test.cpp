
#include <I_communication_interface.hpp>
#include <candle_v2.hpp>
#include <mab_types.hpp>

#include <bit>
#include <memory>
#include <variant>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

using ::testing::_;
using ::testing::Return;

class MockBus : public mab::I_CommunicationInterface
{
  public:
    MOCK_METHOD(I_CommunicationInterface::Error_t, connect, (), (override));
    MOCK_METHOD(I_CommunicationInterface::Error_t, disconnect, (), (override));
    MOCK_METHOD(I_CommunicationInterface::Error_t,
                transfer,
                (std::vector<u8> data, const u32 timeoutMs),
                (override));
    MOCK_METHOD((std::pair<std::vector<u8>, I_CommunicationInterface::Error_t>),
                transfer,
                (std::vector<u8> data, const u32 timeoutMs, const size_t expectedReceivedDataSize),
                (override));
};

class CandleV2Test : public ::testing::Test
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
        mockData                       = {mab::CandleV2::CandleCommands_t::RESET, 0x0};
        Logger::g_m_verbosity          = Logger::Verbosity_E::SILENT;
        mockBus                        = std::make_unique<MockBus>();
        ::testing::FLAGS_gmock_verbose = "error";
    }
};

TEST_F(CandleV2Test, failAttach)
{
    EXPECT_CALL(*mockBus, connect())
        .Times(1)
        .WillOnce(Return(mab::I_CommunicationInterface::Error_t::NOT_CONNECTED));
    testing::Mock::AllowLeak(mockBus.get());
    EXPECT_THROW(mab::attachCandle(mab::CAN_BAUD_1M, std::move(mockBus)), std::runtime_error);
}

TEST_F(CandleV2Test, passAttach)
{
    EXPECT_CALL(*mockBus, connect())
        .Times(1)
        .WillOnce(Return(mab::I_CommunicationInterface::Error_t::OK));
    EXPECT_CALL(*mockBus, transfer(_, _, _))
        .Times(1)
        .WillOnce(Return(std::pair(mockData, mab::I_CommunicationInterface::Error_t::OK)));
    EXPECT_NO_THROW(mab::attachCandle(mab::CAN_BAUD_1M, std::move(mockBus)));
}

TEST_F(CandleV2Test, failAfterInit)
{
    EXPECT_CALL(*mockBus, connect())
        .Times(1)
        .WillOnce(Return(mab::I_CommunicationInterface::Error_t::OK));
    EXPECT_CALL(*mockBus, transfer(_, _, _))
        .Times(1)
        .WillOnce(Return(std::pair(mockData, mab::I_CommunicationInterface::Error_t::OK)));
    EXPECT_CALL(*mockBus, transfer(_, _))
        .WillOnce(Return(mab::I_CommunicationInterface::Error_t::UNKNOWN_ERROR));
    auto candle = mab::attachCandle(mab::CAN_BAUD_1M, std::move(mockBus));
    auto result = candle->transferCANFrame(candle, mockId, mockData, 0);
    ASSERT_NE(result.second, mab::candleTypes::Error_t::OK);
}

TEST_F(CandleV2Test, successAfterInit)
{
    EXPECT_CALL(*mockBus, connect())
        .Times(1)
        .WillOnce(Return(mab::I_CommunicationInterface::Error_t::OK));
    EXPECT_CALL(*mockBus, transfer(_, _, _))
        .Times(1)
        .WillOnce(Return(std::pair(mockData, mab::I_CommunicationInterface::Error_t::OK)));
    auto candle = mab::attachCandle(mab::CAN_BAUD_1M, std::move(mockBus));
    auto result = candle->transferCANFrame(candle, mockId, mockData, 0);
    ASSERT_EQ(result.second, mab::candleTypes::Error_t::OK);
}
