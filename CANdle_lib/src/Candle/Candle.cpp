#include "Candle.hpp"

Candle::Candle() : syncPoint(3)
{
	std::string name = "candle_logger";
	logger = spdlog::stdout_color_mt(name + std::to_string(candleNum++));
	logger->set_pattern("[%^%l%$] %v");
	logger->set_level(spdlog::level::level_enum::debug);
	interface = std::make_shared<CandleInterface>(std::make_unique<UsbHandler>(logger));
}

Candle::Candle(std::shared_ptr<ICommunication> interface, std::shared_ptr<spdlog::logger> logger) : syncPoint(3),
																									interface(interface),
																									logger(logger)

{
}

Candle::~Candle()
{
	deInit();
}

bool Candle::init(Baud baud)
{
	ICommunication::Settings settings;

	if (baud == Baud::BAUD_8M)
		settings.baudrate = 8000000;
	else
		settings.baudrate = 1000000;

	auto status = interface->init(settings);

	if (!status)
	{
		logger->error("Unable to init communication interface!");
		return false;
	}

	candleChannels = interface->getCanChannels();

	logger->debug("Hardware version: {}", interface->getHardwareVersion());
	logger->debug("CANdle active CAN channels: {}", candleChannels);

	canopenStack = std::make_unique<CanopenStack>(interface, logger);
	receiveThread = std::thread(&Candle::receiveHandler, this);
	transmitThread = std::thread(&Candle::transmitHandler, this);

	syncPoint.wait();

	isInitialized = true;

	return status;
}

void Candle::deInit()
{
	done = true;
	if (receiveThread.joinable())
		receiveThread.join();
	if (transmitThread.joinable())
		transmitThread.join();
}

void Candle::setSendSync(bool state, uint32_t intervalUs)
{
	sendSync = state;
	syncIntervalUs = intervalUs;
}

std::vector<uint32_t> Candle::ping()
{
	std::vector<uint32_t> ids{};
	uint32_t deviceType = 0;
	uint32_t errorCode = 0;

	for (size_t id = 1; id < 31; id++)
		if (canopenStack->readSDO(id, 0x1000, 0x00, deviceType, errorCode, false, 0xff))
			ids.push_back(id);

	return ids;
}

std::vector<std::pair<uint32_t, uint8_t>> Candle::pingWithChannel()
{
	std::vector<std::pair<uint32_t, uint8_t>> idsAndChannel;
	uint32_t deviceType = 0;
	uint32_t errorCode = 0;

	for (uint8_t ch = 0; ch < candleChannels; ch++)
	{
		for (size_t id = 1; id < 31; id++)
		{
			if (canopenStack->readSDO(id, 0x1000, 0x00, deviceType, errorCode, false, ch))
				idsAndChannel.push_back({id, ch});
		}
	}
	return idsAndChannel;
}

uint8_t Candle::getChannelBasedOnId(uint32_t id)
{
	uint32_t deviceType = 0;
	uint32_t errorCode = 0;

	for (uint8_t channel = 0; channel < candleChannels; channel++)
	{
		if (canopenStack->readSDO(id, 0x1000, 0x00, deviceType, errorCode, false, channel))
			return channel;
	}
	return 0;
}

bool Candle::addMd80(uint32_t id)
{
	uint32_t deviceType = 0;
	bool success = false;
	uint8_t channel = 0;
	uint32_t errorCode = 0;

	for (; channel < candleChannels; channel++)
	{
		if (canopenStack->readSDO(id, 0x1000, 0x00, deviceType, errorCode, false, channel))
		{
			logger->info("MD80 with ID {} found on channel {}!", id, channel);
			success = true;
			break;
		}

		logger->debug("MD80 with ID {} not found on channel {}", id, channel);
	}

	if (!success)
		return false;

	md80s[id] = std::make_unique<MD80>();
	canopenStack->setOD(id, &md80s[id]->OD);
	canopenStack->setChannel(id, channel);
	return true;
}

std::shared_ptr<MD80> Candle::getMd80(uint32_t id) const
{
	return md80s.at(id);
}

