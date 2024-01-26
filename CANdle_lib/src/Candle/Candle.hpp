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
		CYCLIC_SYNC_VELOCTIY = 9,
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

	void setSendSync(bool state, uint32_t intervalUs)
	{
		sendSync = state;
		syncIntervalUs = intervalUs;
	}

	std::vector<uint32_t> ping()
	{
		std::vector<uint32_t> ids{};
		uint32_t deviceType = 0;

		for (size_t i = 1; i < 31; i++)
			if (readSDO(i, 0x1000, 0x00, deviceType, false))
				ids.push_back(i);

		return ids;
	}

	bool addMd80(uint32_t id)
	{
		uint32_t deviceType = 0;
		if (!readSDO(id, 0x1000, 0x00, deviceType, false))
		{
			logger->error("Unable to add MD80 with ID {}", id);
			return false;
		}

		md80s[id] = std::make_unique<MD80>();
		canopenStack->setOD(id, &md80s[id]->OD);

		return true;
	}

	MD80* getMd80(uint32_t id) const
	{
		return md80s.at(id).get();
	}

	bool enterOperational(uint32_t id)
	{
		return writeSDO(id, 0x6040, 0x00, static_cast<uint16_t>(0x0080)) &&
			   writeSDO(id, 0x6040, 0x00, static_cast<uint16_t>(0x0006)) &&
			   writeSDO(id, 0x6040, 0x00, static_cast<uint16_t>(0x000f));
	}

	bool enterSwitchOnDisabled(uint32_t id)
	{
		return writeSDO(id, 0x6040, 0x00, static_cast<uint16_t>(0x0008));
	}

	bool setModeOfOperation(uint32_t id, ModesOfOperation mode)
	{
		return writeSDO(id, 0x6060, 0x00, static_cast<int8_t>(mode));
	}

	bool setTargetPosition(uint32_t id, uint32_t target)
	{
		return writeSDO(id, 0x607A, 0x00, target);
	}

	bool startCalibration(uint32_t id)
	{
		return enterOperational(id) &&
			   writeSDO(id, 0x6060, 0x00, static_cast<int8_t>(-2)) &&
			   writeSDO(id, 0x2003, 0x03, static_cast<uint8_t>(1));
	}

	bool setupResponse(uint32_t id, CanopenStack::PDO pdoID, std::vector<std::pair<uint16_t, uint8_t>>& fields)
	{
		return setupResponse(id, pdoID, std::move(fields));
	}

	bool setupResponse(uint32_t id, CanopenStack::PDO pdoID, std::vector<std::pair<uint16_t, uint8_t>>&& fields)
	{
		return canopenStack->setupPDO(id, pdoID, fields);
	}

	bool setupCommand(uint32_t id, CanopenStack::PDO pdoID, std::vector<std::pair<uint16_t, uint8_t>>& fields)
	{
		return setupCommand(id, pdoID, std::move(fields));
	}

	bool setupCommand(uint32_t id, CanopenStack::PDO pdoID, std::vector<std::pair<uint16_t, uint8_t>>&& fields)
	{
		return canopenStack->setupPDO(id, pdoID, fields);
	}

	template <typename T>
	bool writeSDO(uint32_t id, uint16_t index_, uint8_t subindex_, const T&& value)
	{
		uint32_t errorCode = 0;
		bool result = canopenStack->writeSDO(id, index_, subindex_, value, errorCode);

		if (errorCode)
		{
			logger->error("SDO write error! Code {:x}", errorCode);
			return false;
		}

		return result;
	}

	template <typename T>
	bool writeSDO(uint32_t id, uint16_t index_, uint8_t subindex_, const T& value)
	{
		return writeSDO(id, index_, subindex_, std::move(value));
	}

	template <typename T>
	bool readSDO(uint32_t id, uint16_t index_, uint8_t subindex_, T& value, bool checkOD = true)
	{
		uint32_t errorCode = 0;
		bool result = canopenStack->readSDO(id, index_, subindex_, value, errorCode, checkOD);

		if (errorCode)
		{
			logger->error("SDO read error! Error code: 0x{:x}", errorCode);
			return false;
		}

		return result;
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
			/* SEND RPDOs */
			canopenStack->sendRPDOs();

			if (sendSync)
				canopenStack->sendSYNC();

			auto end_time = std::chrono::high_resolution_clock::now() + std::chrono::microseconds(syncIntervalUs);
			while (std::chrono::high_resolution_clock::now() < end_time)
			{
			}
		}
	}

   public:
	std::unique_ptr<CanopenStack> canopenStack;

   private:
	std::thread receiveThread;
	std::thread transmitThread;

	std::atomic<bool> done = false;
	std::atomic<bool> sendSync = false;

	std::atomic<uint32_t> syncIntervalUs = 10000;

	std::unordered_map<uint32_t, std::unique_ptr<MD80>> md80s;
	ICommunication* interface;
	spdlog::logger* logger;
};

#endif