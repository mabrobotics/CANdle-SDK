#include <gtest/gtest.h>
#include "pds_protocol.hpp"
#include "pds_module.hpp"
#include "power_stage.hpp"
#include "brake_resistor.hpp"
#include <string>
#include <cstring>

namespace mab
{

    class SetPropertyMessageTest : public ::testing::Test
    {
      protected:
        void SetUp() override
        {
        }
    };

    class GetPropertyMessageTest : public ::testing::Test
    {
      protected:
        void SetUp() override
        {
        }
    };

    TEST_F(SetPropertyMessageTest, DummyTestShouldPass)
    {
        EXPECT_TRUE(true);
    }

    TEST_F(SetPropertyMessageTest, SerializePropertyWriteMessageWithNoProperties)
    {
        PropertySetMessage testMessage(moduleType_E::BRAKE_RESISTOR, socketIndex_E::SOCKET_1);
        EXPECT_THROW(testMessage.serialize(), std::runtime_error);
    }

    TEST_F(SetPropertyMessageTest, CheckSerializedMessageWhenPropertyAdded)
    {
        std::vector<u8>    EXPECTED_SERIALIZED_MESSAGE = {0x21, 0x02, 0x01, 0x01, 0x02, 0x01};
        PropertySetMessage testMessage(moduleType_E::BRAKE_RESISTOR, socketIndex_E::SOCKET_1);
        testMessage.addProperty(propertyId_E::ENABLE, true);
        std::vector<u8> serializedMessage = testMessage.serialize();

        ASSERT_EQ(EXPECTED_SERIALIZED_MESSAGE.size(), serializedMessage.size())
            << "Serialized message has invalid size!";

        for (u8 i = 0; i < serializedMessage.size(); i++)
        {
            EXPECT_EQ(EXPECTED_SERIALIZED_MESSAGE[i], serializedMessage[i])
                << "Invalid byte at index: " << i;
        }
    }

