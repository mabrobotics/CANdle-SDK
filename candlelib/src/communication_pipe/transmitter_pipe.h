#pragma once

#include <mab_types.hpp>
#include <functional>
#include <memory>
#include <thread>
#include "logger.hpp"
namespace mab
{
    class TransmitterPipe
    {
        std::function<void(std::vector<u8>)> m_output;

        Logger m_log = Logger(Logger::ProgramLayer_E::BOTTOM, "Transmitter Pipe");

      public:
        enum class transmitterPipeError_E : u8
        {
            OK                    = 0,
            THREAD_FAILED         = 1 << 1,
            OUPUT_FUNCTION_FAILED = 1 << 2,
        };

        TransmitterPipe(std::function<void(std::vector<u8>)> output) : m_output(output)
        {
        }

        transmitterPipeError_E enqueue(std::vector<u8> data);
        transmitterPipeError_E awaitComplete();

      private:
        transmitterPipeError_E writeOutput(std::vector<u8> data);
    };
}  // namespace mab