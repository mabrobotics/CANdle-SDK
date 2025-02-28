#include "MD.hpp"

namespace mab
{

    MD::Error_t MD::init()
    {
        // m_mdRegisters.hardwareType.value.deviceType = deviceType_E::UNKNOWN_DEVICE;
        // auto mfDataResult                           = readRegister(m_mdRegisters.hardwareType);

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
        auto regTuple          = std::make_tuple(m_mdRegisters.runBlink);

        auto result = writeRegisters(regTuple);
        if (result != Error_t::OK)
        {
            m_log.error("Blink failed!");
        }
    }
}  // namespace mab