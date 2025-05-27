#include <asm-generic/errno-base.h>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <functional>

#include "I_communication_interface.hpp"
#include "I_communication_interface_mock.hpp"
#include "MD.hpp"
#include "candle.hpp"
#include "gmock/gmock.h"
#include "mab_types.hpp"
#include "md_types.hpp"

using testing::_;
using testing::Return;

class MD_test : public ::testing::Test
{
  protected:
    std::unique_ptr<MockBus> m_bus;
    MockBus*                 m_debugBus;
    mab::Candle*             m_candle;
    void                     SetUp() override
    {
        ::testing::FLAGS_gmock_verbose = "error";
        Logger::g_m_verbosity          = Logger::Verbosity_E::SILENT;
        m_bus                          = std::make_unique<MockBus>();
        m_debugBus                     = m_bus.get();
        m_candle                       = mab::attachCandle(mab::CAN_BAUD_1M, std::move(m_bus));
    }
    void TearDown() override
    {
        mab::detachCandle(m_candle);
    }
};

TEST_F(MD_test, checkROAccess)
{
    mab::MDRegisters_S registers;
    std::vector<u8>    mockReponse = {0x00,
                                      0x00,
                                      (u8)(registers.auxEncoderPosition.m_regAddress),
                                      (u8)(registers.auxEncoderPosition.m_regAddress >> 8),
                                      0x00,
                                      0x00,
                                      0x00,
                                      0x00};

    EXPECT_CALL(*m_debugBus, transfer(_, _, _))
        .Times(1)
        .WillOnce(Return(std::make_pair(mockReponse, mab::I_CommunicationInterface::Error_t::OK)));

    mab::MD md(100, m_candle);

    auto resultRead = md.readRegister(registers.auxEncoderPosition);
    EXPECT_EQ(resultRead, mab::MD::Error_t::OK);

    auto resultWrite = md.writeRegister(registers.auxEncoderPosition);
    EXPECT_EQ(resultWrite, mab::MD::Error_t::REQUEST_INVALID);
}

TEST_F(MD_test, checkWOAccess)
{
    mab::MDRegisters_S registers;
    mab::MD            md(100, m_candle);
    std::vector<u8>    mockReponse = {0x0,  // header
                                      0x01,
                                      0xA0,  // payload
                                      0x00};

    EXPECT_CALL(*m_debugBus, transfer(_, _, _))
        .Times(1)
        .WillOnce(Return(std::make_pair(mockReponse, mab::I_CommunicationInterface::Error_t::OK)));

    auto resultWrite = md.writeRegister(registers.runBlink);
    EXPECT_EQ(resultWrite, mab::MD::Error_t::OK);

    auto resultRead = md.readRegister(registers.runBlink);
    EXPECT_EQ(resultRead, mab::MD::Error_t::REQUEST_INVALID);
}
