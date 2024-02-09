#include "mdtool.hpp"

#include <array>
#include <utility>

#include "BinaryParser.hpp"
#include "CANdleDownloader.hpp"
#include "MD80Downloader.hpp"
#include "spdlog/fmt/ostr.h"

namespace fmt
{
template <typename T, std::size_t N>
struct formatter<std::array<T, N>> : formatter<std::string_view>
{
	template <typename FormatContext>
	auto format(const std::array<T, N>& arr, FormatContext& ctx)
	{
		std::string result = "";
		auto it = arr.begin();
		while (it != arr.end() && *it != 0)
		{
			result += (std::isprint(static_cast<unsigned char>(*it)) ? *it : '?');
			it++;
		}

		return formatter<std::string_view>::format(result, ctx);
	}
};
}  // namespace fmt

Mdtool::Mdtool(std::shared_ptr<spdlog::logger> logger) : logger(logger)
{
}

bool Mdtool::init(std::shared_ptr<ICommunication> interface, Candle::Baud baud)
{
	logger->debug("Initalizing...");
	this->interface = interface;
	this->baudrate = baud;
	candle = std::make_unique<Candle>(interface, logger);
	return candle->init(baud);
}

void Mdtool::ping()
{
	auto drives = candle->ping();
	logger->info("Found drives: ");

	for (auto& md80 : drives)
		logger->info(std::to_string(md80));
}

bool Mdtool::updateMd80(std::string& filePath, uint32_t id, bool recover, bool all)
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

	std::vector<std::pair<uint32_t, MD80Downloader::Status>> ids = {};

	if (all)
	{
		logger->info("Pinging drives at baudrate {}M...", static_cast<uint32_t>(baudrate));

		for (auto id : candle->ping())
			ids.push_back({id, MD80Downloader::Status::OK});

		if (ids.empty())
			logger->info("No drives found!");
		else
			logger->info("Found {} drives!", ids.size());
	}
	else
		ids.push_back({id, MD80Downloader::Status::OK});

	candle->deInit();
	MD80Downloader downloader(interface, logger);

	for (auto& [id, status] : ids)
	{
		logger->info("Preparing to update drive ID {} at baudrate {}M...", id, static_cast<uint32_t>(baudrate));
		auto buffer = BinaryParser::getPrimaryFirmwareFile();
		status = downloader.doLoad(std::span<uint8_t>(buffer), id, recover, 0, false);

		if (status != MD80Downloader::Status::OK)
			logger->error("Status: {}", static_cast<uint8_t>(status));
	}

	auto updateOk = std::count_if(ids.begin(), ids.end(),
								  [](const auto& md80)
								  { return md80.second == MD80Downloader::Status::OK; });

	logger->info("Update successful for {} drive(s)", updateOk);

	for (auto md80 : ids)
	{
		if (md80.second == MD80Downloader::Status::OK)
			logger->info("ID {}", md80.first);
	}

	logger->info("Update failed for {} drive(s) ", ids.size() - updateOk);

	for (auto& [id, status] : ids)
	{
		if (status != MD80Downloader::Status::OK)
			logger->error("ID {} with status {}", id, static_cast<uint8_t>(status));
	}

	return true;
}

bool Mdtool::updateBootloader(std::string& filePath, uint32_t id, bool recover)
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
		logger->error("Wrong file type! Please make sure the file is intended for MD80 controller bootloader.");
		return false;
	}

	candle->deInit();
	MD80Downloader downloader(interface, logger);

	logger->info("Preparing to update the secondary bootloader of drive ID {}", id);
	auto buffer = BinaryParser::getSecondaryFirmwareFile();
	auto updateStatus = downloader.doLoad(std::span<uint8_t>(buffer), id, recover, secondaryBootloaderAddress, false);

	if (updateStatus != MD80Downloader::Status::OK)
	{
		logger->error("Update failed! Status: {}", static_cast<uint8_t>(updateStatus));
		return false;
	}

	logger->info("Preparing to update the primary bootloader of drive ID {}", id);
	buffer = BinaryParser::getPrimaryFirmwareFile();
	updateStatus = downloader.doLoad(std::span<uint8_t>(buffer), id, recover, primaryBootloaderAddress, true);

	if (updateStatus != MD80Downloader::Status::OK)
	{
		logger->error("Update failed! Status: {}", static_cast<uint8_t>(updateStatus));
		return false;
	}
	else
		logger->info("Bootloader update successful! Now please update MD80 firmware using \"update_md80\" command.");

	return true;
}

bool Mdtool::updateCANdle(std::string& filePath, bool recover)
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

	if (BinaryParser::getFirmwareFileType() != BinaryParser::Type::CANDLE)
	{
		logger->error("Wrong file type! Please make sure the file is intended for CANdle communication dongle.");
		return false;
	}

	CANdleDownloader downloader(logger);

	logger->info("Preparing to update CANdle device...");

	auto buffer = BinaryParser::getPrimaryFirmwareFile();
	auto downloadStatus = downloader.doLoad(std::span<uint8_t>(buffer), recover);

	if (downloadStatus != CANdleDownloader::Status::OK)
	{
		logger->error("Update failed! Status: {}", static_cast<uint8_t>(downloadStatus));
		return false;
	}
	else
		logger->info("Update successful!");

	return true;
}

