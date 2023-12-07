#ifndef CANDLE_HPP
#define CANDLE_HPP

#include <memory>
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

	explicit Candle(ICommunication* interface) : interface(interface)
	{
		canopenStack = std::make_unique<CanopenStack>(interface);
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
		canopenStack.reset();
	}

	std::vector<uint32_t> ping()
	{
		std::vector<uint32_t> ids{};

		for (size_t i = 1; i < 10; i++)
		{
			uint32_t deviceType = 0;
			if (canopenStack->readSDO(i, 0x1000, 0x00, deviceType))
				ids.push_back(i);
		}

		return ids;
	}

   public:
	std::unique_ptr<CanopenStack> canopenStack;

   private:
	std::vector<MD80> md80s;
	ICommunication* interface;
};

#endif