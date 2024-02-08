#ifndef CANDLE_HPP
#define CANDLE_HPP

#include <iostream>
#include <memory>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

#include "Barrier.hpp"
#include "BusHandler/UsbHandler.hpp"
#include "CanopenStack/CANopen/CanopenStack.hpp"
#include "Communication/CandleInterface.hpp"
#include "ICommunication.hpp"
#include "MD80.hpp"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

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
		IMPEDANCE = -3,
		SERVICE = -2,
		IDLE = 0,
		PROFILE_POSITION = 1,
		PROFILE_VELOCITY = 2,
		CYCLIC_SYNC_POSITION = 8,
		CYCLIC_SYNC_VELOCTIY = 9,
	};

	Candle();
	Candle(std::shared_ptr<ICommunication> interface, std::shared_ptr<spdlog::logger> logger);
	~Candle();

	bool init(Baud baud = Baud::BAUD_1M);
	void deInit();
	void setSendSync(bool state, uint32_t intervalUs);
	std::vector<uint32_t> ping();
	bool addMd80(uint32_t id);
	std::shared_ptr<MD80> getMd80(uint32_t id) const;
	bool enterOperational(uint32_t id);
	bool enterSwitchOnDisabled(uint32_t id);
	bool setModeOfOperation(uint32_t id, ModesOfOperation mode);
	bool setTargetPosition(uint32_t id, uint32_t target);
	bool startCalibration(uint32_t id);

	bool setupResponse(uint32_t id, CanopenStack::PDO pdoID, const std::vector<std::pair<uint16_t, uint8_t>>& fields);
	bool setupCommand(uint32_t id, CanopenStack::PDO pdoID, const std::vector<std::pair<uint16_t, uint8_t>>& fields);

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
	void receiveHandler();
	void transmitHandler();

   public:
	std::unique_ptr<CanopenStack> canopenStack;

   private:
	std::thread receiveThread;
	std::thread transmitThread;
	Barrier syncPoint;

	std::atomic<bool> done = false;
	std::atomic<bool> sendSync = false;

	std::atomic<uint32_t> syncIntervalUs = 10000;

	bool isInitialized = false;

	std::unordered_map<uint32_t, std::shared_ptr<MD80>> md80s;
	std::shared_ptr<ICommunication> interface;
	std::shared_ptr<spdlog::logger> logger;
};

#endif