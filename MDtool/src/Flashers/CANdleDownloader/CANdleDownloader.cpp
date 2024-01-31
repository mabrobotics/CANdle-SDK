#include "CANdleDownloader.hpp"

#include <algorithm>
#include <functional>
#include <thread>
#include <utility>

#include "BusHandler/UsbHandler.hpp"
#include "Checksum/Checksum.hpp"

CANdleDownloader::CANdleDownloader(spdlog::logger* logger) : logger(logger)
{
}

CANdleDownloader::~CANdleDownloader()
{
	done = true;
	if (receiveThread.joinable())
		receiveThread.join();
}

CANdleDownloader::Status CANdleDownloader::doLoad(std::span<const uint8_t>&& firmwareData, bool recover)
{
	if (!recover)
	{
		usbHandler = std::make_unique<UsbHandler>(logger);

		if (!usbHandler->init())
			return Status::ERROR_INIT;

		if (!sendResetCmd())
			return Status::ERROR_RESET;

		logger->debug("Reset OK");
	}

	std::this_thread::sleep_for(std::chrono::milliseconds(100));

	usbHandler.reset();
	usbHandler = std::make_unique<UsbHandler>(logger);

	std::this_thread::sleep_for(std::chrono::milliseconds(1000));

	if (!usbHandler->init(0x0069, 0x2000, true))
		return Status::ERROR_INIT;

	logger->debug("Init bootloder OK");
	receiveThread = std::thread(&CANdleDownloader::receiveHandler, this);
	auto success = sendInitCmd();

	if (!waitForActionWithTimeout([&]() -> bool
								  { return response; },
								  100))
		return Status::ERROR_INIT;

	success = sendPage();

	if (!waitForActionWithTimeout([&]() -> bool
								  { return response; },
								  100))
		return Status::ERROR_INIT;

	logger->debug("Boot OK");
	return Status::OK;
}

void CANdleDownloader::receiveHandler()
{
	std::array<uint8_t, 3> buf;
	std::span<uint8_t> data(buf.data(), buf.size());

	while (!done)
	{
		if (!usbHandler->receiveDataDirectly(data))
			continue;

		auto id = static_cast<BootloaderFrameId>(data[0]);
		if (id == expectedId && data[1] == 'O' && data[2] == 'K')
			response = true;
	}
}

bool CANdleDownloader::waitForActionWithTimeout(std::function<bool()> condition, uint32_t timeoutMs)
{
	auto start_time = std::chrono::high_resolution_clock::now();
	auto end_time = start_time + std::chrono::milliseconds(timeoutMs);

	while (condition())
	{
		if (std::chrono::high_resolution_clock::now() >= end_time)
		{
			logger->debug("Timeout!");
			return false;
		}
	}
	return true;
}

bool CANdleDownloader::sendResetCmd()
{
	logger->debug("Resetting...");

	IBusHandler::BusFrame frame{};
	frame.header.id = 10;
	frame.header.length = 1;
	return usbHandler->addToFifo(frame);
}

bool CANdleDownloader::sendInitCmd()
{
	std::array<uint8_t, 3> buf;
	std::span<uint8_t> data(buf.data(), buf.size());

	response = false;
	expectedId = 100;

	buf[0] = 100;
	buf[1] = 0xaa;
	buf[2] = 0xaa;

	return usbHandler->sendDataDirectly(data);
}

bool CANdleDownloader::sendPage()
{
	std::array<uint8_t, 3> buf;
	std::span<uint8_t> data(buf.data(), buf.size());

	response = false;
	expectedId = 100;

	buf[0] = 101;
	buf[1] = 0xaa;
	buf[2] = 0xaa;

	return usbHandler->sendDataDirectly(data);
}
