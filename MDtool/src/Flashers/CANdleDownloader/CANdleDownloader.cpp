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

		while (!usbHandler->isOutputFifoEmpty())
			;
	}

	usbHandler.reset();
	usbHandler = std::make_unique<UsbHandler>(logger);

	for (size_t i = 0; i < 100; i++)
	{
		if (usbHandler->init(BOOTLOADER_VID, BOOTLOADER_PID, true, false))
			break;
	}

	if (!usbHandler->init(BOOTLOADER_VID, BOOTLOADER_PID, true))
		return Status::ERROR_INIT;

	logger->debug("Init bootloder OK");
	receiveThread = std::thread(&CANdleDownloader::receiveHandler, this);

	if (!executeAndWaitForResponse([&]()
								   { sendInitCmd(); }))
		return Status::ERROR_INIT;

	sendFirmware(firmwareData);

	if (!executeAndWaitForResponse([&]()
								   { sendBootCmd(); }))
		return Status::ERROR_BOOT;

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
			logger->debug("OK received!");
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
			logger->warn("Timeout!");
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
			auto sizeToSend = remainingSize > chunkSize ? chunkSize : remainingSize;
			std::span<const uint8_t> firmwareChunk(it, sizeToSend);

			if (!executeAndWaitForResponse([&]()
										   { sendPageCmd(firmwareChunk); }))
				return false;

			remainingSize -= sizeToSend;
			it += sizeToSend;
		}

		if (!executeAndWaitForResponse([&]()
									   { sendCheckCRCAndWriteCmd(std::span<const uint8_t>(begin, pageSize)); }))
			return false;

		logger->debug("sent: {} bytes, remainingSize {}", it - begin, remainingSize);
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

bool CANdleDownloader::sendBootCmd()
{
	std::array<uint8_t, 3> buf;
	std::span<uint8_t> data(buf.data(), buf.size());

	response = false;
	expectedId = 103;

	buf[0] = 103;
	buf[1] = 0xaa;
	buf[2] = 0xaa;

	return usbHandler->sendDataDirectly(data);
}

bool CANdleDownloader::executeAndWaitForResponse(std::function<void()> function)
{
	if (function)
		function();

	return waitForActionWithTimeout([&]() -> bool
									{ return !response; },
									100);
}