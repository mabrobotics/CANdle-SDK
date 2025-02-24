#include "MD.hpp"

namespace mab
{
    void MD::blink()
    {
        mdRegisters.runBlink = 1;
        auto regTuple        = std::make_tuple(mdRegisters.runBlink);

        auto result = writeRegisters(regTuple);
        if (result != Error_t::OK)
        {
            m_log.error("Blink failed!");
        }
    }
}  // namespace mab