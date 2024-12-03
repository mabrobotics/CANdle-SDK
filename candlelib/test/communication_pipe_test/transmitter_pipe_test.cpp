#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "transmitter_pipe.h"
#include <memory>
#include <functional>
#include <utility>

using namespace mab;

class TransmitterPipeTests : public ::testing::Test
{
  protected:
    void SetUp() override
    {
    }
};

TEST_F(TransmitterPipeTests, shouldPass)
{
    ASSERT_TRUE(true);
}

TEST_F(TransmitterPipeTests, CheckPipeWithQueue)
{
    std::vector<u8> testData = {1, 2, 3, 4};
    std::vector<u8> outputBufferTest;
    auto            outputLambda = [&](std::vector<u8> data) { outputBufferTest = data; };
    TransmitterPipe tp(outputLambda);
    tp.enqueue(testData);
    tp.awaitComplete();
    ASSERT_THAT(outputBufferTest, testing::ElementsAre(1, 2, 3, 4));
}