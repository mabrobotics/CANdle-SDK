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
	enum PDO : uint16_t
	{
		RPDO1 = 0x200,
		RPDO2 = 0x300,
		RPDO3 = 0x400,
		RPDO4 = 0x500,
		TPDO1 = 0x180,
		TPDO2 = 0x280,
		TPDO3 = 0x380,
		TPDO4 = 0x480
	};

	enum PDOType
	{
		RPDO = 1,
		TPDO = 2,
	};

	explicit CanopenStack(std::shared_ptr<ICommunication> interface, std::shared_ptr<spdlog::logger> logger);
	void setOD(uint32_t id, IODParser::ODType* OD);
	void setChannel(uint32_t id, uint8_t channel);

	template <typename T>
	bool readSDO(uint32_t id, uint16_t index_, uint8_t subindex_, T& value, uint32_t& errorCode, bool checkOD = true, std::optional<uint8_t> maybeChannel = std::nullopt)
	{
		std::vector<uint8_t> data;
		/* ensure at least as many elements as there are in the largest element (sizeof(T)) */
		data.resize(maxSingleFieldSize, 0);

		uint8_t channel = 0;
		if (maybeChannel.has_value())
			channel = maybeChannel.value();
		else
			channel = idToChannelMap[id];

		std::optional<IODParser::Entry*> maybeEntry;

		if (checkOD)
		{
			maybeEntry = checkEntryExists(ODmap.at(id), index_, subindex_);
			if (!maybeEntry.has_value())
				return false;
		}

		if (!readSdoToBytes(id, index_, subindex_, data, errorCode, channel))
			return false;

		value = deserialize<T>(data.data());

		if (checkOD)
			maybeEntry.value()->value = value;

		return true;
	}

	bool readSdoToBytes(uint32_t id, uint16_t index_, uint8_t subindex_, std::vector<uint8_t>& dataOut, uint32_t& errorCode, uint8_t channel = 0);

	template <typename T>
	bool writeSDO(uint32_t id, uint16_t index_, uint8_t subindex_, const T& value, uint32_t& errorCode)
	{
		std::vector<uint8_t> data;
		/* ensure at least as many elements as there are in the largest element (sizeof(T)) */
		data.resize(maxSingleFieldSize, 0);

		auto maybeEntry = checkEntryExists(ODmap.at(id), index_, subindex_);

		if (!maybeEntry.has_value())
			return false;

		auto entry = maybeEntry.value();

		/* this simply checks the limits from EDS, only if they are enabled (their type is the same as the main value) */
		auto checkLimit = [&](const IODParser::ValueType& limit, auto&& comparator) -> bool
		{
			return std::visit([&](const auto& limitValue) -> bool
							  {
				using Type1 = std::remove_cvref_t<decltype(value)>;
				using Type2 = std::remove_cvref_t<decltype(limitValue)>;
				if constexpr (std::is_same<Type1, Type2>::value)
					return comparator(value, limitValue);
				return false; },
							  limit);
		};

		if (entry->value.index() != IODParser::ValueType(value).index())
		{
			logger->error("Provided type and type from the EDS are not consistent!");
			return false;
		}

		if (checkLimit(entry->lowLimit, std::less<T>()))
		{
			logger->error("The value exceeds low limit from the EDS file 0x{:x}:0x{:x}", index_, subindex_);
			return false;
		}
		else if (checkLimit(entry->highLimit, std::greater<T>()))
		{
			logger->error("The value exceeds high limit from the EDS file 0x{:x}:0x{:x}", index_, subindex_);
			return false;
		}

		serialize(value, data.begin());

		if (!writeSdoBytes(id, index_, subindex_, data, sizeof(T), errorCode, idToChannelMap[id]))
			return false;

		entry->value = value;
		return true;
	}

	bool writeSdoBytes(uint32_t id, uint16_t index_, uint8_t subindex_, const std::vector<uint8_t>& dataIn, uint32_t size, uint32_t& errorCode, uint8_t channel = 0);
	bool setupPDO(uint32_t id, PDO pdoId, const std::vector<std::pair<uint16_t, uint8_t>>& fields);
	bool sendSYNC();
	bool sendRPDOs();
	void parse(ICommunication::CANFrame& frame);
	std::optional<IODParser::Entry*> checkEntryExists(IODParser::ODType* OD, uint16_t index, uint8_t subindex);
	IODParser::ValueType getTypeBasedOnTag(IODParser::DataType tag);

   private:
	std::shared_ptr<ICommunication> interface;
	std::shared_ptr<spdlog::logger> logger;
	std::function<void(uint32_t driveId, uint16_t index, uint8_t subindex, std::span<uint8_t>& data, uint8_t command, uint32_t errorCode_)> processSDO;
	std::atomic<bool> segmentedReadOngoing = false;

	static constexpr uint8_t maxDevices = 31;
	static constexpr uint32_t defaultSdoTimeout = 10;
	static constexpr uint32_t maxSingleFieldSize = 100;

	std::unordered_map<uint32_t, IODParser::ODType*> ODmap;
	std::unordered_map<uint32_t, uint8_t> idToChannelMap;

	static constexpr uint16_t TPDOComunicationParamIndex = 0x1800;
	static constexpr uint16_t TPDOMappingParamIndex = 0x1A00;

	static constexpr uint16_t RPDOComunicationParamIndex = 0x1400;
	static constexpr uint16_t RPDOMappingParamIndex = 0x1600;

	bool fillODBasedOnTPDO(const ICommunication::CANFrame& frame);
	ICommunication::CANFrame prepareRPDO(IODParser::ODType* OD, uint8_t offset);
	PDOType getPDOType(PDO pdoId);
	bool sendFrameWaitForCompletion(const ICommunication::CANFrame& frame, std::atomic<bool>& conditionVar, uint32_t timeoutMs);
	bool waitForActionWithTimeout(std::function<bool()> condition, uint32_t timeoutMs);
};

#endif
