#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <functional>

#include "I_communication_interface.hpp"
#include "I_communication_interface_mock.hpp"
#include "MDCO.hpp"
#include "OD.hpp"
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
