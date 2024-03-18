#include "mdtool.hpp"

#include <array>
#include <utility>

#include "BinaryParser.hpp"
#include "CANdleDownloader.hpp"
#include "ConfigParser/ConfigParser.hpp"
#include "MD80Downloader.hpp"
#include "spdlog/fmt/ostr.h"

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

bool Mdtool::ping(bool checkChannels)
{
	if (!checkChannels)
	{
		auto ids = candle->ping();

		if (ids.size() > 0)
			logger->info("Found drives: ");
		else
			logger->warn("No drives found!");

		for (auto& id : ids)
			logger->info(std::to_string(id));

		return true;
	}

	auto idsAndChannels = candle->pingWithChannel();

	if (idsAndChannels.size() > 0)
		logger->info("Found drives!");
	else
		logger->warn("No drives found!");

	for (auto& [id, ch] : idsAndChannels)
	{
		logger->info("{} on channel {}", id, ch);
	}

	return true;
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

	std::vector<std::pair<std::pair<uint32_t, uint8_t>, MD80Downloader::Status>> ids = {};

	if (all)
	{
		logger->info("Pinging drives at baudrate {}M...", static_cast<uint32_t>(baudrate));

		for (auto [id, channel] : candle->pingWithChannel())
			ids.push_back({{id, channel}, MD80Downloader::Status::OK});

		if (ids.empty())
			logger->info("No drives found!");
		else
			logger->info("Found {} drives!", ids.size());
	}
	else
	{
		auto ch = candle->getChannelBasedOnId(id);
		ids.push_back({{id, ch}, MD80Downloader::Status::OK});
	}

	candle->deInit();
	MD80Downloader downloader(interface, logger);

	for (auto& [idChPair, status] : ids)
	{
		logger->info("Preparing to update drive ID {} at baudrate {}M...", idChPair.first, static_cast<uint32_t>(baudrate));
		auto buffer = BinaryParser::getPrimaryFirmwareFile();
		status = downloader.doLoad(std::span<uint8_t>(buffer), idChPair.first, idChPair.second, recover, 0, false);

		if (status != MD80Downloader::Status::OK)
			logger->error("Status: {}", static_cast<uint8_t>(status));
	}

	auto updateOk = std::count_if(ids.begin(), ids.end(),
								  [](const auto& md80)
								  { return md80.second == MD80Downloader::Status::OK; });

	logger->info("Update successful for {} drive(s)", updateOk);

	for (auto [idChPair, status] : ids)
	{
		if (status == MD80Downloader::Status::OK)
			logger->info("ID {}", idChPair.first);
	}

	logger->info("Update failed for {} drive(s) ", ids.size() - updateOk);

	for (auto [idChPair, status] : ids)
	{
		if (status != MD80Downloader::Status::OK)
			logger->error("ID {} with status {}", idChPair.first, static_cast<uint8_t>(status));
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
	auto updateStatus = downloader.doLoad(std::span<uint8_t>(buffer), id, 0, recover, secondaryBootloaderAddress, false);

	if (updateStatus != MD80Downloader::Status::OK)
	{
		logger->error("Update failed! Status: {}", static_cast<uint8_t>(updateStatus));
		return false;
	}

	logger->info("Preparing to update the primary bootloader of drive ID {}", id);
	buffer = BinaryParser::getPrimaryFirmwareFile();
	updateStatus = downloader.doLoad(std::span<uint8_t>(buffer), id, 0, recover, primaryBootloaderAddress, true);

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
	if (!candle->addMd80(id))
		return false;

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
	if (!candle->addMd80(id))
		return false;

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
	return performAction(id, 0x2003, 0x03, true);
}

bool Mdtool::calibrateOutput(uint32_t id)
{
	return performAction(id, 0x2003, 0x04, true);
}

bool Mdtool::home(uint32_t id)
{
	return performAction(id, 0x2003, 0x0E, true);
}

bool Mdtool::save(uint32_t id, bool all)
{
	if (all)
	{
		auto drives = candle->ping();

		logger->info("Saving config on drives: ");

		for (auto& id : drives)
		{
			auto result = candle->addMd80(id) && candle->writeSDO(id, 0x1010, 0x01, static_cast<uint32_t>(0x65766173));
			if (result)
				logger->info("ID{} - success", std::to_string(id));
			else
				logger->error("ID{} - error", std::to_string(id));
		}

		return true;
	}

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

	std::array<std::pair<std::string, errorMapType>, 10> errorMapList = {{{"main encoder errors", encoderErrorList},
																		  {"output encoder errors", encoderErrorList},
																		  {"calibration errors", calibrationErrorList},
																		  {"bridge errors", bridgeErrorList},
																		  {"hardware errors", hardwareErrorList},
																		  {"homing errors", homingErrorList},
																		  {"motion errors", motionErrorList},
																		  {"communication errors", communicationErrorList},
																		  {"persistent storage read errors", persistentStorageErrorList},
																		  {"persistent storage write errors", persistentStorageErrorList}}};

	for (uint32_t i = 0; i < errorMapList.size(); i++)
	{
		uint32_t status = 0;
		candle->readSDO(id, 0x2004, i + 1, status);
		logger->info("{}: 0x{:x}", errorMapList[i].first, status);
		printErrors(status, errorMapList[i].second);

		/* check if we should disply last setup error */
		if (errorMapList[i].first == "calibration errors")
		{
			uint32_t errorSetupMask = calibrationErrorList.at("ERROR_SETUP");
			if (status & errorSetupMask)
			{
				uint32_t setupError = 0;
				candle->readSDO(id, 0x2004, 0x0B, setupError);
				logger->error("ERROR_SETUP_CODE: {}", setupErrorList.at(setupError));
			}
		}
	}

	return true;
}

bool Mdtool::changeId(uint32_t id, uint32_t newId)
{
	return candle->addMd80(id) &&
		   candle->writeSDO(id, 0x2000, 0x0A, newId) &&
		   candle->writeSDO(id, 0x1010, 0x01, static_cast<uint32_t>(0x65766173));
}

bool Mdtool::changeBaud(uint32_t id, uint32_t newBaud, bool all)
{
	if (all)
	{
		auto drives = candle->ping();

		logger->info("Changing baudrate on drives: ");

		for (auto& id : drives)
		{
			auto result = candle->addMd80(id) && candle->writeSDO(id, 0x2000, 0x0B, newBaud);
			if (result)
				logger->info("ID{} - success", std::to_string(id));
			else
				logger->error("ID{} - error", std::to_string(id));
		}

		return true;
	}

	return candle->addMd80(id) && candle->writeSDO(id, 0x2000, 0x0B, newBaud);
}

bool Mdtool::clearError(uint32_t id)
{
	return candle->addMd80(id) &&
		   candle->writeSDO(id, 0x2003, 0x0B, true);
}

bool Mdtool::clearWarning(uint32_t id)
{
	return candle->addMd80(id) &&
		   candle->writeSDO(id, 0x2003, 0x0C, true);
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

bool Mdtool::setZero(uint32_t id)
{
	return candle->addMd80(id) && candle->setZeroPosition(id);
}

bool Mdtool::reset(uint32_t id)
{
	return candle->addMd80(id) && candle->reset(id);
}

bool Mdtool::setupMotor(uint32_t id, const std::string& filePath, bool all)
{
	std::vector<uint32_t> drives;

	if (all)
		drives = candle->ping();
	else
		drives.push_back(id);

	bool state = true;

	for (auto& id : drives)
	{
		if (!candle->addMd80(id))
		{
			logger->error("Error adding MD80 with ID{}", id);
			return false;
		}

		auto& OD = candle->getMd80(id)->OD;

		ConfigParser CP(logger, &OD);

		if (!CP.openFile(filePath))
		{
			logger->error("Error opening file: {}", filePath);
			return false;
		}

		auto ODentries = CP.parseFile();

		if (!ODentries.has_value())
		{
			logger->error("Error parsing file: {}", filePath);
			return false;
		}

		for (auto& [index, subindex] : ODentries.value())
		{
			auto lambdaFunc = [&](auto& arg) -> bool
			{
				if (!candle->writeSDO(id, index, subindex, std::move(arg)))
					return false;

				logger->debug("Writing successful! 0x{:x}:0x{:x} = {}", index, subindex, arg);
				return true;
			};

			if (!std::visit(lambdaFunc, OD[index]->subEntries[subindex]->value))
				state = false;
		}

		if (!candle->writeSDO(id, 0x1010, 0x01, static_cast<uint32_t>(0x65766173)))
		{
			logger->error("Error saving on ID{}", id);
			return false;
		}

		logger->info("Setup successfull on ID{}", id);
	}

	return state;
}

bool Mdtool::move(uint32_t id, bool relative, float targetPosition, float profileVelocity, float profileAcceleration)
{
	if (!candle->addMd80(id))
		return false;

	logger->info("Target position {}", targetPosition);

	if (relative)
		candle->setZeroPosition(id);

	if (!std::isnan(profileVelocity))
		candle->writeSDO(id, 0x2008, 0x03, profileVelocity);

	if (!std::isnan(profileAcceleration))
	{
		candle->writeSDO(id, 0x2008, 0x04, profileAcceleration);
		candle->writeSDO(id, 0x2008, 0x05, profileAcceleration);
	}

	candle->setModeOfOperation(id, Candle::ModesOfOperation::PROFILE_POSITION);
	candle->enterOperational(id);

	auto md80 = candle->getMd80(id);

	float actualPosition = 0;
	candle->readSDO(id, 0x2009, 0x01, actualPosition);
	logger->info("Actual position {}", actualPosition);

	candle->writeSDO(id, 0x2008, 0x09, targetPosition);

	uint16_t statusWord = 0;
	while (!md80->isTargetReached())
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
		candle->readSDO(id, 0x6041, 0x00, statusWord);
	}

	return true;
}

bool Mdtool::blink(uint32_t id)
{
	return performAction(id, 0x2003, 0x01);
}

bool Mdtool::performAction(uint32_t id, uint16_t index, uint8_t subindex, bool operationalServiceRequired)
{
	if (!candle->addMd80(id))
		return false;

	if (operationalServiceRequired)
	{
		if (!candle->enterOperational(id) || !candle->setModeOfOperation(id, Candle::ModesOfOperation::SERVICE))
			return false;
	}

	bool inProgress = true;

	candle->writeSDO(id, index, subindex, inProgress);

	while (inProgress)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
		candle->readSDO(id, index, subindex, inProgress);
	}

	return true;
}