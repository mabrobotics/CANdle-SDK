#include <iostream>
#include <span>

#include "BinaryParser.hpp"
#include "CLI11.hpp"
#include "Candle.hpp"
#include "CandleInterface.hpp"
#include "Downloader.hpp"
#include "UsbHandler.hpp"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

class Mdtool
{
   public:
	~Mdtool()
	{
		std::cout << "deinitializing MDtool..." << std::endl;
		candle->deInit();
	}

	bool init(ICommunication* interface, spdlog::logger* logger, Candle::Baud baud)
	{
		logger->info("Initalizing...");
		this->logger = logger;
		this->interface = interface;
		this->baudrate = baud;
		candle = std::make_unique<Candle>(interface);
		return candle->init(baud);
	}

	void ping()
	{
		auto drives = candle->ping();
		logger->info("Found drives: ");

		for (auto& md80 : drives)
			logger->info(std::to_string(md80));
	}

	bool updateMd80(std::string& filePath, uint32_t id, bool recover, bool all)
	{
		auto status = BinaryParser::processFile(filePath);

		if (status == BinaryParser::Status::ERROR_FILE)
		{
			logger->error("Error opening file: {}", filePath);
			return false;
		}
		else if (status != BinaryParser::Status::OK)
		{
			logger->error("Error while parsing firmware file! Error code: {}", static_cast<std::underlying_type<BinaryParser::Status>::type>(status));
			return false;
		}

		if (BinaryParser::getFirmwareFileType() != BinaryParser::Type::MD80)
		{
			logger->error("Wrong file type! Please make sure the file is intended for MD80 controller.");
			return false;
		}

		std::vector<std::pair<uint32_t, Downloader::Status>> ids = {};

		if (all)
		{
			logger->info("Pinging drives at baudrate {}M...", static_cast<uint32_t>(baudrate));

			for (auto id : candle->ping())
				ids.push_back({id, Downloader::Status::OK});

			if (ids.empty())
				logger->info("No drives found!");
			else
				logger->info("Found {} drives!", ids.size());
		}
		else
			ids.push_back({id, Downloader::Status::OK});

		candle->deInit();
		Downloader downloader(interface, logger);

		for (auto& [id, status] : ids)
		{
			logger->info("Preparing to update drive ID {} at baudrate {}M...", id, static_cast<uint32_t>(baudrate));
			auto buffer = BinaryParser::getPrimaryFirmwareFile();
			status = downloader.doLoad(std::span<uint8_t>(buffer), id, recover, 0, false);

			if (status != Downloader::Status::OK)
				logger->error("Status: {}", static_cast<uint8_t>(status));
		}

		auto updateOk = std::count_if(ids.begin(), ids.end(),
									  [](const auto& md80)
									  { return md80.second == Downloader::Status::OK; });

		logger->info("Update successful for {} drive(s)", updateOk);

		for (auto md80 : ids)
		{
			if (md80.second == Downloader::Status::OK)
				logger->info("ID {}", md80.first);
		}

		logger->info("Update failed for {} drive(s) ", ids.size() - updateOk);

		for (auto& [id, status] : ids)
		{
			if (status != Downloader::Status::OK)
				logger->error("ID {} with status {}", id, static_cast<uint8_t>(status));
		}

		return true;
	}

	bool updateBootloader(std::string& filePath, uint32_t id, bool recover)
	{
		logger->warn("You're about to update the MD80 bootloader. Ensure proper power supply. Disconnecting the power mid-update can brick the MD80! Please type \"iamaware\" to continue:");

		std::string safetyCode;
		std::cin >> safetyCode;

		if (safetyCode != "iamaware")
		{
			logger->error("Wrong passphrase. Exiting...");
			return false;
		}

		auto status = BinaryParser::processFile(filePath);

		if (status == BinaryParser::Status::ERROR_FILE)
		{
			logger->error("Error opening file: {}", filePath);
			return false;
		}
		else if (status != BinaryParser::Status::OK)
		{
			logger->error("Error while parsing firmware file! Error code: {}", static_cast<std::underlying_type<BinaryParser::Status>::type>(status));
			return false;
		}

		if (BinaryParser::getFirmwareFileType() != BinaryParser::Type::BOOT)
		{
			logger->error("Wrong file type! Please make sure the file is intended for MD80 controller bootlaoder.");
			return false;
		}

		candle->deInit();
		Downloader downloader(interface, logger);

		logger->info("Preparing to update the secondary bootloader of drive ID {}", id);
		auto buffer = BinaryParser::getSecondaryFirmwareFile();
		auto updateStatus = downloader.doLoad(std::span<uint8_t>(buffer), id, recover, secondaryBootloaderAddress, false);

		if (updateStatus != Downloader::Status::OK)
			logger->error("Status: {}", static_cast<uint8_t>(updateStatus));

		logger->info("Preparing to update the primary bootloader of drive ID {}", id);
		buffer = BinaryParser::getPrimaryFirmwareFile();
		updateStatus = downloader.doLoad(std::span<uint8_t>(buffer), id, recover, primaryBootloaderAddress, true);

		if (updateStatus != Downloader::Status::OK)
			logger->error("Status: {}", static_cast<uint8_t>(updateStatus));

		return true;
	}

	bool readSDO(uint32_t id, uint32_t index, uint32_t subindex)
	{
		candle->addMd80(id);

		// auto value = candle->getMd80(id)->OD.at(index)->subEntries.at(subindex)->value;

		uint32_t value = 0;

		if (!candle->canopenStack->readSDO(id, index, subindex, value))
			return false;

		logger->info("SDO value: {}", value);
	}

   private:
	static constexpr uint32_t secondaryBootloaderAddress = 0x8005000;
	static constexpr uint32_t primaryBootloaderAddress = 0x8000000;
	std::unique_ptr<Candle> candle;
	ICommunication* interface;
	spdlog::logger* logger;

	Candle::Baud baudrate = Candle::Baud::BAUD_1M;
};

int main(int argc, char** argv)
{
	CLI::App app{"MDtool"};
	auto* ping = app.add_subcommand("ping", "Discovers all drives connected to CANdle");
	auto* updateMD80 = app.add_subcommand("update_md80", "Use to update MD80");
	auto* updateCANdle = app.add_subcommand("update_candle", "Use to update CANdle");
	auto* updateBootloader = app.add_subcommand("update_bootloader", "Use to update MD80 bootloader");

	auto* readSDO = app.add_subcommand("readSDO", "Use to read SDO value");

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
	readSDO->add_option("--idx", index, "SDO index")->required();
	readSDO->add_option("--subidx", subindex, "SDO subindex")->required();

	CLI11_PARSE(app, argc, argv);

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

	return 0;
}
