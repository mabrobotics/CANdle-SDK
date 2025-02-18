#include "MD.hpp"

namespace mab
{
    void MD::blink()
    {
        mdRegisters.runBlink     = 1;
        auto            regTuple = std::make_tuple(mdRegisters.runBlink);
        std::vector<u8> frame;
        frame.push_back((u8)MdFrameId_E::FRAME_WRITE_REGISTER);
        frame.push_back((u8)0x0);
        auto payload = serializeMDRegisters(regTuple);
        frame.insert(frame.end(), payload.begin(), payload.end());
    }
}  // namespace mab