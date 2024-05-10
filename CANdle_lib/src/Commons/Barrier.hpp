/**
 * @file Barrier.hpp
 * @author Piotr Wasilewski (piotr.wasilewski@mabrobotics.pl)
 * @brief
 * @version 0.1
 * @date 2024-03-18
 *
 * @copyright Copyright (c) 2024
 *
 */
#ifndef BARRIER_HPP
#define BARRIER_HPP

#include <condition_variable>
#include <mutex>
#include <thread>

/**
 * @brief std::barrier equivalent for mingw C++20
 *
 */
class Barrier
{
   public:
	explicit Barrier(size_t num_threads) : count(num_threads),
										   num_threads(num_threads),
										   generation(0) {}

	void wait()
	{
		std::unique_lock<std::mutex> lock(mutex);
		auto current_generation = generation;
		if (--count == 0)
		{
			generation++;
			count = num_threads;
			cv.notify_all();
		}
		else
		{
			cv.wait(lock, [this, current_generation]()
					{ return current_generation != generation; });
		}
	}

   private:
	std::mutex mutex;
	std::condition_variable cv;
	size_t count;
	size_t num_threads;
	size_t generation;
};

#endif