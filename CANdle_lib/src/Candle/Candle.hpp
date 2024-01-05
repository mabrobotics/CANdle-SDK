#ifndef CANDLE_HPP
#define CANDLE_HPP

#include <iostream>
#include <memory>
#include <unordered_map>
#include <utility>
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

	explicit Candle(ICommunication* interface) : interface(interface)
	{
		canopenStack = std::make_unique<CanopenStack>(interface);
		receiveThread = std::thread(&Candle::receiveHandler, this);
		transmitThread = std::thread(&Candle::transmitHandler, this);
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
		std::cout << "deinitializing Candle module..." << std::endl;
		done = true;
		if (receiveThread.joinable())
			receiveThread.join();
		if (transmitThread.joinable())
			transmitThread.join();
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
			canopenStack->sendSYNC();
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
	}

   public:
	std::unique_ptr<CanopenStack> canopenStack;

   private:
	std::thread receiveThread;
	std::thread transmitThread;

	std::atomic<bool> done = false;

	std::unordered_map<uint32_t, std::unique_ptr<MD80>> md80s;
	ICommunication* interface;
};

#endif