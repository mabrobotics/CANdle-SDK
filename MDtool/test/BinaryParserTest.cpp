#include <gtest/gtest.h>

#include "BinaryParser.hpp"

TEST(BinaryParserTest, testFailOpenFile)
{
	BinaryParser BP;
	auto status = BP.processFile("testFailOpenFile.mab");
	ASSERT_EQ(status, BinaryParser::Status::ERROR_FILE);
}

TEST(BinaryParserTest, testFailWrongTag)
{
	BinaryParser BP;
	auto status = BP.processFile("testFailWrongTag.mab");
	ASSERT_EQ(status, BinaryParser::Status::ERROR_TAG);
}

TEST(BinaryParserTest, testFailWrongChecksum)
{
	BinaryParser BP;
	auto status = BP.processFile("testFailWrongChecksum.mab");
	ASSERT_EQ(status, BinaryParser::Status::ERROR_CHECKSUM);
}