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

	candle.addMd80(2);

	std::vector<std::pair<uint16_t, uint8_t>> fields;

	fields.push_back({0x6064, 0x00});
	fields.push_back({0x6064, 0x00});

	candle.setupResponse(2, Candle::TPDO::TPDO1, fields);

	auto md80 = candle.getMd80(2);

	candle.setSendSync(true);

	while (1)
	{
		int32_t position = std::get<int32_t>(md80->OD.at(0x6064)->value);
		std::cout << position << std::endl;
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	return 0;
}
