#include <gtest/gtest.h>

class ExampleTest : public ::testing::Test
{
  protected:
    void SetUp() override
    {
    }
};

TEST_F(ExampleTest, DummyTestShouldPass)
{
    EXPECT_TRUE(true);
}
