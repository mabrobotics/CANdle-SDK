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
    log.m_layer      = Logger::ProgramLayer_E::TOP;
    *log.m_verbosity = Logger::Verbosity_E::DEFAULT;
    ASSERT_EQ(log.getVerbosity(), Logger::LogLevel_E::WARN);
    log.m_layer      = Logger::ProgramLayer_E::BOTTOM;
    *log.m_verbosity = Logger::Verbosity_E::VERBOSITY_3;
    ASSERT_EQ(log.getVerbosity(), Logger::LogLevel_E::DEBUG);
    log.m_layer      = Logger::ProgramLayer_E::MIDDLE;
    *log.m_verbosity = Logger::Verbosity_E::SILENT;
    ASSERT_EQ(log.getVerbosity(), Logger::LogLevel_E::SILENT);
}
