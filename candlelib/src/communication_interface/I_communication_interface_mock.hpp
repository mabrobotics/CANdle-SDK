#pragma once
#include "I_communication_interface.hpp"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

class MockBus : public mab::I_CommunicationInterface
{
  public:
    MOCK_METHOD(I_CommunicationInterface::Error_t, connect, (), (override));
    MOCK_METHOD(I_CommunicationInterface::Error_t, disconnect, (), (override));
    MOCK_METHOD(I_CommunicationInterface::Error_t,
                transfer,
                (std::vector<u8> data, const u32 timeoutMs),
                (override));
    MOCK_METHOD((std::pair<std::vector<u8>, I_CommunicationInterface::Error_t>),
                transfer,
                (std::vector<u8> data, const u32 timeoutMs, const size_t expectedReceivedDataSize),
                (override));
};