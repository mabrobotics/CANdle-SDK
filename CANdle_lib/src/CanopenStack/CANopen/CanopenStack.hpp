/**
 * @file CanopenStack.hpp
 * @author Piotr Wasilewski (piotr.wasilewski@mabrobotics.pl)
 * @brief
 * @version 0.1
 * @date 2024-03-08
 *
 * @copyright Copyright (c) 2024
 *
 */
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

/**
 * @brief Implements minimal CANopen stack
 *
 */
class CanopenStack
{
   public:
	/**
	 * @brief Fixed PDO addresses.
	 *
	 */
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

	/**
	 * @brief PDO Type.
	 *
	 */
	enum PDOType
	{
		RPDO = 1, /**< command msg  */
		TPDO = 2, /**< response msg */
	};

	/**
	 * @brief Construct a new Canopen Stack object.
	 *
	 * @param interface shared_ptr to ICommunication derived object.
	 * @param logger shared_ptr to logger object.
	 */
	explicit CanopenStack(std::shared_ptr<ICommunication> interface, std::shared_ptr<spdlog::logger> logger);

	/**
	 * @brief Add OD pointer to the internal map.
	 *
	 * @param id ID of the MD80.
	 * @param OD pointer to the object dictionary.
	 */
	void setOD(uint32_t id, IODParser::ODType* OD);

	/**
	 * @brief Set the channel on which a specific MD80 can be communicated.
	 *
	 * @param id ID of the MD80.
	 * @param channel Candle channel
	 */
	void setChannel(uint32_t id, uint8_t channel);

	/**
	 * @brief reads SDO
	 *
	 * @tparam T type of the field, one of \ref ValueType.
	 * @param id
	 * @param index_ OD index.
	 * @param subindex_ OD subindex.
	 * @param value variable reference to which the read value will be written.
	 * @param errorCode SDO CiA301 error code.
	 * @param checkOD set to false if checking the type against the OD is not needed and the value should not be written to the MD80s OD, only returned using the value reference.
	 * @param maybeChannel set to a non-std::nullopt for reading SDO on specific channel. Useful for pinging.
	 * @return true
	 * @return false
	 */
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

	/**
	 * @brief Write SDO.
	 *
	 * @tparam T type of the field, one of \ref ValueType.
	 * @param id
	 * @param index_ OD index.
	 * @param subindex_ OD subindex.
	 * @param value variable reference to which the read value will be written.
	 * @param errorCode SDO CiA301 error code.
	 * @return true
	 * @return false
	 */
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
			logger->error("Provided type and type from the EDS are not consistent! 0x{:x}:0x{:x}", index_, subindex_);
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

	/**
	 * @brief Setup PDO objects.
	 *
	 * This function can be used to setup the response and command frames structures. Always check if the total field length does not exceed the max frame length (1M baudrate max is 8 bytes, 8M baudrate is 64 bytes). RPDOs are sent before SYNC msg, TPDO are received after SYNC msg. SYNC must be turned on using \ref setSendSync().
	 * @param id
	 * @param pdoID ID of the RPDO (response) or TPDO (command) \ref CanopenStack::PDO.
	 * @param fields std::vector of std::pairs of index and subindex from the OD that are expected in the response or command.
	 * @return true
	 * @return false
	 */
	bool setupPDO(uint32_t id, PDO pdoId, const std::vector<std::pair<uint16_t, uint8_t>>& fields);
	/**
	 * @brief Send SYNC msg.
	 *
	 * @return true
	 * @return false
	 */
	bool sendSYNC();
	/**
	 * @brief Send RPDOs based on MD80 OD configuration.
	 *
	 * @return true
	 * @return false
	 */
	bool sendRPDOs();
	/**
	 * @brief Method for parsing incoming CAN messages.
	 *
	 * @param frame
	 */
	void parse(ICommunication::CANFrame& frame);
	/**
	 * @brief Checks if a given field exists in the OD.
	 *
	 * @param OD pointer to the OD.
	 * @param index OD index.
	 * @param subindex OD subindex.
	 * @return std::optional<IODParser::Entry*>.
	 */
	std::optional<IODParser::Entry*> checkEntryExists(IODParser::ODType* OD, uint16_t index, uint8_t subindex);
	/**
	 * @brief Gets the IODParser::ValueType based on OD DataType tag.
	 *
	 * @param tag OD DataType tag.
	 * @return IODParser::ValueType.
	 */
	IODParser::ValueType getTypeBasedOnTag(IODParser::DataType tag);

