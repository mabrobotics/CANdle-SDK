
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <array>
#include <future>
#include <numeric>
#include <thread>
#include <semaphore>
#include <thread>

#include "candle_frame_adapter.hpp"
#include "candle_frame_dto.hpp"
#include "crc.hpp"

using namespace mab;
class CandleFrameAdapterTest : public ::testing::Test
{
  public:
    void mockReadWrite(std::stop_token stoken, mab::CANdleFrameAdapter* cfaPtr)
    {
        while (!stoken.stop_requested())
        {
            m_binSem.acquire();
            if (stoken.stop_requested())
                break;
            auto fr = cfaPtr->getPackedFrame();
            cfaPtr->parsePackedFrame(fr.first, fr.second.load());
        }
    }

  protected:
    static constexpr size_t                    CANDLE_FRAME_COUNT = 10;
    std::vector<std::vector<u8>>               mockDataVector;
    std::counting_semaphore<>                  m_binSem = std::counting_semaphore<>(0);
    std::atomic<bool>                          m_stopRequested{false};
    std::shared_ptr<std::function<void(void)>> m_sync;

    void SetUp() override
    {
        Logger::g_m_verbosity = Logger::Verbosity_E::SILENT;
        for (size_t i = 0; i < CANDLE_FRAME_COUNT; i++)
        {
            std::vector<u8> v;
            v.resize(64, 0);
            std::iota(v.begin(), v.end(), 1);
            mockDataVector.push_back(v);
            m_sync = std::make_shared<std::function<void(void)>>([this]() { m_binSem.release(); });
        }
    }
    void TearDown() override
    {
    }
};

TEST_F(CandleFrameAdapterTest, simultaneousReadWrite)
{
    mab::CANdleFrameAdapter cfa(m_sync);

    std::jthread thread(&CandleFrameAdapterTest::mockReadWrite, this, &cfa);

    std::vector<std::future<std::pair<std::vector<u8>, CANdleFrameAdapter::Error_t>>> futures;

    canId_t canId   = 100;
    u16     timeout = 10;
    for (const auto& data : mockDataVector)
    {
        futures.push_back(std::async(std::launch::async,
                                     &CANdleFrameAdapter::accumulateFrame,
                                     &cfa,
                                     canId++,
                                     data,
                                     timeout));
    }

    for (auto& future : futures)
    {
        auto result = future.get();
        EXPECT_EQ(CANdleFrameAdapter::Error_t::OK, result.second);
    }
    thread.request_stop();
    m_binSem.release();
}

TEST_F(CandleFrameAdapterTest, readerFailError)
{
    mab::CANdleFrameAdapter cfa(m_sync);

    std::jthread thread(&CandleFrameAdapterTest::mockReadWrite, this, &cfa);

    std::vector<std::future<std::pair<std::vector<u8>, CANdleFrameAdapter::Error_t>>> futures;

    canId_t canId   = 100;
    u16     timeout = 10;
    for (const auto& data : mockDataVector)
    {
        futures.push_back(std::async(std::launch::async,
                                     &CANdleFrameAdapter::accumulateFrame,
                                     &cfa,
                                     canId++,
                                     data,
                                     timeout));
    }
    CANdleFrameAdapter::Error_t lastResult = CANdleFrameAdapter::Error_t::OK;

    for (auto& future : futures)
    {
        auto result = future.get();
        thread.request_stop();
        m_binSem.release();
        lastResult = result.second;
    }
    EXPECT_EQ(CANdleFrameAdapter::Error_t::READER_TIMEOUT, lastResult);
}