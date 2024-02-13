#include <gtest/gtest.h>

#include <optional>

#include "ConfigParser.hpp"
#include "MD80.hpp"
#include "gmock/gmock.h"

using testing::InSequence;

class ConfigParserTest : public ::testing::Test
{
   protected:
	void SetUp() override
	{
		logger = spdlog::stdout_color_mt("console");
		logger->set_pattern("[%^%l%$] %v");
		OD = &md80.OD;
	}

	void TearDown() override
	{
		spdlog::shutdown();
	}

   public:
	MD80 md80;
	IODParser::ODType* OD;
	std::shared_ptr<spdlog::logger> logger;
};

TEST_F(ConfigParserTest, testOpenFile)
{
	ConfigParser CP(logger, OD);
	auto status = CP.openFile("EX8108.cfg");
	ASSERT_EQ(status, true);
}

TEST_F(ConfigParserTest, testParse)
{
	ConfigParser CP(logger, OD);
	auto status = CP.openFile("EX8108.cfg");
	ASSERT_EQ(status, true);
	auto statusParse = CP.parseFile();
	ASSERT_EQ(statusParse.has_value(), true);
}

TEST_F(ConfigParserTest, testParseMissingFields)
{
	ConfigParser CP(logger, OD);
	CP.openFile("EX8108_missing_fields.cfg");
	auto statusParse = CP.parseFile();
	ASSERT_EQ(statusParse, std::nullopt);
}
