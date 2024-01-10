#ifndef CANDLE_HPP
#define CANDLE_HPP

#include <iostream>
#include <memory>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

#include "CanopenStack/CANopen/CanopenStack.hpp"
#include "ICommunication.hpp"
#include "MD80.hpp"

class Candle
{
   public:
	enum class Baud : uint32_t
	{
		BAUD_1M = 1,
		BAUD_8M = 8,
	};

	enum class ModesOfOperation : int8_t
	{
		SERVICE = -2,
		IDLE = 0,
		PROFILE_POSITION = 1,
		PROFILE_VELOCITY = 2,
		CYCLIC_SYNC_POSITION = 8,
		CYCLIC_SYNCH_VELOCTIY = 9,
	};

	enum class RPDO : uint16_t
	{
		RPDO1 = 0x200,
		RPDO2 = 0x300,
		RPDO3 = 0x400,
		RPDO4 = 0x500
	};

	enum class TPDO : uint16_t
	{
		TPDO1 = 0x180,
		TPDO2 = 0x280,
		TPDO3 = 0x380,
		TPDO4 = 0x480
	};

	explicit Candle(ICommunication* interface, spdlog::logger* logger) : interface(interface),
																		 logger(logger)
	{
		canopenStack = std::make_unique<CanopenStack>(interface, logger);
		receiveThread = std::thread(&Candle::receiveHandler, this);
		transmitThread = std::thread(&Candle::transmitHandler, this);
	}

	~Candle()
	{
		deInit();
	}

	bool init(Baud baud = Baud::BAUD_1M)
	{
		ICommunication::Settings settings;

		if (baud == Baud::BAUD_8M)
		{
			settings.baudrate = 8000000;
			settings.bitRateSwitch = 0x00100000U;
			settings.fdFormat = 0x00200000U;
		}
		else
		{
			settings.baudrate = 1000000;
			settings.bitRateSwitch = 0;
			settings.fdFormat = 0;
		}

		return interface->setupInterface(settings);
	}

	void deInit()
	{
		done = true;
		if (receiveThread.joinable())
			receiveThread.join();
		if (transmitThread.joinable())
			transmitThread.join();
	}

	void setSendSync(bool state)
	{
		sendSync = state;
	}

	std::vector<uint32_t> ping()
	{
		std::vector<uint32_t> ids{};

		for (size_t i = 1; i < 10; i++)
		{
			uint32_t deviceType = 0;
			uint32_t errorCode = 0;

			if (canopenStack->readSDO(i, 0x1000, 0x00, deviceType, errorCode))
				ids.push_back(i);
		}

		return ids;
	}

	void addMd80(uint32_t id)
	{
		md80s[id] = std::make_unique<MD80>();
		canopenStack->setOD(md80s[id]->OD);
	}

	MD80* getMd80(uint32_t id) const
	{
		return md80s.at(id).get();
	}

	bool enterOperational(uint32_t id)
	{
		uint32_t errorCode = 0;
		return canopenStack->writeSDO(id, 0x6040, 0x00, static_cast<uint16_t>(0x0080), errorCode) &&
			   canopenStack->writeSDO(id, 0x6040, 0x00, static_cast<uint16_t>(0x0006), errorCode) &&
			   canopenStack->writeSDO(id, 0x6040, 0x00, static_cast<uint16_t>(0x000f), errorCode);
	}

	bool enterSwitchOnDisabled(uint32_t id)
	{
		uint32_t errorCode = 0;
		return canopenStack->writeSDO(id, 0x6040, 0x00, static_cast<uint16_t>(0x0008), errorCode);
	}

	bool setModeOfOperation(uint32_t id, ModesOfOperation mode)
	{
		uint32_t errorCode = 0;
		return canopenStack->writeSDO(id, 0x6060, 0x00, static_cast<int8_t>(mode), errorCode);
	}

	bool setTargetPosition(uint32_t id, uint32_t target)
	{
		uint32_t errorCode = 0;
		return canopenStack->writeSDO(id, 0x607A, 0x00, std::move(target), errorCode);
	}

