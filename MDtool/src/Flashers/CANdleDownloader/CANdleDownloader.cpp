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

	sendFirmware(firmwareData);

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
		{
			logger->debug("OK reveiced!");
			response = true;
		}
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

bool CANdleDownloader::sendPageCmd(std::span<const uint8_t> payload)
{
	constexpr size_t headerSize = 3;
	std::array<uint8_t, 2051> buf;
	std::span<uint8_t> data(buf.data(), payload.size() + headerSize);

	response = false;
	expectedId = 101;

	buf[0] = 101;
	buf[1] = 0xaa;
	buf[2] = 0xaa;

	std::copy(payload.begin(), payload.end(), &buf[headerSize]);
	return usbHandler->sendDataDirectly(data);
}

bool CANdleDownloader::sendCheckCRCAndWriteCmd(std::span<const uint8_t> firmwareChunk)
{
	constexpr size_t headerSize = 3;
	std::array<uint8_t, 7> buf;
	std::span<uint8_t> data(buf.data(), buf.size());

	auto calculatedCrc = Checksum::crc32(&*firmwareChunk.begin(), firmwareChunk.size());
	serialize(calculatedCrc, &buf[headerSize]);

	response = false;
	expectedId = 102;

	buf[0] = 102;
	buf[1] = 0xaa;
	buf[2] = 0xaa;

	return usbHandler->sendDataDirectly(data);
}

bool CANdleDownloader::sendFirmware(std::span<const uint8_t> firmwareData)
{
	auto it = firmwareData.begin();
	const size_t chunkSize = 1024;
	const size_t pageSize = 2048;

	size_t size = firmwareData.size();
	size_t remainingSize = size;
	logger->debug("Binary size: {}", size);
	float progress = 0.0f;

	/* then proceed with the rest of the firmware*/
	for (size_t i = 0; i <= size; i += pageSize)
	{
		auto begin = it;

		for (size_t j = 0; j < pageSize / chunkSize; j++)
		{
			std::span<const uint8_t> firmwareChunk(it, chunkSize);
			sendPageCmd(firmwareChunk);

			if (!waitForActionWithTimeout([&]() -> bool
										  { return !response; },
										  100))
				return false;

			it += chunkSize;
		}

		sendCheckCRCAndWriteCmd(std::span<const uint8_t>(begin, pageSize));

		if (!waitForActionWithTimeout([&]() -> bool
									  { return !response; },
									  100))
			return false;

		logger->debug("sent: {} bytes", it - begin);
	}

	// progressBar(1.0f);
	// if (mode == Mode::SAFE)
	// {
	// 	if (!sendWriteCmd())
	// 	{
	// 		logger->error("Programming failed!");
	// 		return false;
	// 	}
	// }

	return true;
}