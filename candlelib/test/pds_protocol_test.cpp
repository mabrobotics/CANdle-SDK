#include <gtest/gtest.h>
#include "pds_protocol.hpp"
#include "pds_module.hpp"

namespace mab
{

    class BrakeResistorMessageTest : public ::testing::Test
    {
      protected:
        // PropertyWriteMessage testMessage;

        // BrakeResistorMessageTest()
        //     : testMessage(moduleType_E::BRAKE_RESISTOR, socketIndex_E::SOCKET_1)
        // {
        // }
        void SetUp() override
        {
        }
    };

    TEST_F(BrakeResistorMessageTest, DummyTestShouldPass)
    {
        EXPECT_TRUE(true);
    }

    TEST_F(BrakeResistorMessageTest, SerializePropertyWriteMessageWithNoProperties)
    {
        PropertySetMessage testMessage(moduleType_E::BRAKE_RESISTOR, socketIndex_E::SOCKET_1);
        EXPECT_THROW(testMessage.serialize(), std::runtime_error);
    }

    TEST_F(BrakeResistorMessageTest, CheckSerializedMessageWhenPropertyAdded)
    {
        std::vector<u8> EXPECTED_SERIALIZED_MESSAGE = {
            0x21, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01};
        PropertySetMessage testMessage(moduleType_E::BRAKE_RESISTOR, socketIndex_E::SOCKET_1);
        testMessage.addProperty(PowerStage::controlParameters_E::ENABLED, true);
        std::vector<u8> serializedMessage = testMessage.serialize();

        ASSERT_EQ(EXPECTED_SERIALIZED_MESSAGE.size(), serializedMessage.size());
        EXPECT_TRUE(true);
    }

    TEST_F(BrakeResistorMessageTest, HandleExceededMessageSizeAfterSerialization)
    {
        PropertySetMessage testMessage(moduleType_E::BRAKE_RESISTOR, socketIndex_E::SOCKET_1);
        /*
            TODO: Expecting that this code should be refactored in the future.
            The idea is that addProperty will be checking if property given is settable
            and if the given value has correct size according to the parameter
        */

        // Adding 100 properties for sure will exceed the fdcan buffer range.
        for (uint8_t i = 0; i < 100u; i++)
        {
            testMessage.addProperty(PowerStage::controlParameters_E::ENABLED, 0xFFFFFFFF);
        }

        EXPECT_THROW(testMessage.serialize(), std::runtime_error);
    }
}  // namespace mab