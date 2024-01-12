#include "Candle.hpp"
#include "CandleInterface.hpp"
#include "UsbHandler.hpp"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

int main(int argc, char** argv)
{
	auto logger = spdlog::stdout_color_mt("console");
	logger->set_pattern("[%^%l%$] %v");

	UsbHandler busHandler(logger.get());
	CandleInterface candleInterface(&busHandler);
	Candle candle(&candleInterface, logger.get());

	candle.init();

	candle.addMd80(1);
	candle.addMd80(2);

	std::vector<std::pair<uint16_t, uint8_t>> fields{{0x2009, 0x01}, {0x2009, 0x02}};

	candle.setupResponse(1, CanopenStack::TPDO::TPDO1, {{0x2009, 0x01}, {0x2009, 0x02}});
	candle.setupResponse(2, CanopenStack::TPDO::TPDO1, fields);

	auto md80 = candle.getMd80(2);

	candle.setSendSync(true, 1000);

	while (1)
	{
		std::cout << md80->getOutputPosition() << "   " << md80->getOutputVelocity() << std::endl;
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	return 0;
}
