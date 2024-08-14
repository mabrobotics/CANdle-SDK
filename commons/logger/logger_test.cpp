#include <gtest/gtest.h>
#include <logger.hpp>
#include <type_traits>

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
    log.m_level = Logger::LogLevel_E::ERROR;
    log.m_tag   = "TEST";
    log.error("Some error!");
}

TEST_F(LoggerTests, printWarning)
{
    Logger log;
    log.m_level = Logger::LogLevel_E::WARN;
    log.m_tag   = "TEST";
    log.warn("Some warning!");
}

TEST_F(LoggerTests, printInfo)
{
    Logger log;
    log.m_level = Logger::LogLevel_E::INFO;
    log.m_tag   = "TEST";
    log.info("Some info!");
}

TEST_F(LoggerTests, printDebug)
{
    Logger log;
    log.m_level = Logger::LogLevel_E::DEBUG;
    log.m_tag   = "TEST";
    log.debug("Some debug!");
}

TEST_F(LoggerTests, printUI)
{
    Logger log;
    log.m_level = Logger::LogLevel_E::ERROR;
    log.m_tag   = "TEST";
    log.ui("Some ui stuff!%s", "123123123");
}