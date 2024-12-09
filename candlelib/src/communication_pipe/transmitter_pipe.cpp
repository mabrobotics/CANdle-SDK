#include "transmitter_pipe.h"
#include <exception>
#include <vector>
#include <chrono>
#include <utility>

namespace mab
{
    TransmitterPipe::transmitterPipeError_E TransmitterPipe::addToQueue(
        std::stop_token                                   stopToken,
        std::shared_ptr<ThreadSafeQueue<std::vector<u8>>> tsQueue,
        const TransmitterPipe::externalOutputFunction_t&  output,
        std::shared_ptr<Logger>                           log)
    {
        // m_stopSource shuts it down
        while (!stopToken.stop_requested())
        {
            try
            {
                // write to "output" callable created on constructor
                output(std::move(tsQueue->pop()));
            }
            catch (std::exception& e)
            {
                log->error("Write output failed: %s", e.what());
                return TransmitterPipe::transmitterPipeError_E::OUPUT_FUNCTION_FAILED;
            }
        }
        return TransmitterPipe::transmitterPipeError_E::OK;
    }

    TransmitterPipe::transmitterPipeError_E TransmitterPipe::enqueue(std::vector<u8> data)
    {
        if (isThreadDead())
            return TransmitterPipe::transmitterPipeError_E::THREAD_FAILED;

        // push to the queue to be handled by transmission thread later, "data" will be processed by
        // "output" callable eventually
        m_tsQueue->push(std::move(data));

        return TransmitterPipe::transmitterPipeError_E::OK;
    }

    bool TransmitterPipe::isThreadDead()
    {
        return !m_pipeFuture.valid() ||
               m_pipeFuture.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready;
    }

}  // namespace mab