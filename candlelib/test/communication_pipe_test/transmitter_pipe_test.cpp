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
        Logger::g_m_verbosity = Logger::Verbosity_E::SILENT;
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

    auto outputLambda = [&](std::vector<u8> data)  // scope MUST outlive the transmitter pipe
    {
        smphSignalMainToThread.acquire();
        outputBufferTest = data;
        smphSignalThreadToMain.release();
    };

    TransmitterPipe*                        tp      = new TransmitterPipe(outputLambda);
    TransmitterPipe::transmitterPipeError_E errCode = tp->enqueue(testData);

    smphSignalMainToThread.release();
    EXPECT_EQ(errCode, TransmitterPipe::transmitterPipeError_E::OK);
    smphSignalThreadToMain.acquire();
    EXPECT_THAT(outputBufferTest, testing::ElementsAre(1, 2, 3, 4));

    testData = {4, 2, 1, 1};
    errCode  = tp->enqueue(testData);
    EXPECT_EQ(errCode, TransmitterPipe::transmitterPipeError_E::OK);
    smphSignalMainToThread.release();
    delete (tp);
    EXPECT_THAT(outputBufferTest, testing::ElementsAre(4, 2, 1, 1));
}

TEST_F(TransmitterPipeTests, CheckThreadFail)
{
    std::binary_semaphore smphSignalThreadToMain{0};

    auto outputLambda = [&](std::vector<u8> data)  // scope MUST outlive the transmitter pipe
    {
        try
        {
            throw std::runtime_error("ExpectedToFaul");
        }
        catch (std::exception& e)
        {
            smphSignalThreadToMain.release();
            throw std::runtime_error(e.what());
        }
    };

    TransmitterPipe* tp = new TransmitterPipe(outputLambda);
    tp->enqueue({0});
    smphSignalThreadToMain.acquire();
    std::this_thread::sleep_for(
        std::chrono::microseconds(100));  // TODO: find more assured way of waiting for the throw
    TransmitterPipe::transmitterPipeError_E errCode = tp->enqueue({0});
    delete (tp);
    EXPECT_EQ(errCode, TransmitterPipe::transmitterPipeError_E::THREAD_FAILED);
}