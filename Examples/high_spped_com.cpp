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

	auto md80s = candle.ping();

	for (auto& elem : md80s)
		std::cout << elem << std::endl;

	std::vector<std::pair<uint16_t, uint8_t>> fields;

	fields.push_back({0x6064, 0x00});

	candle.setupResponse(md80s[1], Candle::TPDO::TPDO1, fields);

	return 0;
}
