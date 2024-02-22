#include "CanopenStack.hpp"

CanopenStack::CanopenStack(std::shared_ptr<ICommunication> interface, std::shared_ptr<spdlog::logger> logger) : interface(interface),
																												logger(logger)
{
}

void CanopenStack::setOD(uint32_t id, IODParser::ODType* OD)
{
	ODmap[id] = OD;
}

void CanopenStack::setChannel(uint32_t id, uint8_t channel)
{
	idToChannelMap[id] = channel;
}

bool CanopenStack::readSdoToBytes(uint32_t id, uint16_t index_, uint8_t subindex_, std::vector<uint8_t>& dataOut, uint32_t& errorCode, uint8_t channel)
{
	ICommunication::CANFrame frame{};
	frame.header.canId = 0x600 + id;
	frame.header.payloadSize = 8;
	frame.header.channel = channel;

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

bool CanopenStack::writeSdoBytes(uint32_t id, uint16_t index_, uint8_t subindex_, const std::vector<uint8_t>& dataIn, uint32_t size, uint32_t& errorCode, uint8_t channel)
{
	bool segmentedTransfer = false;

	if (size > 4)
		segmentedTransfer = true;

	ICommunication::CANFrame frame{};
	frame.header.canId = 0x600 + id;
	frame.header.payloadSize = 8;
	frame.header.channel = channel;

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
		std::copy(dataIn.begin(), dataIn.begin() + size, &frame.payload[4]);
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

bool CanopenStack::setupPDO(uint32_t id, PDO pdoId, const std::vector<std::pair<uint16_t, uint8_t>>& fields)
{
	auto pdoType = getPDOType(pdoId);

	uint16_t commParamIdx = pdoType == PDOType::RPDO ? RPDOComunicationParamIndex : TPDOComunicationParamIndex;
	uint16_t mapParamIdx = pdoType == PDOType::RPDO ? RPDOMappingParamIndex : TPDOMappingParamIndex;

	uint32_t COBID = static_cast<uint32_t>(pdoId) + id;
	uint32_t errorCode = 0;
	/*  disable PDO (set 31 bit to 1)*/
	if (!writeSDO(id, commParamIdx, 0x01, static_cast<uint32_t>(0x80000000 | COBID), errorCode))
		return false;

	/* set transsmission type to synch 1*/
	if (!writeSDO(id, commParamIdx, 0x02, static_cast<uint8_t>(1), errorCode))
		return false;

	/* set PDO mapping objects count to zero */
	if (!writeSDO(id, mapParamIdx, 0x00, static_cast<uint8_t>(0), errorCode))
		return false;

	uint8_t mapRegsubidx = 0;
	for (auto& [idx, subidx] : fields)
	{
		mapRegsubidx++;

		auto entry = checkEntryExists(ODmap[id], idx, subidx);

		if (!entry.has_value())
			return false;

		entry.value()->value = getTypeBasedOnTag(entry.value()->dataType);

		uint8_t currentlyHeldFieldSize = std::visit([](const auto& value) -> uint8_t
													{ return sizeof(std::decay_t<decltype(value)>); },
													entry.value()->value);

		/* size by 8 to get size in bits */
		uint32_t mappedObject = idx << 16 | subidx << 8 | (currentlyHeldFieldSize * 8);

		/* set PDO mapping objects count to zero */
		if (!writeSDO(id, mapParamIdx, mapRegsubidx, mappedObject, errorCode))
			return false;
	}

	/* set PDO mapping objects count to zero */
	if (!writeSDO(id, mapParamIdx, 0x00, static_cast<uint8_t>(mapRegsubidx), errorCode))
		return false;

	/*  enable PDO (set 31 bit to 0)*/
	if (!writeSDO(id, commParamIdx, 0x01, COBID, errorCode))
		return false;

	return true;
}

bool CanopenStack::sendSYNC()
{
	ICommunication::CANFrame frame{};
	frame.header.canId = 0x80;
	frame.header.payloadSize = 0;
	frame.header.channel = ICommunication::CANChannel::ALL;
	return interface->sendCanFrame(frame);
}

bool CanopenStack::sendRPDOs()
{
	for (auto& [deviceId, OD] : ODmap)
	{
		for (uint16_t i = 0; i < maxPdoNum; i++)
		{
			auto transmissionType = std::get<uint8_t>(OD->at(0x1400 + i)->subEntries.at(0x02)->value);

			/* TODO base it on SYNC as it should be */
			if (transmissionType > 0 && transmissionType < 250)
			{
				auto canFrame = prepareRPDO(OD, i);
				canFrame.header.canId |= deviceId;
				canFrame.header.channel = idToChannelMap[deviceId];
				interface->sendCanFrame(canFrame);
			}
		}
	}

	return true;
}

void CanopenStack::parse(ICommunication::CANFrame& frame)
{
	if (frame.header.canId >= static_cast<uint16_t>(PDO::TPDO1) && frame.header.canId < static_cast<uint16_t>(PDO::TPDO4))
		fillODBasedOnTPDO(frame);

	else if (frame.header.canId >= 0x580 && frame.header.canId < 0x600)
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
	else if (frame.header.canId >= 0x080 && frame.header.canId < (0x080 + maxDevices))
	{
		uint8_t id = frame.header.canId & 0x1f;
		uint16_t errorCode = deserialize<uint16_t>(&frame.payload[0]);
		uint16_t errorIndex = deserialize<uint16_t>(&frame.payload[2]);
		logger->error("Emergency frame received ID 0x{:x}! Error code (0x{:x}) error index (0x{:x})", id, errorCode, errorIndex);
	}
}

std::optional<IODParser::Entry*> CanopenStack::checkEntryExists(IODParser::ODType* OD, uint16_t index, uint8_t subindex)
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

IODParser::ValueType CanopenStack::getTypeBasedOnTag(IODParser::DataType tag)
{
	switch (tag)
	{
		case IODParser::DataType::BOOLEAN:
			[[fallthrough]];
		case IODParser::DataType::UNSIGNED8:
			return uint8_t{};
		case IODParser::DataType::INTEGER8:
			return int8_t{};
		case IODParser::DataType::UNSIGNED16:
			return uint16_t{};
		case IODParser::DataType::INTEGER16:
			return int16_t{};
		case IODParser::DataType::UNSIGNED32:
			return uint32_t{};
		case IODParser::DataType::INTEGER32:
			return int32_t{};
		case IODParser::DataType::REAL32:
			return float{};
		case IODParser::DataType::VISIBLE_STRING:
			return std::array<uint8_t, 24>{};
		default:
			return uint32_t{};
	}
}

bool CanopenStack::fillODBasedOnTPDO(const ICommunication::CANFrame& frame)
{
	/* offset - 0 for 0x180, 1 for 0x280, 2 for 0x380, 3 for 0x480 */
	uint16_t offset = ((frame.header.canId & 0xff00) >> 8) - 1;
	uint32_t driveId = frame.header.canId & 0x1F;

	if (!ODmap.contains(driveId))
		return false;

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

ICommunication::CANFrame CanopenStack::prepareRPDO(IODParser::ODType* OD, uint8_t offset)
{
	ICommunication::CANFrame canFrame{};

	canFrame.header.canId = std::get<uint32_t>(OD->at(RPDOComunicationParamIndex + offset)->subEntries.at(0x01)->value);
	uint8_t numberOfMappedObjects = std::get<uint8_t>(OD->at(RPDOMappingParamIndex + offset)->subEntries.at(0x00)->value);

	auto it = canFrame.payload.begin();

	for (uint8_t i = 1; i <= numberOfMappedObjects; i++)
	{
		uint32_t mappedObject = std::get<uint32_t>(OD->at(RPDOMappingParamIndex + offset).get()->subEntries.at(i).get()->value);
		uint16_t index = mappedObject >> 16;
		uint8_t subindex = (mappedObject & 0x0000ff00) >> 8;

		auto entry = checkEntryExists(OD, index, subindex);

		if (!entry.has_value())
			return ICommunication::CANFrame{};

		auto lambdaFunc = [&](auto& arg)
		{
			it += serialize(arg, it);
		};

		std::visit(lambdaFunc, entry.value()->value);
	}

	canFrame.header.payloadSize = it - canFrame.payload.begin();

	return canFrame;
}

CanopenStack::PDOType CanopenStack::getPDOType(PDO pdoId)
{
	switch (pdoId)
	{
		case PDO::RPDO1:
			[[fallthrough]];
		case PDO::RPDO2:
			[[fallthrough]];
		case PDO::RPDO3:
			[[fallthrough]];
		case PDO::RPDO4:
			return PDOType::RPDO;
		case PDO::TPDO1:
			[[fallthrough]];
		case PDO::TPDO2:
			[[fallthrough]];
		case PDO::TPDO3:
			[[fallthrough]];
		case PDO::TPDO4:
			return PDOType::TPDO;
		default:
			return PDOType::TPDO;
	}
}

bool CanopenStack::sendFrameWaitForCompletion(const ICommunication::CANFrame& frame, std::atomic<bool>& conditionVar, uint32_t timeoutMs)
{
	conditionVar = false;

	if (!interface->sendCanFrame(frame))
		return false;

	return waitForActionWithTimeout([&]() -> bool
									{ return !conditionVar; },
									timeoutMs);
}

bool CanopenStack::waitForActionWithTimeout(std::function<bool()> condition, uint32_t timeoutMs)
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
