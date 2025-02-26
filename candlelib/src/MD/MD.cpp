#include "MD.hpp"

namespace mab
{

    MD::Error_t MD::init()
    {
        m_mdRegisters.hardwareType.value.deviceType = deviceType_E::UNKNOWN_DEVICE;
        auto hwDataReg                              = std::make_tuple(m_mdRegisters.hardwareType);
        auto mfDataResult                           = readRegisters(hwDataReg);

        if (mfDataResult.second != Error_t::OK)
            return Error_t::NOT_CONNECTED;

        m_mdRegisters.hardwareType = std::get<0>(mfDataResult.first);

        auto devType = m_mdRegisters.hardwareType.value.deviceType;

        if (devType != deviceType_E::UNKNOWN_DEVICE)
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