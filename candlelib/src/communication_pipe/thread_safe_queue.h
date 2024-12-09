

#include <queue>
#include <condition_variable>
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
}  // namespace mab