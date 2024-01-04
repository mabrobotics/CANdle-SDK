#include <iostream>
#include <span>

#include "BinaryParser.hpp"
#include "CLI11.hpp"
#include "Candle.hpp"
#include "CandleInterface.hpp"
#include "Downloader.hpp"
#include "UsbHandler.hpp"
#include "mdtool.hpp"
#include "spdlog/fmt/ostr.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

int main(int argc, char** argv)
{
	CLI::App app{"MDtool"};
	auto* ping = app.add_subcommand("ping", "Discovers all drives connected to CANdle");
	auto* updateMD80 = app.add_subcommand("update_md80", "Use to update MD80");
	auto* updateCANdle = app.add_subcommand("update_candle", "Use to update CANdle");
	auto* updateBootloader = app.add_subcommand("update_bootloader", "Use to update MD80 bootloader");

	auto* readSDO = app.add_subcommand("readSDO", "Use to read SDO value");
	auto* writeSDO = app.add_subcommand("writeSDO", "Use to write SDO value");

	auto logger = spdlog::stdout_color_mt("console");
	logger->set_pattern("[%^%l%$] %v");

	uint32_t baud = 1;
	app.add_option("-b,--baud", baud, "CAN Baudrate in Mbps used to setup CANdle");

	bool all = false;
	auto* all_option = updateMD80->add_flag("-a,--all", all, "Use to update all drives detected by ping() method");

	uint32_t id = 1;
	app.add_option("-i,--id", id, "ID of the drive")->check(CLI::Range(1, 31))->excludes(all_option);

	std::string filePath;
	updateMD80->add_option("-f,--file", filePath, "Update filename")->required();
	updateBootloader->add_option("-f,--file", filePath, "Update filename")->required();

	bool recover = false;
	updateMD80->add_flag("-r,--recover", recover, "Use if the MD80 is already in bootloader mode");

	bool verbose = false;
	app.add_flag("-v,--verbose", verbose, "Use for verbose mode");

	uint32_t index = 0;
	uint32_t subindex = 0;
	readSDO->add_option("idx", index, "SDO index")->required();
	readSDO->add_option("subidx", subindex, "SDO subindex")->required();

	IODParser::ValueType value;
	std::string valueStr{};
	std::string valueType{};
	writeSDO->add_option("idx", index, "SDO index")->required();
	writeSDO->add_option("subidx", subindex, "SDO subindex")->required();
	writeSDO->add_option("value", valueStr, "SDO value to be written")->required();
	writeSDO->add_option("type", valueType, "SDO value type to be written")->required();

	CLI11_PARSE(app, argc, argv);

	if (writeSDO->count("type") > 0)
	{
		if (valueType == "f32")
			value = std::stof(valueStr);
		else if (valueType == "i8")
			value = static_cast<int8_t>(std::stoi(valueStr, nullptr, 0));
		else if (valueType == "u16")
			value = static_cast<uint16_t>(std::stoi(valueStr, nullptr, 0));
		else if (valueType == "i16")
			value = static_cast<int16_t>(std::stoi(valueStr, nullptr, 0));
		else if (valueType == "u32")
			value = static_cast<uint32_t>(std::stoul(valueStr, nullptr, 0));
		else if (valueType == "i32")
			value = static_cast<uint32_t>(std::stoi(valueStr, nullptr, 0));
		else if (valueType == "str")
		{
			std::array<uint8_t, 24> arr{};
			std::copy(valueStr.begin(), valueStr.end(), arr.begin());
			value = arr;
		}
		else
		{
			logger->error("Wrong --type argument!");
			return 0;
		}
	}

	if (verbose)
		logger->set_level(spdlog::level::debug);

	std::unique_ptr<IBusHandler> busHandler = std::make_unique<UsbHandler>();
	std::unique_ptr<ICommunication> candleInterface = std::make_unique<CandleInterface>(busHandler.get());

	Mdtool mdtool;
	if (!mdtool.init(candleInterface.get(), logger.get(), static_cast<Candle::Baud>(baud)))
		return false;

	if (app.got_subcommand("ping"))
		mdtool.ping();
	else if (app.got_subcommand("update_md80"))
		mdtool.updateMd80(filePath, id, recover, all);
	else if (app.got_subcommand("update_bootloader"))
		mdtool.updateBootloader(filePath, id, recover);
	else if (app.got_subcommand("readSDO"))
		mdtool.readSDO(id, index, subindex);
	else if (app.got_subcommand("writeSDO"))
		mdtool.writeSDO(id, index, subindex, value);

	return 0;
}
