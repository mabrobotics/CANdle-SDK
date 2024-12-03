#include "transmitter_pipe.h"
#include <exception>
#include <vector>

namespace mab
{

    TransmitterPipe::transmitterPipeError_E TransmitterPipe::writeOutput(std::vector<u8> data)
    {
        try
        {
            m_output(data);
        }
        catch (std::exception& e)
        {
            m_log.error("Write output failed: %s", e.what());
            return TransmitterPipe::transmitterPipeError_E::OUPUT_FUNCTION_FAILED;
        }
        return TransmitterPipe::transmitterPipeError_E::OK;
    }

    TransmitterPipe::transmitterPipeError_E TransmitterPipe::enqueue(std::vector<u8> data)
    {
        return TransmitterPipe::transmitterPipeError_E::OK;
    }
    TransmitterPipe::transmitterPipeError_E TransmitterPipe::awaitComplete()
    {
        return TransmitterPipe::transmitterPipeError_E::OK;
    }

}  // namespace mab