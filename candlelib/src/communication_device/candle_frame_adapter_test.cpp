
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <array>
#include <future>
#include <numeric>
#include <thread>

#include "candle_frame_adapter.hpp"
#include "candle_frame_dto.hpp"
#include "crc.hpp"

using namespace mab;
class CandleFrameAdapterTest : public ::testing::Test
{
  protected:
    static constexpr size_t      CANDLE_FRAME_COUNT = 10;
    std::vector<std::vector<u8>> mockDataVector;

    void SetUp() override
    {
        for (size_t i = 0; i < CANDLE_FRAME_COUNT; i++)
        {
            std::vector<u8> v;
            v.resize(64, 0);
            std::iota(v.begin(), v.end(), 1);
            mockDataVector.push_back(v);
        }
    }
    void TearDown() override
    {
    }
};

TEST_F(CandleFrameAdapterTest, simultaneousReadWrite)
{
    mab::CANdleFrameAdapter cfa;

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

    auto fr = cfa.getPackedFrame();
    cfa.parsePackedFrame(fr);
    fr = cfa.getPackedFrame();
    cfa.parsePackedFrame(fr);

    for (auto& future : futures)
    {
        auto result = future.get();
        EXPECT_EQ(CANdleFrameAdapter::Error_t::OK, result.second);
    }
}