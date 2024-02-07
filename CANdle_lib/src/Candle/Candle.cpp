#include "Candle.hpp"

Candle::Candle()
{
	logger = spdlog::stdout_color_mt("console");
	logger->set_pattern("[%^%l%$] %v");
	interface = std::make_shared<CandleInterface>(std::make_unique<UsbHandler>(logger));
	canopenStack = std::make_unique<CanopenStack>(interface, logger);
	receiveThread = std::thread(&Candle::receiveHandler, this);
	transmitThread = std::thread(&Candle::transmitHandler, this);
}

Candle::Candle(std::shared_ptr<ICommunication> interface, std::shared_ptr<spdlog::logger> logger) : interface(interface),
																									logger(logger)
{
	canopenStack = std::make_unique<CanopenStack>(interface, logger);
	receiveThread = std::thread(&Candle::receiveHandler, this);
	transmitThread = std::thread(&Candle::transmitHandler, this);
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

	if (status)
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

	if (!isInitialized)
		return;

	interface->deinit();
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

	for (size_t i = 1; i < 31; i++)
		if (readSDO(i, 0x1000, 0x00, deviceType, false))
			ids.push_back(i);

	return ids;
}

bool Candle::addMd80(uint32_t id)
{
	uint32_t deviceType = 0;
	if (!readSDO(id, 0x1000, 0x00, deviceType, false))
	{
		logger->error("Unable to add MD80 with ID {}", id);
		return false;
	}

	md80s[id] = std::make_unique<MD80>();
	canopenStack->setOD(id, &md80s[id]->OD);

	return true;
}

MD80* Candle::getMd80(uint32_t id) const
{
	return md80s.at(id).get();
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

bool Candle::setupResponse(uint32_t id, CanopenStack::PDO pdoID, std::vector<std::pair<uint16_t, uint8_t>>& fields)
{
	return setupResponse(id, pdoID, std::move(fields));
}

bool Candle::setupResponse(uint32_t id, CanopenStack::PDO pdoID, std::vector<std::pair<uint16_t, uint8_t>>&& fields)
{
	return canopenStack->setupPDO(id, pdoID, fields);
}

bool Candle::setupCommand(uint32_t id, CanopenStack::PDO pdoID, std::vector<std::pair<uint16_t, uint8_t>>& fields)
{
	return setupCommand(id, pdoID, std::move(fields));
}

bool Candle::setupCommand(uint32_t id, CanopenStack::PDO pdoID, std::vector<std::pair<uint16_t, uint8_t>>&& fields)
{
	return canopenStack->setupPDO(id, pdoID, fields);
}

void Candle::receiveHandler()
{
	while (!done)
	{
		auto maybeFrame = interface->receiveCanFrame();

		if (!maybeFrame.has_value())
			continue;

		canopenStack->parse(maybeFrame.value());
	}
}

void Candle::transmitHandler()
{
	while (!done)
	{
		/* SEND RPDOs */
		canopenStack->sendRPDOs();

		if (sendSync)
			canopenStack->sendSYNC();

		auto end_time = std::chrono::high_resolution_clock::now() + std::chrono::microseconds(syncIntervalUs);
		while (std::chrono::high_resolution_clock::now() < end_time)
		{
		}
	}
}