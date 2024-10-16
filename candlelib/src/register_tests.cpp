#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "register.hpp"
#include "candle.hpp"
#include <iostream>

class MockCandle : public mab::Candle
{
  public:
	MockCandle() : Candle(mab::CANdleBaudrate_E::CAN_BAUD_1M, false) {}
};

class RegisterTests : public ::testing::Test
{
  protected:
	void SetUp() override {}
};

TEST_F(RegisterTests, shouldPass) { ASSERT_TRUE(true); }
