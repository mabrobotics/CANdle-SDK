#pragma once

#include <mab_types.hpp>
#include <functional>
#include <memory>
#include <future>
#include <queue>
#include <condition_variable>
#include <mutex>
#include <stop_token>
#include "logger.hpp"
namespace mab
{
    template <typename T>
    class ThreadSafeQueue
    {
      private:
        std::queue<T>           m_queue;
        std::mutex              m_mutex;
        std::condition_variable m_cond;

      public:
        // Pushes an element to the queue
        void push(T item)
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_queue.push(item);
            m_cond.notify_one();
        }

        // Pops an element off the queue
        T pop()
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_cond.wait(lock, [this]() { return !m_queue.empty(); });
            T item = m_queue.front();
            m_queue.pop();
            return item;
        }
    };

    class TransmitterPipe
    {
        std::shared_ptr<ThreadSafeQueue<std::vector<u8>>> m_tsQueue;
        std::shared_ptr<Logger>                           m_log;
        std::stop_source                                  m_stopSource;

      public:
        using externalOutputFunction = std::function<void(std::vector<u8>)>;
        enum class transmitterPipeError_E : u8
        {
            OK                    = 0,
            THREAD_FAILED         = 1 << 1,
            OUPUT_FUNCTION_FAILED = 1 << 2,
        };

        TransmitterPipe(std::function<void(std::vector<u8>)> output)
        {
            m_log     = std::make_shared<Logger>(Logger::ProgramLayer_E::BOTTOM, "TransmitterPipe");
            m_tsQueue = std::make_shared<ThreadSafeQueue<std::vector<u8>>>();
            m_pipeFuture = std::async(std::launch::async,
                                      &addToQueue,
                                      m_stopSource.get_token(),
                                      m_tsQueue,
                                      std::move(output),
                                      m_log);
        }
        ~TransmitterPipe()
        {
            m_stopSource.request_stop();
        }

        transmitterPipeError_E enqueue(std::vector<u8> data);

      private:
        static transmitterPipeError_E addToQueue(
            std::stop_token                                   stopToken,
            std::shared_ptr<ThreadSafeQueue<std::vector<u8>>> tsQueue,
            const TransmitterPipe::externalOutputFunction&    output,
            std::shared_ptr<Logger>                           log);

        std::future<transmitterPipeError_E> m_pipeFuture;
        bool                                isThreadDead();
    };
}  // namespace mab