    TEST_F(SetPropertyMessageTest, HandleExceededMessageSizeAfterSerialization)
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
            testMessage.addProperty(propertyId_E::ENABLE, 0xFFFFFFFF);
        }

        EXPECT_THROW(testMessage.serialize(), std::runtime_error);
    }

    TEST_F(GetPropertyMessageTest, DummyTestShouldPass)
    {
        EXPECT_TRUE(true);
    }

    TEST_F(GetPropertyMessageTest, SerializePropertyWriteMessageWithNoProperties)
    {
        PropertyGetMessage testMessage(moduleType_E::BRAKE_RESISTOR, socketIndex_E::SOCKET_1);
        EXPECT_THROW(testMessage.serialize(), std::runtime_error);
    }

    TEST_F(GetPropertyMessageTest, HandleExceededMessageSizeAfterSerialization)
    {
        PropertyGetMessage testMessage(moduleType_E::BRAKE_RESISTOR, socketIndex_E::SOCKET_1);
        /*
            TODO: Expecting that this code should be refactored in the future.
            The idea is that addProperty will be checking if property given is settable
            and if the given value has correct size according to the parameter
        */

        // Adding 100 properties for sure will exceed the fdcan buffer range.
        for (uint8_t i = 0; i < 100u; i++)
        {
            testMessage.addProperty(propertyId_E::ENABLE);
        }

        EXPECT_THROW(testMessage.serialize(), std::runtime_error);
    }

    TEST_F(GetPropertyMessageTest, CheckSerializedMessageWhenPropertyAdded)
    {
        std::vector<u8>    EXPECTED_SERIALIZED_MESSAGE = {0x20, 0x02, 0x01, 0x01, 0x02};
        PropertyGetMessage testMessage(moduleType_E::BRAKE_RESISTOR, socketIndex_E::SOCKET_1);
        testMessage.addProperty(propertyId_E::ENABLE);
        std::vector<u8> serializedMessage = testMessage.serialize();

        ASSERT_EQ(EXPECTED_SERIALIZED_MESSAGE.size(), serializedMessage.size())
            << "Serialized message has invalid size!";

        for (u8 i = 0; i < serializedMessage.size(); i++)
        {
            EXPECT_EQ(EXPECTED_SERIALIZED_MESSAGE[i], serializedMessage[i])
                << "Invalid byte at index: " << i;
        }
    }

    TEST_F(GetPropertyMessageTest, parseResponseWithSingleBoolPropertyWithoutErrors)
    {
        std::vector<u8> EXPECTED_SERIALIZED_MESSAGE = {0x20, 0x02, 0x01, 0x01, 0x02};
        std::vector<u8> RESPONSE                    = {0x00, 0x01, 0x00, 0x01};

        constexpr bool EXPECTED_READ_ENABLE_PROPERTY = true;

        PdsMessage::error_E result = PdsMessage::error_E::OK;

        bool readEnableProperty = false;

        PropertyGetMessage testMessage(moduleType_E::BRAKE_RESISTOR, socketIndex_E::SOCKET_1);
        testMessage.addProperty(propertyId_E::ENABLE);
        std::vector<u8> serializedMessage = testMessage.serialize();

        ASSERT_EQ(EXPECTED_SERIALIZED_MESSAGE.size(), serializedMessage.size())
            << "Serialized message has invalid size!";

        for (u8 i = 0; i < serializedMessage.size(); i++)
        {
            EXPECT_EQ(EXPECTED_SERIALIZED_MESSAGE[i], serializedMessage[i])
                << "Invalid byte at index: " << i;
        }

        result = testMessage.parseResponse(RESPONSE.data(), RESPONSE.size());
        ASSERT_EQ(PdsMessage::error_E::OK, result);

        u32 rawPropertyData = 0;

        result = testMessage.getProperty(propertyId_E::ENABLE, &rawPropertyData);
        ASSERT_EQ(PdsMessage::error_E::OK, result);

        readEnableProperty = static_cast<bool>(rawPropertyData);

        ASSERT_EQ(EXPECTED_READ_ENABLE_PROPERTY, readEnableProperty);
    }

    TEST_F(GetPropertyMessageTest, parsePowerStageGetBusVoltageResponse)
    {
        std::vector<u8> EXPECTED_SERIALIZED_MESSAGE   = {0x20, 0x04, 0x01, 0x01, 0x05};
        std::vector<u8> RESPONSE                      = {0x00, 0x01, 0x00, 0xE0, 0x5B, 0x00, 0x00};
        constexpr u32   EXPECTED_RECEIVED_BUS_VOLTAGE = 23520;

        PdsMessage::error_E result = PdsMessage::error_E::OK;

        u32 receivedBusVoltage = 0;

        PropertyGetMessage testMessage(moduleType_E::POWER_STAGE, socketIndex_E::SOCKET_1);
        testMessage.addProperty(propertyId_E::BUS_VOLTAGE);
        std::vector<u8> serializedMessage = testMessage.serialize();

        ASSERT_EQ(EXPECTED_SERIALIZED_MESSAGE.size(), serializedMessage.size())
            << "Serialized message has invalid size!";

        for (u8 i = 0; i < serializedMessage.size(); i++)
        {
            EXPECT_EQ(EXPECTED_SERIALIZED_MESSAGE[i], serializedMessage[i])
                << "Invalid byte at index: " << i;
        }

        result = testMessage.parseResponse(RESPONSE.data(), RESPONSE.size());
        ASSERT_EQ(PdsMessage::error_E::OK, result);

        result = testMessage.getProperty(propertyId_E::BUS_VOLTAGE, &receivedBusVoltage);
        ASSERT_EQ(PdsMessage::error_E::OK, result);

        ASSERT_EQ(EXPECTED_RECEIVED_BUS_VOLTAGE, receivedBusVoltage);
    }

    TEST_F(GetPropertyMessageTest, parseResponseWithResponseStatusError)
    {
        // Response containing single byte ( Response code :: ERROR )
        std::vector<u8> RESPONSE = {0x01};

        PdsMessage::error_E result = PdsMessage::error_E::OK;

        PropertyGetMessage testMessage(moduleType_E::BRAKE_RESISTOR, socketIndex_E::SOCKET_1);

        result = testMessage.parseResponse(RESPONSE.data(), RESPONSE.size());
        ASSERT_EQ(PdsMessage::error_E::RESPONSE_STATUS_ERROR, result);
    }

}  // namespace mab