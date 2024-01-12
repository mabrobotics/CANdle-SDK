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

	std::vector<std::pair<uint16_t, uint8_t>> TPDO{{0x2009, 0x01}, {0x2009, 0x02}};
	candle.setupResponse(1, CanopenStack::PDO::TPDO1, TPDO);
	candle.setupResponse(2, CanopenStack::PDO::TPDO1, TPDO);

	std::vector<std::pair<uint16_t, uint8_t>> RPDO{{0x2008, 0x09}, {0x2008, 0x0A}};
	candle.setupResponse(1, CanopenStack::PDO::RPDO1, RPDO);
	candle.setupResponse(2, CanopenStack::PDO::RPDO1, RPDO);

	auto md80 = candle.getMd80(2);

	candle.setSendSync(true, 2000);

	while (1)
	{
		md80->setPositionTarget(21.37f);
		std::cout << md80->getOutputPosition() << "   " << md80->getOutputVelocity() << std::endl;
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	return 0;
}
