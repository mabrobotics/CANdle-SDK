#ifndef CANOPEN_STACK_HPP
#define CANOPEN_STACK_HPP

#include <atomic>
#include <functional>
#include <span>
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
	bool readSDO(uint32_t id, uint16_t index_, uint8_t subindex_, T& value)
	{
		ICommunication::CANFrame frame{};
		frame.header.canId = 0x600 + id;
		frame.header.length = 8;

		frame.payload[0] = 0x40;
		serialize(index_, &frame.payload[1]);
		frame.payload[3] = subindex_;

		std::atomic<bool> SDOvalid = false;
		processSDO = [&](uint32_t driveId, uint16_t index, uint8_t subindex, std::span<uint8_t>& data)
		{
			std::cout << "CANID: " << (int)driveId << " size: " << (int)data.size() << std::endl;
			value = deserialize<T>(data.begin());
			if (index == index_ && subindex == subindex_ && driveId == id)
				SDOvalid = true;
		};

		if (!interface->sendCanFrame(frame))
			return false;

		if (!waitForActionWithTimeout([&]() -> bool
									  { return !SDOvalid; },
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

			auto frame = maybeFrame.value();

			if (frame.header.canId >= 0x580 && frame.header.canId < 0x600)
			{
				uint32_t driveId = frame.header.canId - 0x580;
				size_t dataSize = 4 - ((frame.payload[0] >> 2) & 0b00000011);
				uint16_t index = deserialize<uint16_t>(&frame.payload[1]);
				uint8_t subindex = frame.payload[3];
				std::span<uint8_t> data(&frame.payload[4], dataSize);

				if (processSDO)
					processSDO(driveId, index, subindex, data);
			}
		}
	}

   private:
	std::thread receiveThread;

	ICommunication* interface;
	std::atomic<bool> done = false;

	std::function<void(uint32_t, uint32_t, uint8_t, std::span<uint8_t>&)> processSDO;

	bool waitForActionWithTimeout(std::function<bool()> condition, uint32_t timeoutMs)
	{
		auto start_time = std::chrono::high_resolution_clock::now();
		auto end_time = start_time + std::chrono::milliseconds(timeoutMs);

		while (condition())
		{
			if (std::chrono::high_resolution_clock::now() >= end_time)
				return false;
		}
		return true;
	}
};

#endif
