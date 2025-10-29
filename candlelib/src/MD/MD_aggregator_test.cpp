#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "MD_aggregator.hpp"
#include "md_types.hpp"

using namespace mab;

class MDAggregatorTest : public ::testing::Test
{
  protected:
    void SetUp() override
    {
    }

    void TearDown() override
    {
    }
};

TEST_F(MDAggregatorTest, UsageCompilationTest)
{
    auto sendFrameFunction =
        std::make_shared<std::function<MDAggregator::CFFrameFuture_t(candleTypes::CANFrameData_t)>>(
            [](candleTypes::CANFrameData_t frame)
            {
                std::promise<std::pair<std::vector<u8>, candleTypes::Error_t>> prom;
                prom.set_value({std::vector<u8>{}, candleTypes::Error_t::OK});
                return prom.get_future().share();
            });

    auto busyFlag = std::make_shared<std::atomic_bool>(false);

    MDAggregator aggregator(100, sendFrameFunction, busyFlag);

    MDRegisters_S regs;

    auto future = aggregator.readRegisterAsync(std::move(regs.positionLimitMax));
}
