#ifndef CANDLE_HPP
#define CANDLE_HPP

#include <memory>
#include <vector>

#include "CanopenStack/CANopen/CanopenStack.hpp"
#include "ICommunication.hpp"
#include "MD80.hpp"

class Candle
{
   public:
	Candle(ICommunication* interface)
	{
		canopenStack = std::make_unique<CanopenStack>(interface);
	}

	std::vector<uint32_t> ping()
	{
		std::vector<uint32_t> ids{};

		for (size_t i = 1; i < 10; i++)
		{
			IODParser::ValueType deviceType = 0;
			if (canopenStack->readSDO(i, 0x1000, 0x00, deviceType))
				ids.push_back(i);
		}

		return ids;
	}

   private:
	std::vector<MD80> md80s;
	std::unique_ptr<CanopenStack> canopenStack;
};

#endif