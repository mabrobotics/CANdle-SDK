#ifndef CANDLEDOWNLOADER_H
#define CANDLEDOWNLOADER_H

#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>
#include <span>
#include <thread>

#include "Communication/CandleInterface.hpp"
#include "IBusHandler.hpp"
#include "spdlog/spdlog.h"

class CANdleDownloader
{
   public:
	enum class Command : uint8_t
	{
		HOST_INIT = 0xA0,
		HOST_INIT_SECONDARY = 0xA9,
		PROG = 0xA1,
		BOOT = 0xA2,
		CHECK_CRC = 0xA3
	};

	enum class Response : uint8_t
	{
		NONE = 0x00,
		HOST_INIT_OK = 0xB0,
		PROG_OK = 0xB1,
		BOOT_OK = 0xB2,
		CRC_OK = 0xB3,
		CHUNK_OK = 0xB4,
	};

	enum class Status : uint8_t
	{
		OK = 0,
		ERROR_RESET = 1,
		ERROR_INIT = 2, /*!< will be set when decription fails */
		ERROR_FIRMWARE = 3,
		ERROR_PROG = 4,
		ERROR_BOOT = 5,
	};

	CANdleDownloader(IBusHandler* busHandler, spdlog::logger* logger);
	~CANdleDownloader();

	Status doLoad(std::span<const uint8_t>&& firmwareData, bool recover);

   private:
	IBusHandler* busHandler;
	spdlog::logger* logger;

	std::atomic<bool> done = false;
	std::thread receiveThread;

   private:
	void receiveHandler();
	bool waitForActionWithTimeout(std::function<bool()> condition, uint32_t timeoutMs);
	bool sendFrameWaitForResponse(ICommunication::CANFrame& frame, Response expectedResponse, uint32_t timeout);

	bool sendResetCmd();
	bool sendInitCmd(Command initCommand);
};

#endif