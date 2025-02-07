#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <bus.hpp>
#include <candle_v2.hpp>
#include <mab_types.hpp>

#include <bit>
#include <memory>
#include <variant>

using ::testing::_;
using ::testing::Return;

class MockBusLegacy : public mab::Bus
{
  public:
    MOCK_METHOD(bool,
                transmit,
                (char* buffer,
                 int   len,
                 bool  waitForResponse,
                 int   timeout,
                 int   responseLen,
                 bool  faultVerbose),
                (override));
    MOCK_METHOD(bool, transfer, (char* buffer, int commandLen, int responseLen), (override));
    MOCK_METHOD(bool,
                receive,
                (int responseLen, int timeoutMs, bool checkCrc, bool faultVerbose),
                (override));
    MOCK_METHOD(unsigned long, getId, (), (override));
    MOCK_METHOD(std::string, getDeviceName, (), (override));
    MOCK_METHOD(void, flushReceiveBuffer, (), (override));
};

class CandleV2Test : public ::testing::Test
{
  protected:
    std::unique_ptr<MockBusLegacy> mockBus;

    struct __attribute__((packed)) exampleFrame_t
    {
        u16 mdId    = 0x64;
        u8  padding = 0x0;
        u16 dataOne = 0x3E;
        u16 dataTwo = 0xE7;
    };

    void SetUp() override
    {
        Logger::g_m_verbosity = Logger::Verbosity_E::SILENT;
        mockBus               = std::make_unique<MockBusLegacy>();
    }
};

TEST_F(CandleV2Test, failAttach)
{
    EXPECT_CALL(*mockBus, transmit(_, _, false, _, _, _)).Times(1).WillOnce(Return(false));
    EXPECT_THROW(mab::attachCandle(mab::CAN_BAUD_1M, std::move(mockBus)), std::runtime_error);
}

TEST_F(CandleV2Test, passAttach)
{
    EXPECT_CALL(*mockBus, transmit(_, _, false, _, _, _)).Times(1).WillOnce(Return(true));
    EXPECT_NO_THROW(mab::attachCandle(mab::CAN_BAUD_1M, std::move(mockBus)));
}

TEST_F(CandleV2Test, failAfterInit)
{
    EXPECT_CALL(*mockBus, transmit(_, _, false, _, _, _)).Times(1).WillOnce(Return(true));
    EXPECT_CALL(*mockBus, transmit(_, _, true, _, _, _)).Times(1).WillOnce(Return(false));
    auto            candle   = mab::attachCandle(mab::CAN_BAUD_1M, std::move(mockBus));
    std::vector<u8> mockData = {mab::CandleV2::CandleCommands_t::PING_START, 0x0};
    auto            result   = candle->transferCANFrame(mockData, 0);
    ASSERT_NE(result.second, mab::I_CommunicationDevice::Error_t::OK);
}

TEST_F(CandleV2Test, successAfterInit)
{
    EXPECT_CALL(*mockBus, transmit(_, _, false, _, _, _)).Times(1).WillOnce(Return(true));
    EXPECT_CALL(*mockBus, transmit(_, _, true, _, _, _)).Times(1).WillOnce(Return(true));
    auto            candle   = mab::attachCandle(mab::CAN_BAUD_1M, std::move(mockBus));
    std::vector<u8> mockData = {mab::CandleV2::CandleCommands_t::PING_START, 0x0};
    auto            result   = candle->transferCANFrame(mockData, 0);
    ASSERT_EQ(result.second, mab::I_CommunicationDevice::Error_t::OK);
}

TEST_F(CandleV2Test, transferCanData)
{
    exampleFrame_t canFrame = exampleFrame_t();

    size_t candleCanFrameHeaderSize =
        3;  // size of the header to let candle know to pass the rest of the frame

    std::vector<u8> canFrameBuffer;
    canFrameBuffer.reserve(sizeof(canFrameBuffer));
    canFrameBuffer.insert(
        canFrameBuffer.end(), (u8*)(&canFrame), (u8*)(&canFrame) + sizeof(canFrame));

    EXPECT_CALL(*mockBus, transmit(_, 2, false, _, _, _)).Times(1).WillOnce(Return(true));
    EXPECT_CALL(*mockBus, transmit(_, sizeof(canFrame) + candleCanFrameHeaderSize, true, _, _, _))
        .Times(1)
        .WillOnce(Return(true));
    auto candle = mab::attachCandle(mab::CAN_BAUD_1M, std::move(mockBus));

    candle->transferCANFrame(canFrameBuffer, 0);
}

TEST_F(CandleV2Test, deinitializeOnFail)
{
    exampleFrame_t canFrame = exampleFrame_t();

    size_t candleCanFrameHeaderSize =
        3;  // size of the header to let candle know to pass the rest of the frame

    std::vector<u8> canFrameBuffer;
    canFrameBuffer.reserve(sizeof(canFrameBuffer));
    canFrameBuffer.insert(
        canFrameBuffer.end(), (u8*)(&canFrame), (u8*)(&canFrame) + sizeof(canFrame));

    EXPECT_CALL(*mockBus, transmit(_, 2, false, _, _, _)).Times(2).WillRepeatedly(Return(true));
    EXPECT_CALL(*mockBus, transmit(_, sizeof(canFrame) + candleCanFrameHeaderSize, true, _, _, _))
        .Times(2)
        .WillOnce(Return(false))
        .WillRepeatedly(Return(true));
    auto candle = mab::attachCandle(mab::CAN_BAUD_1M, std::move(mockBus));

    auto result = candle->transferCANFrame(canFrameBuffer, 0);
    ASSERT_NE(result.second, mab::I_CommunicationDevice::OK);
    result = candle->transferCANFrame(canFrameBuffer, 0);
    ASSERT_EQ(result.second, mab::I_CommunicationDevice::OK);
}
