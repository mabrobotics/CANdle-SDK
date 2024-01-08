#include "mdtool.hpp"

#include "BinaryParser.hpp"
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

Mdtool::~Mdtool()
{
	candle->deInit();
}

bool Mdtool::init(ICommunication* interface, spdlog::logger* logger, Candle::Baud baud)
{
	logger->debug("Initalizing...");
	this->logger = logger;
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

bool Mdtool::readSDO(uint32_t id, uint32_t index, uint32_t subindex)
{
	candle->addMd80(id);
	auto md80 = candle->getMd80(id);
	auto maybeEntry = checkEntryExists(md80, index, subindex);

	if (!maybeEntry.has_value())
		return false;

	auto& value = maybeEntry.value()->value;
	value = getTypeBasedOnTag(maybeEntry.value()->datatype);

	uint32_t errorCode = 0;

	auto lambdaFunc = [&](auto& arg)
	{
		if (!candle->canopenStack->readSDO(id, index, subindex, arg, errorCode))
			return false;

		if (errorCode)
		{
			logger->error("SDO read error! Error code: 0x{:x}", errorCode);
			return false;
		}
		else
		{
			logger->info("SDO value: {}", arg);
			return true;
		}
	};

	return std::visit(lambdaFunc, value);
}

bool Mdtool::writeSDO(uint32_t id, uint32_t index, uint32_t subindex, const IODParser::ValueType& value)
{
	candle->addMd80(id);
	uint32_t errorCode = 0;

	auto md80 = candle->getMd80(id);
	auto maybeEntry = checkEntryExists(md80, index, subindex);

	if (!maybeEntry.has_value())
		return false;

	maybeEntry.value()->value = value;

	auto lambdaFunc = [&](auto& arg)
	{
		using T = std::decay_t<decltype(arg)>;
		if constexpr (std::is_same_v<T, std::array<uint8_t, 24>>)
		{
			if (!candle->canopenStack->writeSDO(id, index, subindex, std::move(arg), errorCode, strlen(reinterpret_cast<const char*>(arg.data()))))
			{
				logger->error("SDO write error! Error code: 0x{:x}", errorCode);
				return false;
			}
		}
		else
		{
			if (!candle->canopenStack->writeSDO(id, index, subindex, std::move(arg), errorCode))
			{
				logger->error("SDO write error! Error code: 0x{:x}", errorCode);
				return false;
			}
		}

		logger->info("Writing successful! 0x{:x}:0x{:x} ({}) = {}", index, subindex, maybeEntry.value()->parameterName, arg);
		return true;
	};

	return std::visit(lambdaFunc, value);
}

IODParser::ValueType Mdtool::getTypeBasedOnTag(IODParser::DataType tag)
{
	switch (tag)
	{
		case IODParser::DataType::BOOLEAN:
			[[fallthrough]];
		case IODParser::DataType::UNSIGNED8:
			return uint8_t{};
		case IODParser::DataType::INTEGER8:
			return int8_t{};
		case IODParser::DataType::UNSIGNED16:
			return uint16_t{};
		case IODParser::DataType::INTEGER16:
			return int16_t{};
		case IODParser::DataType::UNSIGNED32:
			return uint32_t{};
		case IODParser::DataType::INTEGER32:
			return int32_t{};
		case IODParser::DataType::REAL32:
			return float{};
		case IODParser::DataType::VISIBLE_STRING:
			return std::array<uint8_t, 24>{};
		default:
			return uint32_t{};
	}
}

std::optional<IODParser::Entry*> Mdtool::checkEntryExists(MD80* md80, uint16_t index, uint8_t subindex)
{
	if (!md80->OD.contains(index))
	{
		logger->error("Entry index not found in OD (0x{:x})", index);
		return std::nullopt;
	}
	else if (md80->OD.contains(index) && md80->OD.at(index)->objectType == IODParser::ObjectType::VAR)
		return md80->OD.at(index).get();

	if (!md80->OD.at(index)->subEntries.contains(subindex))
	{
		logger->error("Entry subindex not found in OD (0x{:x}:0x{:x})", index, subindex);
		return std::nullopt;
	}

	return md80->OD.at(index)->subEntries.at(subindex).get();
}
