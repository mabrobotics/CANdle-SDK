#include "MD80Downloader.hpp"

#include <algorithm>
#include <functional>
#include <thread>
#include <utility>

#include "BusHandler/IBusHandler.hpp"
#include "Checksum/Checksum.hpp"
#include "Commons/FlasherCommons.hpp"
#include "Communication/CandleInterface.hpp"

MD80Downloader::MD80Downloader(std::shared_ptr<ICommunication> interface, std::shared_ptr<spdlog::logger> logger) : interface(interface), logger(logger)
{
	receiveThread = std::thread(&MD80Downloader::receiveHandler, this);
}

MD80Downloader::~MD80Downloader()
{
	done = true;
	if (receiveThread.joinable())
		receiveThread.join();
}

MD80Downloader::Status MD80Downloader::doLoad(std::span<const uint8_t>&& firmwareData, uint32_t id, bool recover, uint32_t address, bool secondaryBootloader)
{
	deviceId = id;

	if (address != 0)
		bootAddress = address;

	logger->debug("Boot address: 0x{:x}", bootAddress);

	if (recover)
	{
		canIdCommand = BACKDOOR_CMD_ID;
		canIdResponse = BACKDOOR_RESP_ID;
	}
	else
	{
		canIdCommand = BASE_CMD_ID + id;
		canIdResponse = BASE_RESP_ID + id;
	}

	if (!recover)
	{
		if (!sendResetCmd())
			return Status::ERROR_RESET;

		logger->debug("Reset OK");
	}

	bool success = false;
	auto initCommand = secondaryBootloader ? Command::HOST_INIT_SECONDARY : Command::HOST_INIT;

	if (secondaryBootloader)
		bootAddress = 0x8000000;

	while (recover && !success)
		success = sendInitCmd(initCommand);

	if (!recover)
	{
		for (int i = 0; i < 10; i++)
		{
			success = sendInitCmd(initCommand);
			if (success)
				break;
		}
	}

	if (!success)
		return Status::ERROR_INIT;

	logger->debug("Init OK");

	if (!sendFirmware(firmwareData))
		return Status::ERROR_FIRMWARE;

	logger->debug("Firmware OK");

	if (!sendBootCmd())
		return Status::ERROR_BOOT;

	logger->debug("Boot OK");
	return Status::OK;
}

void MD80Downloader::receiveHandler()
{
	while (!done)
	{
		auto frame = interface->receiveCanFrame();

		if (!frame.has_value())
			continue;

		auto canFrame = frame.value();
		if (canFrame.header.canId == canIdResponse && canFrame.header.length == 5)
			lastResponse = static_cast<Response>(canFrame.payload[0]);
	}
}

bool MD80Downloader::waitForActionWithTimeout(std::function<bool()> condition, uint32_t timeoutMs)
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

bool MD80Downloader::sendFrameWaitForResponse(ICommunication::CANFrame& frame, Response expectedResponse, uint32_t timeout)
{
	lastResponse = Response::NONE;
	if (!interface->sendCanFrame(frame))
		return false;

	if (!waitForActionWithTimeout([&]() -> bool
								  { return lastResponse != expectedResponse; },
								  timeout))
		return false;

	return true;
}

bool MD80Downloader::sendResetCmd()
{
	logger->debug("Resetting...");
	ICommunication::CANFrame frame{};
	frame.header.canId = 0x600 + deviceId;
	frame.header.length = 8;
	frame.payload = {0x2F, 0x03, 0x20, 0x02, 0x01, 0x00, 0x00, 0x00};

	return interface->sendCanFrame(frame);
}

bool MD80Downloader::sendInitCmd(Command initCommand)
{
	ICommunication::CANFrame frame{};
	frame.header.canId = canIdCommand;
	frame.header.length = 5;
	frame.payload = {static_cast<uint8_t>(initCommand)};
	serialize(bootAddress, &frame.payload[1]);

	return sendFrameWaitForResponse(frame, Response::HOST_INIT_OK, 500);
}
bool MD80Downloader::doSendFirmware(std::span<const uint8_t> firmwareData)
{
	const size_t ivSize = 16;
	auto it = firmwareData.begin();
	const uint8_t frameSize = interface->getSettings().baudrate == 1000000 ? 8 : 64;
	const size_t pageSize = 2048;
	Mode mode = Mode::SAFE;

	ICommunication::CANFrame frame{};
	frame.header.canId = canIdCommand;
	frame.header.length = frameSize;

	size_t size = firmwareData.size() - ivSize;

	if (size > maxFirmwareSizeSafeUpdate)
		mode = Mode::UNSAFE;

	size_t remainingSize = firmwareData.size();
	logger->debug("Binary size: {}", size);

	/* send 16 byte IV first */
	frame.header.length = 8;
	for (size_t i = 0; i < ivSize / frame.header.length; i++)
	{
		std::copy(it, it + frame.header.length, frame.payload.begin());
		if (!sendFrameWaitForResponse(frame, Response::CHUNK_OK, 100))
			return false;
		it += frame.header.length;
	}

	auto itLastCrc = it;
	size_t sentSize = 0;

	/* then proceed with the rest of the firmware*/
	for (size_t i = 0; i <= size; i += frameSize)
	{
		frame.payload = {0xff};
		size_t copySize = remainingSize >= frameSize ? frameSize : remainingSize;
		std::copy(it, it + copySize, frame.payload.begin());
		it += copySize;
		sentSize += copySize;

		if (!sendFrameWaitForResponse(frame, Response::CHUNK_OK, 100))
		{
			logger->error("Sending data failed!");
			return false;
		}

		if (sentSize >= pageSize || (size - i < frameSize))
		{
			FlasherCommons::progressBar(static_cast<float>(i) / static_cast<float>(size));

			ICommunication::CANFrame frame{};
			frame.header.canId = canIdCommand;
			frame.header.length = 5;
			frame.payload[0] = static_cast<uint8_t>(Command::CHECK_CRC);
			auto calculatedCrc = Checksum::crc32(&*itLastCrc, it - itLastCrc);

			serialize(calculatedCrc, &frame.payload[1]);

			if (!sendFrameWaitForResponse(frame, Response::CRC_OK, 100))
			{
				logger->error("CRC check failed!");
				return false;
			}

			if (mode == Mode::UNSAFE)
			{
				if (!sendWriteCmd())
				{
					logger->error("Programming failed!");
					return false;
				}
			}
			itLastCrc = it;
			sentSize = 0;
		}
	}

	FlasherCommons::progressBar(1.0f);

	if (mode == Mode::SAFE)
	{
		if (!sendWriteCmd())
		{
			logger->error("Programming failed!");
			return false;
		}
	}

	return true;
}

bool MD80Downloader::sendFirmware(std::span<const uint8_t> firmwareData)
{
	bool result = doSendFirmware(firmwareData);
	if (!result)
		std::cout << std::endl;
	return result;
}

bool MD80Downloader::sendWriteCmd()
{
	ICommunication::CANFrame frame{};
	frame.header.canId = canIdCommand;
	frame.header.length = 5;
	frame.payload = {static_cast<uint8_t>(Command::PROG)};

	return sendFrameWaitForResponse(frame, Response::PROG_OK, 3000);
}
bool MD80Downloader::sendBootCmd()
{
	ICommunication::CANFrame frame{};
	frame.header.canId = canIdCommand;
	frame.header.length = 5;
	frame.payload = {static_cast<uint8_t>(Command::BOOT)};

	return sendFrameWaitForResponse(frame, Response::BOOT_OK, 100);
}
