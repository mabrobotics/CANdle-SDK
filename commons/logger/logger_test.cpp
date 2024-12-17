#include <gtest/gtest.h>
#include <logger.hpp>

class LoggerTests : public ::testing::Test
{
  protected:
    void SetUp() override
    {
    }
};

TEST_F(LoggerTests, printError)
{
    Logger log;
    log.m_layer = Logger::ProgramLayer_E::TOP;
    ASSERT_EQ(log.getCurrentLevel(), Logger::LogLevel_E::INFO);
    log.m_layer           = Logger::ProgramLayer_E::BOTTOM;
    Logger::g_m_verbosity = Logger::Verbosity_E::VERBOSITY_3;
    ASSERT_EQ(log.getCurrentLevel(), Logger::LogLevel_E::DEBUG);
    log.m_layer           = Logger::ProgramLayer_E::LAYER_2;
    Logger::g_m_verbosity = Logger::Verbosity_E::SILENT;
    ASSERT_EQ(log.getCurrentLevel(), Logger::LogLevel_E::SILENT);
}
