#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <bus.hpp>
#include <candle_v2.hpp>

#include <memory>

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
    void SetUp() override
    {
    }
};

TEST_F(CandleV2Test, constructorTest)
{
    auto bus = std::make_unique<MockBusLegacy>();

    auto candle = mab::attachCandle(mab::CAN_BAUD_1M, std::move(bus));
}
