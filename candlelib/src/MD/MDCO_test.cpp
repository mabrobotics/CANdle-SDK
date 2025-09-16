#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <functional>

#include "I_communication_interface.hpp"
#include "I_communication_interface_mock.hpp"
#include "MDCO.hpp"
#include "candle.hpp"
#include "gmock/gmock.h"
#include "mab_types.hpp"

using testing::_;
using testing::Return;

class MDCO_test : public ::testing::Test
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
        m_candle = mab::attachCandle(mab::CAN_DATARATE_1M, std::move(m_bus), true);
    }
    void TearDown() override
    {
        mab::detachCandle(m_candle);
    }
};

TEST_F(MDCO_test, checkROAccess)
{
    std::vector<u8> mockReponse = {0x4F, 0x61, 0x60, 0x00, 0xFF, 0x00, 0x00, 0x00};

    EXPECT_CALL(*m_debugBus, transfer(_, _, _))
        .Times(1)
        .WillRepeatedly(
            Return(std::make_pair(mockReponse, mab::I_CommunicationInterface::Error_t::OK)));

    mab::MDCO mdco(1, m_candle);

    auto resultRead = mdco.readOpenRegisters(0x6061, 0x0);
    EXPECT_EQ(resultRead, mab::MDCO::Error_t::OK);

    auto resultWrite = mdco.writeOpenRegisters(0x6061, 0x0, 0);
    EXPECT_EQ(resultWrite, mab::MDCO::Error_t::REQUEST_INVALID);
}