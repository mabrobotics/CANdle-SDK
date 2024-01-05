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
		std::atomic<bool> segmentedTransfer = false;

		uint32_t segmentedDataSize = 0;

		processSDO = [&](uint32_t driveId, uint16_t index, uint8_t subindex, std::span<uint8_t>& data, uint8_t command, uint32_t errorCode_)
		{
			segmentedTransfer = command == 0x41;

			if (errorCode_)
			{
				errorCode = errorCode_;
				return;
			}

			if (index == index_ && subindex == subindex_ && driveId == id)
			{
				std::copy(data.begin(), data.end(), dataOut.begin());
				SDOvalid = true;

				if (segmentedTransfer)
					segmentedDataSize = deserialize<uint32_t>(data.begin());
			}
		};

		if (!interface->sendCanFrame(frame))
			return false;

		if (!waitForActionWithTimeout([&]() -> bool
									  { return !SDOvalid; },
									  10))
			return false;

		if (segmentedTransfer)
		{
			segmentedReadOngoing = true;
			std::cout << "Segmented transfer detected! Size = " << segmentedDataSize << std::endl;

			auto it = dataOut.begin();
			std::atomic<bool> lastSegment = false;

			processSDO = [&](uint32_t driveId, uint16_t index, uint8_t subindex, std::span<uint8_t>& data, uint8_t command, uint32_t errorCode_)
			{
				if (errorCode_)
				{
					errorCode = errorCode_;
					return;
				}

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

		segmentedReadOngoing = false;

		return true;
	}

	template <typename T>
	bool writeSDO(uint32_t id, uint16_t index_, uint8_t subindex_, const T&& value, uint32_t& errorCode, uint32_t size = sizeof(T))
	{
		std::vector<uint8_t> data;
		/* ensure at least as many elements as there are in the largest element (sizeof(T)) */
		data.resize(100, 0);

		serialize(value, data.begin());

		if (!writeSdoBytes(id, index_, subindex_, data, size, errorCode))
			return false;

		return true;
	}

	bool writeSdoBytes(uint32_t id, uint16_t index_, uint8_t subindex_, std::vector<uint8_t>& dataIn, uint32_t size, uint32_t& errorCode)
	{
		bool segmentedTransfer = false;

		if (size > 4)
			segmentedTransfer = true;

		ICommunication::CANFrame frame{};
		frame.header.canId = 0x600 + id;
		frame.header.length = 8;

		if (segmentedTransfer)
		{
			frame.payload[0] = 0x21;
			serialize(index_, &frame.payload[1]);
			frame.payload[3] = subindex_;
			serialize(size, &frame.payload[4]);
		}
		else
		{
			frame.payload[0] = 0b00100011 | ((4 - size) << 2);
			serialize(index_, &frame.payload[1]);
			frame.payload[3] = subindex_;
			std::copy(dataIn.begin(), dataIn.end(), &frame.payload[4]);
		}

		std::atomic<bool> SDOvalid = false;
		processSDO = [&](uint32_t driveId, uint16_t index, uint8_t subindex, std::span<uint8_t>& data, uint8_t command, uint32_t errorCode_)
		{
			if (errorCode_)
			{
				errorCode = errorCode_;
				return;
			}

			if (index == index_ && subindex == subindex_ && driveId == id)
				SDOvalid = true;
		};

		if (!interface->sendCanFrame(frame))
			return false;

		if (!waitForActionWithTimeout([&]() -> bool
									  { return !SDOvalid; },
									  10))
			return false;

		if (segmentedTransfer)
		{
			std::cout << "Segmented transfer detected! Size = " << size << std::endl;

			processSDO = [&](uint32_t driveId, uint16_t index, uint8_t subindex, std::span<uint8_t>& data, uint8_t command, uint32_t errorCode_)
			{
				if (errorCode_)
				{
					errorCode = errorCode_;
					return;
				}

				if ((command & 0xef) == 0x20)
					SDOvalid = true;
			};

			auto it = dataIn.begin();
			bool toggleBit = false;
			bool result = true;
			bool lastSegment = false;

			while (size > 0 && result)
			{
				uint8_t currentSize = size >= 7 ? 7 : size;
				if (size < 7)
					lastSegment = true;

				uint8_t emptyBytes = 7 - currentSize;

				frame.payload = {};
				frame.payload[0] = 0x00;
				/* flip 5th bit - the toggle bit */
				if (toggleBit)
					frame.payload[0] ^= (1 << 4);
				frame.payload[0] |= (emptyBytes << 1);
				frame.payload[0] |= lastSegment;

				std::copy(it, it + currentSize, &frame.payload[1]);

				std::cout << "size: " << size << std::endl;

				size -= currentSize;
				it += currentSize;

				SDOvalid = false;

				if (!interface->sendCanFrame(frame))
					return false;

				result = waitForActionWithTimeout([&]() -> bool
												  { return !SDOvalid; },
												  10);

				toggleBit = !toggleBit;
			}
		}

		return true;
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

			uint32_t errorCode = 0;
			bool error = command == 0x80;

			if (segmentedReadOngoing && !error)
			{
				dataSize = 7 - ((command >> 1) & 0b00000111);
				data = std::span<uint8_t>(&frame.payload[1], dataSize);
			}
			else
			{
				dataSize = 4 - ((command >> 2) & 0b00000011);
				index = deserialize<uint16_t>(&frame.payload[1]);
				subindex = frame.payload[3];

				if (!error)
					data = std::span<uint8_t>(&frame.payload[4], dataSize);
				else
					errorCode = deserialize<uint32_t>(&frame.payload[4]);
			}

			if (processSDO)
				processSDO(driveId, index, subindex, data, command, errorCode);
		}
	}

   private:
	ICommunication* interface;
	std::function<void(uint32_t driveId, uint16_t index, uint8_t subindex, std::span<uint8_t>& data, uint8_t command, uint32_t errorCode_)> processSDO;
	std::atomic<bool> segmentedReadOngoing = false;

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
