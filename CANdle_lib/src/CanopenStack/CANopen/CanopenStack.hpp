#ifndef CANOPEN_STACK_HPP
#define CANOPEN_STACK_HPP

#include <atomic>
#include <thread>

#include "Commons/Deserializer.hpp"
#include "ICommunication.hpp"
#include "IObjectDictionaryParser.hpp"

class CanopenStack
{
   public:
	CanopenStack(ICommunication* interface) : interface(interface)
	{
		receiveThread = std::thread(&CanopenStack::receiveHandler, this);
	}

	~CanopenStack()
	{
		done = true;
		if (receiveThread.joinable())
			receiveThread.join();
	}

	template <typename T>
	bool readSDO(uint32_t id, uint16_t index_, uint8_t subindex_, T value)
	{
		ICommunication::CANFrame frame{};
		frame.header.canId = 0x600 + id;
		frame.header.length = 8;

		frame.payload[0] = 0x40;
		serialize(index_, &frame.payload[1]);
		frame.payload[3] = subindex_;

		index = 0;
		subindex = 0;
		driveId = 0;

		if (!interface->sendCanFrame(frame))
			return false;

		if (!waitForActionWithTimeout([&]() -> bool
									  { return !(index == index_ && subindex == subindex_ && driveId == id); },
									  10))
			return false;

		return true;
	}

	void receiveHandler()
	{
		while (!done)
		{
			auto maybeFrame = interface->receiveCanFrame();

			if (!maybeFrame.has_value())
				continue;

			std::cout << "FRAME RECEIVED!" << std::endl;

			auto frame = maybeFrame.value();

			if (frame.header.canId >= 0x580 && frame.header.canId < 0x600)
			{
				driveId = frame.header.canId - 0x580;
				size_t dataSize = (frame.payload[0] >> 2) & 0b00000011;
				index = deserialize<uint16_t>(&frame.payload[1]);
				subindex = frame.payload[3];

				std::cout << "CANID: " << (int)frame.header.canId << " index:" << (int)index << std::endl;
			}
		}
	}

   private:
	std::thread receiveThread;

	ICommunication* interface;

	std::atomic<uint16_t> index = 0;
	std::atomic<uint8_t> subindex = 0;
	std::atomic<uint32_t> driveId = 0;

	std::atomic<bool> done = false;

	bool waitForActionWithTimeout(std::function<bool()> condition, uint32_t timeoutMs)
	{
		auto start_time = std::chrono::high_resolution_clock::now();
		auto end_time = start_time + std::chrono::milliseconds(timeoutMs);

		while (condition())
		{
			if (std::chrono::high_resolution_clock::now() >= end_time)
			{
				std::cout << "Timeout!" << std::endl;
				return false;
			}
		}
		return true;
	}
};

#endif
