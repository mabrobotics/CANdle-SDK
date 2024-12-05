#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "transmitter_pipe.h"
#include <memory>
#include <functional>
#include <utility>
#include <semaphore>

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
    std::vector<u8>       testData = {1, 2, 3, 4};
    std::vector<u8>       outputBufferTest;
    std::binary_semaphore smphSignalMainToThread{0}, smphSignalThreadToMain{0};

    static auto outputLambda = [&](std::vector<u8> data)
    {
        smphSignalMainToThread.acquire();
        outputBufferTest = data;
        smphSignalThreadToMain.release();
    };
    TransmitterPipe*                        tp      = new TransmitterPipe(outputLambda);
    TransmitterPipe::transmitterPipeError_E errCode = tp->enqueue(testData);
    smphSignalMainToThread.release();
    if (errCode != TransmitterPipe::transmitterPipeError_E::OK)
        FAIL();
    smphSignalThreadToMain.acquire();
    EXPECT_THAT(outputBufferTest, testing::ElementsAre(1, 2, 3, 4));
    testData = {4, 2, 1, 1};
    errCode  = tp->enqueue(testData);
    if (errCode != TransmitterPipe::transmitterPipeError_E::OK)
        FAIL();
    smphSignalMainToThread.release();
    delete (tp);
    EXPECT_THAT(outputBufferTest, testing::ElementsAre(4, 2, 1, 1));
}