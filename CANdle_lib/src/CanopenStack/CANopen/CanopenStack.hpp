#ifndef CANOPEN_STACK_HPP
#define CANOPEN_STACK_HPP

#include <atomic>
#include <functional>
#include <span>
#include <thread>
#include <unordered_map>

#include "Commons/Deserializer.hpp"
#include "ICommunication.hpp"
#include "IObjectDictionaryParser.hpp"
#include "spdlog/spdlog.h"

class CanopenStack
{
   public:
	enum RPDO : uint16_t
	{
		RPDO1 = 0x200,
		RPDO2 = 0x300,
		RPDO3 = 0x400,
		RPDO4 = 0x500
	};

	enum TPDO : uint16_t
	{
		TPDO1 = 0x180,
		TPDO2 = 0x280,
		TPDO3 = 0x380,
		TPDO4 = 0x480
	};

	explicit CanopenStack(ICommunication* interface, spdlog::logger* logger) : interface(interface), logger(logger)
	{
	}

	void setOD(uint32_t id, IODParser::ODType* OD)
	{
		ODmap[id] = OD;
	}

	template <typename T>
	bool readSDO(uint32_t id, uint16_t index_, uint8_t subindex_, T& value, uint32_t& errorCode, bool checkOD = true)
	{
		std::vector<uint8_t> data;
		/* ensure at least as many elements as there are in the largest element (sizeof(T)) */
		data.resize(100, 0);

		std::optional<IODParser::Entry*> maybeEntry;

		if (checkOD)
		{
			maybeEntry = checkEntryExists(ODmap.at(id), index_, subindex_);
			if (!maybeEntry.has_value())
				return false;
		}

		if (!readSdoToBytes(id, index_, subindex_, data, errorCode))
			return false;

		value = deserialize<T>(data.data());

		if (checkOD)
			maybeEntry.value()->value = value;

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

		if (!sendFrameWaitForCompletion(frame, SDOvalid, defaultSdoTimeout))
			return false;

		if (segmentedTransfer)
		{
			segmentedReadOngoing = true;
			logger->debug("Segmented transfer detected! Size = {}", segmentedDataSize);

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

				result = sendFrameWaitForCompletion(frame, SDOvalid, defaultSdoTimeout);
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

		auto maybeEntry = checkEntryExists(ODmap.at(id), index_, subindex_);

		if (!maybeEntry.has_value())
			return false;

		serialize(value, data.begin());

		if (!writeSdoBytes(id, index_, subindex_, data, size, errorCode))
			return false;

		maybeEntry.value()->value = value;

		return true;
	}

	bool writeSdoBytes(uint32_t id, uint16_t index_, uint8_t subindex_, const std::vector<uint8_t>& dataIn, uint32_t size, uint32_t& errorCode)
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

		if (!sendFrameWaitForCompletion(frame, SDOvalid, defaultSdoTimeout))
			return false;

		if (segmentedTransfer)
		{
			logger->debug("Segmented transfer detected! Size = {}", size);

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

				logger->debug("remaining size = {}", size);

				size -= currentSize;
				it += currentSize;

				result = sendFrameWaitForCompletion(frame, SDOvalid, defaultSdoTimeout);

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
		if (frame.header.canId >= static_cast<uint16_t>(TPDO::TPDO1) && frame.header.canId < static_cast<uint16_t>(TPDO::TPDO4))
			fillODBasedOnTPDO(frame);

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

	std::optional<IODParser::Entry*> checkEntryExists(IODParser::ODType* OD, uint16_t index, uint8_t subindex)
	{
		if (!OD->contains(index))
		{
			logger->error("Entry index not found in OD (0x{:x})", index);
			return std::nullopt;
		}

		else if (OD->contains(index) && OD->at(index)->objectType == IODParser::ObjectType::VAR)
			return OD->at(index).get();

		if (!OD->at(index)->subEntries.contains(subindex))
		{
			logger->error("Entry subindex not found in OD (0x{:x}:0x{:x})", index, subindex);
			return std::nullopt;
		}

		return OD->at(index)->subEntries.at(subindex).get();
	}

   private:
	ICommunication* interface;
	spdlog::logger* logger;
	std::function<void(uint32_t driveId, uint16_t index, uint8_t subindex, std::span<uint8_t>& data, uint8_t command, uint32_t errorCode_)> processSDO;
	std::atomic<bool> segmentedReadOngoing = false;
	static constexpr uint32_t defaultSdoTimeout = 10;

	std::unordered_map<uint32_t, IODParser::ODType*> ODmap;

	static constexpr uint16_t TPDOComunicationParamIndex = 0x1800;
	static constexpr uint16_t TPDOMappingParamIndex = 0x1A00;

	bool fillODBasedOnTPDO(const ICommunication::CANFrame& frame)
	{
		/* offset - 0 for 0x180, 1 for 0x280, 2 for 0x380, 3 for 0x480 */
		uint16_t offset = ((frame.header.canId & 0xff00) >> 8) - 1;

		uint32_t driveId = frame.header.canId & 0x1F;
		auto value = ODmap[driveId]->at(TPDOComunicationParamIndex + offset).get()->subEntries.at(0x01).get()->value;
		uint16_t COBID = std::get<uint32_t>(value);

		/* validate the received canID with OD's TPDO COBID */
		if ((COBID | driveId) != frame.header.canId)
			return false;

		uint8_t numberOfObjects = std::get<uint8_t>(ODmap[driveId]->at(TPDOMappingParamIndex + offset).get()->subEntries.at(0x00).get()->value);

		auto it = frame.payload.begin();

		for (int i = 1; i <= numberOfObjects; i++)
		{
			uint32_t mappedObject = std::get<uint32_t>(ODmap[driveId]->at(TPDOMappingParamIndex + offset).get()->subEntries.at(i).get()->value);
			uint16_t index = mappedObject >> 16;
			uint8_t subindex = (mappedObject & 0x0000ff00) >> 8;

			auto entry = checkEntryExists(ODmap[driveId], index, subindex);

			if (!entry.has_value())
				return false;

			auto lambdaFunc = [&](auto& arg)
			{
				using T = std::decay_t<decltype(arg)>;
				arg = deserialize<T>(it);
				it += sizeof(T);
			};

			std::visit(lambdaFunc, entry.value()->value);
		}

		return true;
	}

	bool sendFrameWaitForCompletion(const ICommunication::CANFrame& frame, std::atomic<bool>& conditionVar, uint32_t timeoutMs)
	{
		conditionVar = false;

		if (!interface->sendCanFrame(frame))
			return false;

		return waitForActionWithTimeout([&]() -> bool
										{ return !conditionVar; },
										timeoutMs);
	}

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
