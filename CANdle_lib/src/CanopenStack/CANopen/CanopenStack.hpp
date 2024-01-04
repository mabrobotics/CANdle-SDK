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
	explicit CanopenStack(ICommunication* interface) : interface(interface)
	{
	}

	template <typename T>
	bool readSDO(uint32_t id, uint16_t index_, uint8_t subindex_, T& value, uint32_t& errorCode)
	{
		std::vector<uint8_t> data;
		/* ensure at least as many elements as there are in the largest element (sizeof(T)) */
		data.resize(100, 0);

		if (!readSdoToBytes(id, index_, subindex_, data, errorCode))
			return false;

		value = deserialize<T>(data.data());
		return true;
	}

	bool readSdoToBytes(uint32_t id, uint16_t index_, uint8_t subindex_, std::vector<uint8_t>& dataOut, uint32_t& errorCode)
	{
		ICommunication::CANFrame frame{};
		frame.header.canId = 0x600 + id;
		frame.header.length = 8;

		frame.payload[0] = 0x40;
		serialize(index_, &frame.payload[1]);
		frame.payload[3] = subindex_;

		std::atomic<bool> SDOvalid = false;
		std::atomic<bool> SDOfragmented = false;

		uint32_t fragmentedDataSize = 0;

		processSDO = [&](uint32_t driveId, uint16_t index, uint8_t subindex, std::span<uint8_t>& data, uint8_t command)
		{
			bool error = command == 0x80;
			SDOfragmented = command == 0x41;

			if (index == index_ && subindex == subindex_ && driveId == id)
			{
				if (error)
					errorCode = deserialize<uint32_t>(data.begin());
				std::copy(data.begin(), data.end(), dataOut.begin());
				SDOvalid = true;

				if (SDOfragmented)
					fragmentedDataSize = deserialize<uint32_t>(data.begin());
			}
		};

		if (!interface->sendCanFrame(frame))
			return false;

		if (!SDOfragmented && !waitForActionWithTimeout([&]() -> bool
														{ return !SDOvalid; },
														10))
			return false;

		if (SDOfragmented)
		{
			fragmentedReadOngoing = true;
			std::cout << "Segmented transfer detected! Size = " << fragmentedDataSize << std::endl;

			auto it = dataOut.begin();
			std::atomic<bool> lastSegment = false;

			processSDO = [&](uint32_t driveId, uint16_t index, uint8_t subindex, std::span<uint8_t>& data, uint8_t command)
			{
				bool error = command == 0x80;

				if (error)
					errorCode = deserialize<uint32_t>(data.begin() + 3);

				if ((command & 0xe1) == 0x01)
					lastSegment = true;

				std::copy(data.begin(), data.end(), it);
				it += data.size();

				SDOvalid = true;
			};

			bool result = true;
			bool toggleBit = false;

			while (!lastSegment & result)
			{
				frame.payload = {};
				frame.payload[0] = 0x60;

				/* flip 5th bit - the toggle bit */
				if (toggleBit)
					frame.payload[0] ^= (1 << 4);

				if (!interface->sendCanFrame(frame))
					return false;

				SDOvalid = false;
				result = waitForActionWithTimeout([&]() -> bool
												  { return !SDOvalid; },
												  10);

				toggleBit = !toggleBit;
			}
		}

		fragmentedReadOngoing = false;

		return true;
	}

	template <typename T>
	bool writeSDO(uint32_t id, uint16_t index_, uint8_t subindex_, const T&& value)
	{
		ICommunication::CANFrame frame{};
		frame.header.canId = 0x600 + id;
		frame.header.length = 8;

		frame.payload[0] = 0b00100011 | ((4 - sizeof(T)) << 2);
		serialize(index_, &frame.payload[1]);
		frame.payload[3] = subindex_;
		serialize(value, &frame.payload[4]);

		std::atomic<bool> SDOvalid = false;
		processSDO = [&](uint32_t driveId, uint16_t index, uint8_t subindex, std::span<uint8_t>& data, uint8_t command)
		{
			if (index == index_ && subindex == subindex_ && driveId == id)
				SDOvalid = true;
		};

		if (!interface->sendCanFrame(frame))
			return false;

		return waitForActionWithTimeout([&]() -> bool
										{ return !SDOvalid; },
										10);
	}

	bool sendSYNC()
	{
		ICommunication::CANFrame frame{};
		frame.header.canId = 0x80;
		frame.header.length = 0;
		return interface->sendCanFrame(frame);
	}

	void parse(ICommunication::CANFrame& frame)
	{
		if (frame.header.canId >= 0x580 && frame.header.canId < 0x600)
		{
			uint8_t command = frame.payload[0];
			uint32_t driveId = frame.header.canId - 0x580;

			size_t dataSize = 0;
			uint16_t index = 0;
			uint8_t subindex = 0;
			std::span<uint8_t> data;

			if (fragmentedReadOngoing)
			{
				dataSize = 7 - ((command >> 1) & 0b00000111);
				data = std::span<uint8_t>(&frame.payload[1], dataSize);
			}
			else
			{
				dataSize = 4 - ((command >> 2) & 0b00000011);
				index = deserialize<uint16_t>(&frame.payload[1]);
				subindex = frame.payload[3];
				data = std::span<uint8_t>(&frame.payload[4], dataSize);
			}

			if (processSDO)
				processSDO(driveId, index, subindex, data, command);
		}
	}

   private:
	ICommunication* interface;
	std::function<void(uint32_t, uint16_t, uint8_t, std::span<uint8_t>&, uint8_t command)> processSDO;
	std::atomic<bool> fragmentedReadOngoing = false;

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
