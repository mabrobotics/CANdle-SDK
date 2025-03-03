#include "MD.hpp"

namespace mab
{

    MD::Error_t MD::init()
    {
        // TODO: add new hw struct support
        //  m_mdRegisters.hardwareType.value.deviceType = deviceType_E::UNKNOWN_DEVICE;
        //  auto mfDataResult                           = readRegister(m_mdRegisters.hardwareType);

        // if (mfDataResult.second == Error_t::OK)
        // {
        //     m_mdRegisters.hardwareType = mfDataResult.first;

        //     auto devType = m_mdRegisters.hardwareType.value.deviceType;

        //     if (devType != deviceType_E::UNKNOWN_DEVICE)
        //         return Error_t::OK;
        // }

        auto mfLegacydataResult = readRegister(m_mdRegisters.legacyHardwareVersion);
        if (mfLegacydataResult.second != Error_t::OK)
            return Error_t::NOT_CONNECTED;

        if (m_mdRegisters.legacyHardwareVersion.value != 0)
            return Error_t::OK;

        return Error_t::NOT_CONNECTED;
    }

    void MD::blink()
    {
        m_mdRegisters.runBlink = 1;

        auto result = writeRegisters(m_mdRegisters.runBlink);
        if (result != Error_t::OK)
        {
            m_log.error("Blink failed!");
        }
    }
    MD::Error_t MD::enable()
    {
        m_mdRegisters.state = 39;
        if (writeRegisters(m_mdRegisters.state) != MD::Error_t::OK)
        {
            m_log.error("Enabling failed");
            return MD::Error_t::TRANSFER_FAILED;
        }
        m_log.info("Driver enabled");
        m_mdRegisters.motionModeStatus = 0;
        if (readRegisters(m_mdRegisters.motionModeStatus).second != MD::Error_t::OK)
        {
            m_log.error("Motion status check failed");
            return MD::Error_t::TRANSFER_FAILED;
        }
        if (m_mdRegisters.motionModeStatus.value == mab::Md80Mode_E::IDLE)
            m_log.warn("Motion mode not set");

        return MD::Error_t::OK;
    }
    MD::Error_t MD::disable()
    {
        m_mdRegisters.state = 64;
        if (writeRegisters(m_mdRegisters.state) != MD::Error_t::OK)
        {
            m_log.error("Disabling failed");
            return MD::Error_t::TRANSFER_FAILED;
        }
        m_log.info("Driver disabled");
        return MD::Error_t::OK;
    }
}  // namespace mab