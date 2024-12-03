#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "receiver_pipe.h"


class ReceiverPipeTests : public ::testing::Test
{
  protected:
	void SetUp() override {}
};

TEST_F(ReceiverPipeTests, shouldPass) { ASSERT_TRUE(true); }
