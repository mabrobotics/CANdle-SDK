#include "CLI11.hpp"
#include "Candle.hpp"
#include "CandleInterface.hpp"
#include "UsbHandler.hpp"
#include "mdtool.hpp"
#include "spdlog/fmt/ostr.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

int main(int argc, char** argv)
{
	CLI::App app{"MDtool"};
	app.fallthrough();
	app.add_subcommand("ping", "Discovers all drives connected to CANdle");
	auto* updateMD80 = app.add_subcommand("update_md80", "Use to update MD80");
	auto* updateCANdle = app.add_subcommand("update_candle", "Use to update CANdle");
	auto* updateBootloader = app.add_subcommand("update_bootloader", "Use to update MD80 bootloader");

	auto* readSDO = app.add_subcommand("readSDO", "Use to read SDO value");
	auto* writeSDO = app.add_subcommand("writeSDO", "Use to write SDO value");
	app.add_subcommand("calibrate", "Use to calibrate the motor");
	auto* save = app.add_subcommand("save", "Use to save the motor parameters");
	app.add_subcommand("status", "Use to read motor status");
	app.add_subcommand("home", "Use to run homing");
	app.add_subcommand("info", "Use list all registers");
	app.add_subcommand("reset", "Use to reset a drive");
	auto* changeID = app.add_subcommand("changeID", "Use to change ID");
	auto* changeBaud = app.add_subcommand("changeBaud", "Use to change baudrate");
	auto* setupMotor = app.add_subcommand("setup", "Use to setup a motor using the selected config file");

	auto* clear = app.add_subcommand("clear", "Use clear errors or warnings");
	clear->add_subcommand("error", "Use clear errors");
	clear->add_subcommand("warning", "Use clear warnings");

	app.add_subcommand("zero", "Use to set current position as zero");

	auto logger = spdlog::stdout_color_mt("console");
	logger->set_pattern("[%^%l%$] %v");

	uint32_t baud = 1;
	app.add_option("-b,--baud", baud, "CAN Baudrate in Mbps used to setup CANdle");

	bool all = false;
	auto* all_option = updateMD80->add_flag("-a,--all", all, "Use to update all drives detected by ping() method");
	save->add_flag("-a,--all", all, "Use to save config on all connected drives");

	uint32_t id = 1;
	app.add_option("-i,--id", id, "ID of the drive")->check(CLI::Range(1, 31))->excludes(all_option);

	std::string filePath;
	updateMD80->add_option("-f,--file", filePath, "MD80 update filepath (*.mab)")->required();
	updateBootloader->add_option("-f,--file", filePath, "MD80 bootloader update filepath (*.mab)")->required();
	updateCANdle->add_option("-f,--file", filePath, "CANdle update filepath (*.mab)")->required();
	setupMotor->add_option("-f,--file", filePath, "Motor config filepath (*.cfg)")->required();

	bool recover = false;
	updateMD80->add_flag("-r,--recover", recover, "Use if the MD80 is already in bootloader mode");
	updateCANdle->add_flag("-r,--recover", recover, "Use if CANdle is already in bootloader mode");

	bool verbose = false;
	app.add_flag("-v,--verbose", verbose, "Use for verbose mode");

	uint32_t newID = 0;
	changeID->add_option("newid", newID, "new ID")->required();

	uint32_t newBaud = 1;
	changeBaud->add_option("newBaud", newBaud, "new baud")->required();

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
		else if (valueType == "u8")
			value = static_cast<uint8_t>(std::stoi(valueStr, nullptr, 0));
		else if (valueType == "i8")
			value = static_cast<int8_t>(std::stoi(valueStr, nullptr, 0));
		else if (valueType == "u16")
			value = static_cast<uint16_t>(std::stoi(valueStr, nullptr, 0));
		else if (valueType == "i16")
			value = static_cast<int16_t>(std::stoi(valueStr, nullptr, 0));
		else if (valueType == "u32")
			value = static_cast<uint32_t>(std::stoul(valueStr, nullptr, 0));
		else if (valueType == "i32")
			value = static_cast<int32_t>(std::stoi(valueStr, nullptr, 0));
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

	std::shared_ptr<ICommunication> candleInterface = std::make_shared<CandleInterface>(std::make_unique<UsbHandler>(logger));

	Mdtool mdtool(logger);

	/* dont init candle when we're updating candle (we need to switch vid/pid dynamically) */
	if (app.got_subcommand("update_candle"))
	{
		if (mdtool.updateCANdle(filePath, recover))
			return 0;
		else
			return -1;
	}

	if (!mdtool.init(candleInterface, static_cast<Candle::Baud>(baud)))
		return -1;

	bool success = false;

	if (app.got_subcommand("ping"))
		success = mdtool.ping();
	else if (app.got_subcommand("update_md80"))
		success = mdtool.updateMd80(filePath, id, recover, all);
	else if (app.got_subcommand("update_bootloader"))
		success = mdtool.updateBootloader(filePath, id, recover);
	else if (app.got_subcommand("readSDO"))
		success = mdtool.readSDO(id, index, subindex);
	else if (app.got_subcommand("writeSDO"))
		success = mdtool.writeSDO(id, index, subindex, value);
	else if (app.got_subcommand("calibrate"))
		success = mdtool.calibrate(id);
	else if (app.got_subcommand("save"))
		success = mdtool.save(id, all);
	else if (app.got_subcommand("status"))
		success = mdtool.status(id);
	else if (app.got_subcommand("home"))
		success = mdtool.home(id);
	else if (app.got_subcommand("changeID"))
		success = mdtool.changeId(id, newID);
	else if (app.got_subcommand("changeBaud"))
		success = mdtool.changeBaud(id, newBaud * 1000000);
	else if (app.got_subcommand("clear"))
	{
		if (clear->got_subcommand("error"))
			success = mdtool.clearError(id);
		else if (clear->got_subcommand("warning"))
			success = mdtool.clearWarning(id);
	}
	else if (app.got_subcommand("info"))
		success = mdtool.setupInfo(id);
	else if (app.got_subcommand("zero"))
		success = mdtool.setZero(id);
	else if (app.got_subcommand("reset"))
		success = mdtool.reset(id);
	else if (app.got_subcommand("setup"))
		success = mdtool.setupMotor(id, filePath);

	if (success)
		logger->info("Success!");
	else
		logger->error("Command Failed!");

	return 0;
}
