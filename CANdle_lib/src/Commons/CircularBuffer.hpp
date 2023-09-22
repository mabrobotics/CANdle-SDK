/*
 * CircularBuffer.hpp
 *
 *  Created on: Sep 4, 2023
 *      Author: Piotr Wasilewski
 */

#ifndef MAB_CIRCULARBUFFER_HPP_
#define MAB_CIRCULARBUFFER_HPP_

#include <array>
#include <optional>
#include <mutex>

template <class T, size_t maxSize>
class CircularBuffer
{
public:
	CircularBuffer() = default;

	bool put(const T &item) noexcept
	{
		std::unique_lock<std::mutex> lock(mtx);

		bool wasFull = isFull;

		buf[head] = item;

		if (isFull)
			tail = (tail + 1) % maxSize;

		head = (head + 1) % maxSize;
		isFull = head == tail;

		return !wasFull;
	}

	std::optional<T> get() const noexcept
	{
		std::unique_lock<std::mutex> lock(mtx);

		if (empty())
			return std::nullopt;

		auto val = buf[tail];
		isFull = false;
		tail = (tail + 1) % maxSize;
		return val;
	}

	void reset() noexcept
	{
		std::unique_lock<std::mutex> lock(mtx);
		head = tail;
		isFull = false;
	}

	bool empty() const noexcept
	{
		return (!isFull && (head == tail));
	}

	bool full() const noexcept
	{
		return isFull;
	}

	size_t capacity() const noexcept
	{
		return maxSize;
	}

	size_t size() const noexcept
	{
		std::unique_lock<std::mutex> lock(mtx);

		size_t size = maxSize;

		if (!isFull)
		{
			if (head >= tail)
				size = head - tail;
			else
				size = maxSize + head - tail;
		}
		return size;
	}

	float occupancy()
	{
		return static_cast<float>(size()) / static_cast<float>(capacity());
	}

private:
	mutable std::mutex mtx{};
	std::array<T, maxSize> buf;
	size_t head = 0;
	mutable size_t tail = 0;
	mutable bool isFull = 0;
};

#endif /* MAB_CIRCULARBUFFER_HPP_ */