   private:
	std::shared_ptr<ICommunication> interface;
	std::shared_ptr<spdlog::logger> logger;
	std::function<void(uint32_t driveId, uint16_t index, uint8_t subindex, std::span<uint8_t>& data, uint8_t command, uint32_t errorCode_)> processSDO;
	std::atomic<bool> segmentedReadOngoing = false;

	static constexpr uint8_t maxDevices = 31;
	static constexpr uint32_t defaultSdoTimeout = 20;
	static constexpr uint32_t maxSingleFieldSize = 100;
	static constexpr uint32_t maxPdoNum = 4;

	std::unordered_map<uint32_t, IODParser::ODType*> ODmap;
	std::unordered_map<uint32_t, uint8_t> idToChannelMap;

	static constexpr uint16_t TPDOComunicationParamIndex = 0x1800;
	static constexpr uint16_t TPDOMappingParamIndex = 0x1A00;

	static constexpr uint16_t RPDOComunicationParamIndex = 0x1400;
	static constexpr uint16_t RPDOMappingParamIndex = 0x1600;

	/**
	 * @brief write SDO with a buffer of bytes.
	 *
	 * @param id
	 * @param index_ OD index.
	 * @param subindex_ OD subindex.
	 * @param dataIn std::vector of uint8_ts.
	 * @param size size of the vector.
	 * @param errorCode SDO CiA301 error code.
	 * @param channel used for reading on specific Candle channel.
	 * @return true
	 * @return false
	 */
	bool writeSdoBytes(uint32_t id, uint16_t index_, uint8_t subindex_, const std::vector<uint8_t>& dataIn, uint32_t size, uint32_t& errorCode, uint8_t channel = 0);

	/**
	 * @brief Fills in the OD based on received TPDO from the MD80.
	 *
	 * @param frame CAN frame containing TPDO data.
	 * @return true
	 * @return false
	 */
	bool fillODBasedOnTPDO(const ICommunication::CANFrame& frame);

	/**
	 * @brief Prepares RPDO-based CAN frame to be sent.
	 *
	 * @param OD pointer to OD.
	 * @param offset PDO offset (1-4).
	 * @return ICommunication::CANFrame.
	 */
	ICommunication::CANFrame prepareRPDO(IODParser::ODType* OD, uint8_t offset);

	/**
	 * @brief Get PDO Type (either RPDO or TPDO) based on its id.
	 *
	 * @param pdoId PDO ID.
	 * @return PDOType \ref PDOType.
	 */
	PDOType getPDOType(PDO pdoId);

	/**
	 * @brief Sends CAN frame and waits for compeltion.
	 *
	 * @param frame CAN frame to be sent.
	 * @param conditionVar std::atomic<> reference which controls the sending state.
	 * @param timeoutMs timeout in milliseconds.
	 * @return true
	 * @return false
	 */
	bool sendFrameWaitForCompletion(const ICommunication::CANFrame& frame, std::atomic<bool>& conditionVar, uint32_t timeoutMs);

	/**
	 * @brief Wait for a specific function to complete with timeout.
	 *
	 * @param condition std::function returning bool. While it returns false, the function is blocking.
	 * @param timeoutMs timeout in milliseconds.
	 * @return true
	 * @return false
	 */
	bool waitForActionWithTimeout(std::function<bool()> condition, uint32_t timeoutMs);
};

#endif
