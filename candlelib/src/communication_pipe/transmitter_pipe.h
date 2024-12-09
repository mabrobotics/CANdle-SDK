#pragma once

#include <mab_types.hpp>
#include <functional>
#include <memory>
#include <future>
#include <queue>
#include <condition_variable>
#include <mutex>
#include <stop_token>
#include <utility>
#include "logger.hpp"
#include "thread_safe_queue.h"
namespace mab
{
    class TransmitterPipe
    {
        using internalThreadSafeQueue_t = ThreadSafeQueue<std::vector<u8>>;
        std::shared_ptr<internalThreadSafeQueue_t> m_tsQueue;
        std::shared_ptr<Logger>                    m_log;
        std::stop_source                           m_stopSource;

      public:
        // TYPEDEFS
        using externalOutputFunction_t = std::function<void(std::vector<u8>)>;

        // ENUMS
        /// @brief error types for error handling outside the class
        enum class transmitterPipeError_E : u8
        {
            OK                    = 0,
            THREAD_FAILED         = 1 << 1,
            OUPUT_FUNCTION_FAILED = 1 << 2,
        };

        // CONSTRUCTORS AND DESTRUCTORS
        /// @brief create TransmitterPipe instance
        /// @param output callable to be called by transmitter thread when and "data" object
        /// arrives on the queue. Must be thread-safe. Its main purpose is to write data to
        /// external device for sending.
        explicit TransmitterPipe(std::function<void(std::vector<u8>)> output)
        {
            m_log     = std::make_shared<Logger>(Logger::ProgramLayer_E::BOTTOM, "TransmitterPipe");
            m_tsQueue = std::make_shared<internalThreadSafeQueue_t>();
            m_pipeFuture = std::async(std::launch::async,
                                      &addToQueue,
                                      m_stopSource.get_token(),
                                      m_tsQueue,
                                      std::move(output),
                                      m_log);
        }
        /// @brief safely disposes of the internal thread
        ~TransmitterPipe()
        {
            m_log->info("Shutting down thread...");
            m_stopSource.request_stop();
            if (m_pipeFuture.wait_for(std::chrono::microseconds(100)) == std::future_status::ready)
            {
                m_log->info("Thread successfully shutdown");
                if (m_pipeFuture.get() != transmitterPipeError_E::OK)
                    m_log->warn("Thread was malfunctioning!");
            }
            else
                m_log->error("Could not shut down the thread!");
        }

        // METHODS
        /// @brief method for sending data to the "output" function
        /// @param data data to be written to queue
        /// @return on thread fail will return error, OK for normal operation
        transmitterPipeError_E enqueue(std::vector<u8> data);

      private:
        std::future<transmitterPipeError_E> m_pipeFuture;

        static transmitterPipeError_E addToQueue(
            std::stop_token                                   stopToken,
            std::shared_ptr<ThreadSafeQueue<std::vector<u8>>> tsQueue,
            const TransmitterPipe::externalOutputFunction_t&  output,
            std::shared_ptr<Logger>                           log);

        bool isThreadDead();
    };
}  // namespace mab