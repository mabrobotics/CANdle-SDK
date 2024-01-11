#include "Candle.hpp"
#include "CandleInterface.hpp"
#include "UsbHandler.hpp"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

int main(int argc, char** argv)
{
	auto logger = spdlog::stdout_color_mt("console");
	logger->set_pattern("[%^%l%$] %v");

	std::unique_ptr<IBusHandler> busHandler = std::make_unique<UsbHandler>(logger.get());
	std::unique_ptr<ICommunication> candleInterface = std::make_unique<CandleInterface>(busHandler.get());

	Candle candle(candleInterface.get(), logger.get());

	candle.init();

	candle.addMd80(1);
	candle.addMd80(2);

	std::vector<std::pair<uint16_t, uint8_t>> fields{{0x2009, 0x01}, {0x2009, 0x02}};

	candle.setupResponse(1, CanopenStack::TPDO::TPDO1, fields);
	candle.setupResponse(2, CanopenStack::TPDO::TPDO1, fields);

	auto md80 = candle.getMd80(2);

	candle.setSendSync(true);

	while (1)
	{
		float position = std::get<float>(md80->OD.at(0x2009)->subEntries.at(0x01)->value);
		float velocity = std::get<float>(md80->OD.at(0x2009)->subEntries.at(0x02)->value);

		std::cout << position << "   " << velocity << std::endl;
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	return 0;
}