bool Mdtool::readSDO(uint32_t id, uint16_t index, uint8_t subindex)
{
	candle->addMd80(id);
	auto md80 = candle->getMd80(id);

	auto maybeEntry = candle->canopenStack->checkEntryExists(&md80->OD, index, subindex);

	if (!maybeEntry.has_value())
		return false;

	auto& value = maybeEntry.value()->value;
	value = candle->canopenStack->getTypeBasedOnTag(maybeEntry.value()->dataType);

	auto lambdaFunc = [&](auto& arg)
	{
		if (!candle->readSDO(id, index, subindex, arg))
			return false;

		logger->info("SDO value: {}", arg);
		return true;
	};

	return std::visit(lambdaFunc, value);
}

bool Mdtool::writeSDO(uint32_t id, uint16_t index, uint8_t subindex, const IODParser::ValueType& value)
{
	candle->addMd80(id);

	auto lambdaFunc = [&](auto& arg) -> bool
	{
		if (!candle->writeSDO(id, index, subindex, std::move(arg)))
			return false;

		logger->info("Writing successful! 0x{:x}:0x{:x} = {}", index, subindex, arg);
		return true;
	};

	return std::visit(lambdaFunc, value);
}

bool Mdtool::calibrate(uint32_t id)
{
	return candle->addMd80(id) &&
		   candle->enterOperational(id) &&
		   candle->setModeOfOperation(id, Candle::ModesOfOperation::SERVICE) &&
		   candle->writeSDO(id, 0x2003, 0x03, true);
}

bool Mdtool::home(uint32_t id)
{
	return candle->addMd80(id) &&
		   candle->enterOperational(id) &&
		   candle->setModeOfOperation(id, Candle::ModesOfOperation::SERVICE) &&
		   candle->writeSDO(id, 0x2003, 0x0E, true);
}

bool Mdtool::save(uint32_t id)
{
	return candle->addMd80(id) && candle->writeSDO(id, 0x1010, 0x01, static_cast<uint32_t>(0x65766173));
}

bool Mdtool::status(uint32_t id)
{
	if (!candle->addMd80(id))
		return false;

	auto printErrors = [&](uint32_t status, errorMapType map)
	{
		for (auto& [name, mask] : map)
		{
			if (status & mask)
			{
				if (name.find("WARNING") != std::string::npos)
					logger->warn(name);
				else
					logger->error(name);
			}
		}
	};

	std::array<std::pair<std::string, errorMapType>, 8> errorMapList = {{{"main encoder errors", encoderErrorList},
																		 {"output encoder errors", encoderErrorList},
																		 {"calibration errors", calibrationErrorList},
																		 {"bridge errors", bridgeErrorList},
																		 {"hardware errors", hardwareErrorList},
																		 {"homing errors", homingErrorList},
																		 {"motion errors", motionErrorList},
																		 {"communication errors", communicationErrorList}}};

	for (uint32_t i = 0; i < errorMapList.size(); i++)
	{
		uint32_t status = 0;
		candle->readSDO(id, 0x2004, i + 1, status);
		logger->info("{}: 0x{:x}", errorMapList[i].first, status);
		printErrors(status, errorMapList[i].second);
	}

	return true;
}

bool Mdtool::changeId(uint32_t id, uint32_t newId)
{
	return candle->addMd80(id) &&
		   candle->writeSDO(id, 0x2000, 0x0A, newId) &&
		   candle->writeSDO(id, 0x1010, 0x01, static_cast<uint32_t>(0x65766173));
}

bool Mdtool::changeBaud(uint32_t id, uint32_t newBaud)
{
	return candle->addMd80(id) &&
		   candle->writeSDO(id, 0x2000, 0x0B, newBaud) &&
		   candle->writeSDO(id, 0x1010, 0x01, static_cast<uint32_t>(0x65766173));
}

bool Mdtool::clearError(uint32_t id)
{
	return candle->addMd80(id) &&
		   candle->writeSDO(id, 0x2003, 0x0B, static_cast<uint8_t>(1));
}

bool Mdtool::clearWarning(uint32_t id)
{
	return candle->addMd80(id) &&
		   candle->writeSDO(id, 0x2003, 0x0C, static_cast<uint8_t>(1));
}
/* TODO: refactor when OD read situation is clear */
bool Mdtool::setupInfo(uint32_t id)
{
	if (!candle->addMd80(id))
		return false;

	auto md80 = candle->getMd80(id);

	for (auto& [index, entry] : md80->OD)
	{
		if (index < 0x2000 || index > 0x2100)
			continue;

		logger->info("{}", entry->parameterName);

		for (auto& [subindex, subentry] : entry->subEntries)
		{
			if (subentry->parameterName.find("Highest sub-index supported") != std::string::npos)
				continue;

			auto value = candle->canopenStack->getTypeBasedOnTag(subentry->dataType);

			auto lambdaFunc = [&](auto& arg)
			{
				if (!candle->readSDO(id, index, subindex, arg))
					return false;

				logger->info("     {} = {}", subentry->parameterName, arg);
				return true;
			};

			if (!std::visit(lambdaFunc, value))
				continue;
		}
	}

	return true;
}