	bool startCalibration(uint32_t id)
	{
		uint32_t errorCode = 0;
		return enterOperational(id) &&
			   canopenStack->writeSDO(id, 0x6060, 0x00, static_cast<int8_t>(-2), errorCode) &&
			   canopenStack->writeSDO(id, 0x2003, 0x03, static_cast<uint8_t>(1), errorCode);
	}

	bool setupResponse(uint32_t id, TPDO tpdoID, std::vector<std::pair<uint16_t, uint8_t>>& fields)
	{
		uint32_t errorCode = 0;
		uint32_t COBID = static_cast<uint32_t>(tpdoID) + id;

		/*  disable PDO (set 31 bit to 1)*/
		if (!canopenStack->writeSDO(id, 0x1800, 0x01, static_cast<uint32_t>(0x80000000 | COBID), errorCode))
			return false;

		/* set transsmission type to synch 1*/
		if (!canopenStack->writeSDO(id, 0x1800, 0x02, static_cast<uint8_t>(1), errorCode))
			return false;

		/* set PDO mapping objects count to zero */
		if (!canopenStack->writeSDO(id, 0x1A00, 0x00, static_cast<uint8_t>(0), errorCode))
			return false;

		uint8_t mapRegsubidx = 0;
		for (auto& [idx, subidx] : fields)
		{
			mapRegsubidx++;

			auto entry = checkEntryExists(md80s[id].get()->OD, idx, subidx);

			if (!entry.has_value())
				return false;

			entry.value()->value = getTypeBasedOnTag(entry.value()->dataType);

			uint8_t currentlyHeldFieldSize = std::visit([](const auto& value) -> uint8_t
														{ return sizeof(std::decay_t<decltype(value)>); },
														entry.value()->value);

			/* size by 8 to get size in bits */
			uint32_t mappedObject = idx << 16 | subidx << 8 | (currentlyHeldFieldSize * 8);

			/* set PDO mapping objects count to zero */
			if (!canopenStack->writeSDO(id, 0x1A00, mapRegsubidx, std::move(mappedObject), errorCode))
				return false;
		}

		/* set PDO mapping objects count to zero */
		if (!canopenStack->writeSDO(id, 0x1A00, 0x00, static_cast<uint8_t>(mapRegsubidx), errorCode))
			return false;

		/*  enable PDO (set 31 bit to 0)*/
		if (!canopenStack->writeSDO(id, 0x1800, 0x01, std::move(COBID), errorCode))
			return false;

		return true;
	}

   private:
	void receiveHandler()
	{
		while (!done)
		{
			auto maybeFrame = interface->receiveCanFrame();

			if (!maybeFrame.has_value())
				continue;

			canopenStack->parse(maybeFrame.value());
		}
	}

	void transmitHandler()
	{
		while (!done)
		{
			if (sendSync)
				canopenStack->sendSYNC();
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}
	}

	std::optional<IODParser::Entry*> checkEntryExists(IODParser::ODType& OD, uint16_t index, uint8_t subindex)
	{
		if (!OD.contains(index))
		{
			logger->error("Entry index not found in OD (0x{:x})", index);
			return std::nullopt;
		}
		else if (OD.contains(index) && OD.at(index)->objectType == IODParser::ObjectType::VAR)
			return OD.at(index).get();

		if (!OD.at(index)->subEntries.contains(subindex))
		{
			logger->error("Entry subindex not found in OD (0x{:x}:0x{:x})", index, subindex);
			return std::nullopt;
		}

		return OD.at(index)->subEntries.at(subindex).get();
	}

	IODParser::ValueType getTypeBasedOnTag(IODParser::DataType tag)
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

   public:
	std::unique_ptr<CanopenStack> canopenStack;

   private:
	std::thread receiveThread;
	std::thread transmitThread;

	std::atomic<bool> done = false;
	std::atomic<bool> sendSync = false;

	std::unordered_map<uint32_t, std::unique_ptr<MD80>> md80s;
	ICommunication* interface;
	spdlog::logger* logger;
};

#endif