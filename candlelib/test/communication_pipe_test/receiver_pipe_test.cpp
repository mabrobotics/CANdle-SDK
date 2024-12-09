#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <memory>
#include <utility>
#include "mab_types.hpp"
#include "receiver_pipe.h"
#include "logger.hpp"

using namespace mab;

class ReceiverPipeTests : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        Logger::g_m_verbosity = Logger::Verbosity_E::SILENT;
    }
};

TEST_F(ReceiverPipeTests, shouldPass)
{
    ASSERT_TRUE(true);
}
