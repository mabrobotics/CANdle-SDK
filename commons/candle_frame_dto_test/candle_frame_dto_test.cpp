#include <gtest/gtest.h>
#include "candle_frame_dto.hpp"
#include <iostream>

class CandleFrameDTOTest : public ::testing::Test
{
  protected:
    mab::CANdleFrame frame;

    void SetUp() override
    {
    }

    void TearDown() override
    {
    }
};

TEST_F(CandleFrameDTOTest, InitTest)
{
    frame.init(0x123, 1);
    EXPECT_EQ(frame.isValid(), false);
    EXPECT_EQ(frame.canId(), 0x123);
    EXPECT_EQ(frame.sequenceNo(), 1);
    EXPECT_EQ(frame.length(), 0);

    frame.addData((void*)"\x01\x02\x03\x04", 4);
    EXPECT_EQ(frame.length(), 4);
    EXPECT_EQ(frame.isValid(), true);

    frame.clear();
    EXPECT_EQ(frame.isValid(), false);
    EXPECT_EQ(frame.canId(), 0);
    EXPECT_EQ(frame.sequenceNo(), 0);
    EXPECT_EQ(frame.length(), 0);
}

TEST_F(CandleFrameDTOTest, serializeDeserializeTest)
{
    frame.init(0x123, 1);
    frame.addData((void*)"\x01\x02\x03\x04", 4);
    frame.addData((void*)"\x05\x06\x07\x08", 4);

    u8 buffer[mab::CANdleFrame::DTO_SIZE] = {0};
    frame.serialize(buffer);

    mab::CANdleFrame newFrame;
    newFrame.deserialize(buffer);

    EXPECT_EQ(newFrame.canId(), 0x123);
    EXPECT_EQ(newFrame.sequenceNo(), 1);
    EXPECT_EQ(newFrame.length(), 8);
    EXPECT_EQ(newFrame.isValid(), true);
    for (u8 i = 0; i < newFrame.length(); i++)
    {
        EXPECT_EQ(newFrame.data()[i], i + 1);
    }
}