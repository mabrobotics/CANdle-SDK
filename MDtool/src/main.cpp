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
		auto status = BinaryParser::processFile(filePath, logger);

		// std::vector<std::pair<uint32_t, Downloader::Status>> ids = {};

		// if (all)
		// {
		// 	logger->info("Pinging drives at baudrate {}M...", static_cast<uint32_t>(baudrate));

		// 	for (auto id : candle->ping())
		// 		ids.push_back({id, Downloader::Status::OK});

		// 	if (ids.empty())
		// 		logger->info("No drives found!");
		// 	else
		// 		logger->info("Found {} drives!", ids.size());
		// }
		// else
		// 	ids.push_back({id, Downloader::Status::OK});

		// candle->deInit();
		// Downloader downloader(interface, logger);

		// for (auto& [id, status] : ids)
		// {
		// 	logger->info("Preparing to update drive ID {} at baudrate {}M...", id, static_cast<uint32_t>(baudrate));
		// 	status = downloader.doLoad(std::span<uint8_t>(buffer), id, recover, 0, false);

		// 	if (status != Downloader::Status::OK)
		// 		logger->error("Status: {}", static_cast<uint8_t>(status));
		// }

		// auto updateOk = std::count_if(ids.begin(), ids.end(),
		// 							  [](const auto& md80)
		// 							  { return md80.second == Downloader::Status::OK; });

		// logger->info("Update successful for {} drive(s)", updateOk);

		// for (auto md80 : ids)
		// {
		// 	if (md80.second == Downloader::Status::OK)
		// 		logger->info("ID {}", md80.first);
		// }

		// logger->info("Update failed for {} drive(s) ", ids.size() - updateOk);

		// for (auto& [id, status] : ids)
		// {
		// 	if (status != Downloader::Status::OK)
		// 		logger->error("ID {} with status {}", id, static_cast<uint8_t>(status));
		// }

		return true;
	}

   private:
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

	auto logger = spdlog::stdout_color_mt("console");
	logger->set_pattern("[%^%l%$] %v");

	uint32_t baud = 1;
	ping->add_option("-b,--baud", baud, "CAN Baudrate in Mbps used to setup CANdle");
	updateMD80->add_option("-b,--baud", baud, "CAN Baudrate in Mbps used to setup CANdle");

	bool all = false;
	auto* all_option = updateMD80->add_flag("-a,--all", all, "Use to update all drives detected by ping() method");

	uint32_t id = 1;
	updateMD80->add_option("-i,--id", id, "ID of the drive")->check(CLI::Range(1, 31))->excludes(all_option);

	std::string filePath;
	updateMD80->add_option("-f,--file", filePath, "Update filename");

	bool recover = false;
	updateMD80->add_flag("-r,--recover", recover, "Use if the MD80 is already in bootloader mode");

	bool verbose = false;
	app.add_flag("-v,--verbose", verbose, "Use for verbose mode");

	CLI11_PARSE(app, argc, argv);

	if (verbose)
		logger->set_level(spdlog::level::debug);

	Mdtool mdtool;
	std::unique_ptr<IBusHandler> busHandler = std::make_unique<UsbHandler>();
	std::unique_ptr<ICommunication> candleInterface = std::make_unique<CandleInterface>(busHandler.get());

	mdtool.init(candleInterface.get(), logger.get(), static_cast<Candle::Baud>(baud));

	if (app.got_subcommand("ping"))
		mdtool.ping();

	else if (app.got_subcommand("update_md80"))
		mdtool.updateMd80(filePath, id, recover, all);

	return 0;
}
