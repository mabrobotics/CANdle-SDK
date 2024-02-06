#include "CANdleDownloader.hpp"

#include <algorithm>
#include <functional>
#include <thread>
#include <utility>

#include "BusHandler/UsbHandler.hpp"
#include "Checksum/Checksum.hpp"
#include "Commons/FlasherCommons.hpp"

CANdleDownloader::CANdleDownloader(std::shared_ptr<spdlog::logger> logger) : logger(logger)
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
	auto status = perform(std::forward<std::span<const uint8_t>>(firmwareData), recover);
	/* we have to remember about newline or the progress bar will be overwritten */
	if (status == Status::OK)
		std::cout << std::endl;
	return status;
}

CANdleDownloader::Status CANdleDownloader::perform(std::span<const uint8_t>&& firmwareData, bool recover)
{
	if (!recover)
	{
		usbHandler = std::make_unique<UsbHandler>(logger);

		if (!usbHandler->init(APP_VID, APP_PID, true))
			return Status::ERROR_INIT;

		if (!sendResetCmd())
			return Status::ERROR_RESET;

		logger->debug("Reset OK");
		usbHandler.reset();
	}

	usbHandler = std::make_unique<UsbHandler>(logger);

	for (size_t i = 0; i < 100; i++)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(20));
		if (usbHandler->init(BOOTLOADER_VID, BOOTLOADER_PID, true, false))
			break;
	}

	if (!usbHandler->init(BOOTLOADER_VID, BOOTLOADER_PID, true))
		return Status::ERROR_INIT;

	logger->debug("Init bootloder OK");
	receiveThread = std::thread(&CANdleDownloader::receiveHandler, this);

	if (!sendInitCmd() || !waitForResponse())
		return Status::ERROR_INIT;

	if (!sendFirmware(firmwareData))
		return Status::ERROR_FIRMWARE;

	/* make sure the receiving thread ends after boot command as we're about to switch to application usb device */
	done = true;

	if (!sendBootCmd() || !waitForResponse())
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
			break;

		auto id = static_cast<BootloaderFrameId>(data[0]);
		if (id == expectedId && data[1] == 'O' && data[2] == 'K')
		{
			logger->debug("OK received!");
			response = true;
		}
	}
	logger->debug("Closing receive thread...");
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

	std::array<uint8_t, 3> buf;
	std::span<uint8_t> data(buf.data(), buf.size());

	buf[0] = 10;
	buf[1] = 0;
	buf[2] = 1;

	return usbHandler->sendDataDirectly(data);
}

bool CANdleDownloader::sendInitCmd()
{
	std::array<uint8_t, 3> buf;
	std::span<uint8_t> data(buf.data(), buf.size());

	response = false;
	expectedId = CHECK_ENTERED;

	buf[0] = CHECK_ENTERED;
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
	expectedId = SEND_PAGE;

	buf[0] = SEND_PAGE;
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
	expectedId = WRITE_PAGE;

	buf[0] = WRITE_PAGE;
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

	/* then proceed with the rest of the firmware*/
	for (size_t i = 0; i <= size; i += pageSize)
	{
		auto begin = it;

		for (size_t j = 0; j < pageSize / chunkSize; j++)
		{
			auto sizeToSend = remainingSize > chunkSize ? chunkSize : remainingSize;
			std::span<const uint8_t> firmwareChunk(it, sizeToSend);

			if (!sendPageCmd(firmwareChunk) || !waitForResponse())
				return false;

			remainingSize -= sizeToSend;
			it += sizeToSend;
		}

		auto sentChunkSize = it - begin;

		logger->debug("checking CRC");

		if (!sendCheckCRCAndWriteCmd(std::span<const uint8_t>(begin, sentChunkSize)) || !waitForResponse())
			return false;

		logger->debug("sent: {} bytes, remainingSize {}", sentChunkSize, remainingSize);
		FlasherCommons::progressBar(static_cast<float>(i) / static_cast<float>(size));
	}

	FlasherCommons::progressBar(1.0f);

	return true;
}

bool CANdleDownloader::sendBootCmd()
{
	std::array<uint8_t, 3> buf;
	std::span<uint8_t> data(buf.data(), buf.size());

	response = false;
	expectedId = BOOT_TO_APP;

	buf[0] = BOOT_TO_APP;
	buf[1] = 0xaa;
	buf[2] = 0xaa;

	return usbHandler->sendDataDirectly(data);
}

bool CANdleDownloader::waitForResponse()
{
	return waitForActionWithTimeout([&]() -> bool
									{ return !response; },
									defaultTimeout);
}