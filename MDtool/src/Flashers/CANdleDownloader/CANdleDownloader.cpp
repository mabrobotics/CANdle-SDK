#include "CANdleDownloader.hpp"

#include <algorithm>
#include <functional>
#include <thread>
#include <utility>

#include "BusHandler/IBusHandler.hpp"
#include "Checksum/Checksum.hpp"

CANdleDownloader::CANdleDownloader(IBusHandler* busHandler, spdlog::logger* logger) : busHandler(busHandler), logger(logger)
{
	receiveThread = std::thread(&CANdleDownloader::receiveHandler, this);
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
		if (!sendResetCmd())
			return Status::ERROR_RESET;

		logger->debug("Reset OK");
	}

	// bool success = false;
	// auto initCommand = secondaryBootloader ? Command::HOST_INIT_SECONDARY : Command::HOST_INIT;

	// if (secondaryBootloader)
	// 	bootAddress = 0x8000000;

	// while (recover && !success)
	// 	success = sendInitCmd(initCommand);

	// if (!recover)
	// {
	// 	for (int i = 0; i < 10; i++)
	// 	{
	// 		success = sendInitCmd(initCommand);
	// 		if (success)
	// 			break;
	// 	}
	// }

	// if (!success)
	// 	return Status::ERROR_INIT;

	// logger->debug("Init OK");

	// if (!sendFirmware(firmwareData))
	// 	return Status::ERROR_FIRMWARE;

	// logger->debug("Firmware OK");

	// if (!sendBootCmd())
	// 	return Status::ERROR_BOOT;

	logger->debug("Boot OK");
	return Status::OK;
}

void CANdleDownloader::receiveHandler()
{
	while (!done)
	{
		auto frame = busHandler->getFromFifo();

		if (!frame.has_value())
			continue;

		// auto canFrame = frame.value();
		// if (canFrame.header.canId == canIdResponse && canFrame.header.length == 5)
		// 	lastResponse = static_cast<Response>(canFrame.payload[0]);
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

bool CANdleDownloader::sendFrameWaitForResponse(ICommunication::CANFrame& frame, Response expectedResponse, uint32_t timeout)
{
	return true;
}

bool CANdleDownloader::sendResetCmd()
{
	logger->debug("Resetting...");

	IBusHandler::BusFrame frame{};
	frame.header.id = 10;
	frame.header.length = 1;
	return busHandler->addToFifo(frame);
}

bool CANdleDownloader::sendInitCmd(Command initCommand)
{
	return true;
}