bool Candle::enterOperational(uint32_t id)
{
	return writeSDO(id, 0x6040, 0x00, static_cast<uint16_t>(0x0080)) &&
		   writeSDO(id, 0x6040, 0x00, static_cast<uint16_t>(0x0006)) &&
		   writeSDO(id, 0x6040, 0x00, static_cast<uint16_t>(0x000f));
}

bool Candle::enterSwitchOnDisabled(uint32_t id)
{
	return writeSDO(id, 0x6040, 0x00, static_cast<uint16_t>(0x0008));
}

bool Candle::setModeOfOperation(uint32_t id, ModesOfOperation mode)
{
	return writeSDO(id, 0x6060, 0x00, static_cast<int8_t>(mode));
}

bool Candle::setTargetPosition(uint32_t id, uint32_t target)
{
	return writeSDO(id, 0x607A, 0x00, target);
}

bool Candle::startCalibration(uint32_t id)
{
	return enterOperational(id) &&
		   writeSDO(id, 0x6060, 0x00, static_cast<int8_t>(-2)) &&
		   writeSDO(id, 0x2003, 0x03, static_cast<uint8_t>(1));
}

bool Candle::setZeroPosition(uint32_t id)
{
	return writeSDO(id, 0x2003, 0x05, true);
}

bool Candle::reset(uint32_t id)
{
	return writeSDO(id, 0x2003, 0x02, true);
}

bool Candle::setupPDO(uint32_t id, CanopenStack::PDO pdoID, const std::vector<std::pair<uint16_t, uint8_t>>& fields)
{
	return canopenStack->setupPDO(id, pdoID, fields);
}

void Candle::receiveHandler()
{
	logger->debug("Starting candle receive thread...");

	syncPoint.wait();
	while (!done)
	{
		auto maybeFrame = interface->receiveCanFrame();

		if (!maybeFrame.has_value())
			continue;

		canopenStack->parse(maybeFrame.value());

		handleCandleDeviceStatus();
	}

	logger->debug("Ending candle receive thread...");
}

void Candle::transmitHandler()
{
	logger->debug("Starting candle transmit thread...");

	syncPoint.wait();
	while (!done)
	{
		canopenStack->sendRPDOs();

		if (sendSync)
			canopenStack->sendSYNC();

		auto end_time = std::chrono::high_resolution_clock::now() + std::chrono::microseconds(syncIntervalUs);
		while (std::chrono::high_resolution_clock::now() < end_time)
		{
		}
	}
	logger->debug("Ending candle transmit thread...");
}

void Candle::handleCandleDeviceStatus()
{
	static bool rxFifoWarning = false;
	static bool txFifoWarning = false;
	static bool rxFifoError = false;
	static bool txFifoError = false;

	static size_t loop_count = 0;

	if (loop_count++ >= 1000)
	{
		auto status = interface->getStatus();

		if (!rxFifoWarning && status.statistics.maxRxFifoOccupancyPercent > rxFifoWarningLevel)
		{
			logger->warn("CANdle's internal RX fifo max occupancy exceeded {}%", rxFifoWarningLevel);
			rxFifoWarning = true;
		}

		if (!txFifoWarning && status.statistics.maxTxFifoOccupancyPercent > txFifoWarningLevel)
		{
			logger->warn("CANdle's internal TX fifo max occupancy exceeded {}%", txFifoWarningLevel);
			txFifoWarning = true;
		}

		if (!rxFifoError && status.statistics.maxRxFifoOccupancyPercent >= rxFifoErrorLevel)
		{
			logger->error("CANdle's internal RX fifo max occupancy exceeded {}%. Some messages are probably lost. Please consider slowing down the communication or minimizing the data throughput.", rxFifoErrorLevel);
			rxFifoError = true;
		}

		if (!txFifoError && status.statistics.maxTxFifoOccupancyPercent > txFifoErrorLevel)
		{
			logger->error("CANdle's internal TX fifo max occupancy exceeded {}%. Some messages are probably lost. Please consider slowing down the communication or minimizing the data throughput.", txFifoErrorLevel);
			txFifoError = true;
		}

		loop_count = 0;
	}
}