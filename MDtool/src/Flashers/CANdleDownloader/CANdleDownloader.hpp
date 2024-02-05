#ifndef CANDLEDOWNLOADER_H
#define CANDLEDOWNLOADER_H

#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>
#include <span>
#include <thread>

#include "BusHandler/UsbHandler.hpp"
#include "Communication/CandleInterface.hpp"
#include "spdlog/spdlog.h"

class CANdleDownloader
{
   public:
	enum BootloaderFrameId
	{
		NONE = 0,
		CHECK_ENTERED = 100,
		SEND_PAGE = 101,
		WRITE_PAGE = 102,
		BOOT_TO_APP = 103,
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

	CANdleDownloader(spdlog::logger* logger);
	~CANdleDownloader();

	Status doLoad(std::span<const uint8_t>&& firmwareData, bool recover);

   private:
	std::unique_ptr<UsbHandler> usbHandler;
	spdlog::logger* logger;

	std::atomic<bool> done = false;
	std::thread receiveThread;

	std::atomic<bool> response = false;
	std::atomic<uint8_t> expectedId = BootloaderFrameId::NONE;

   private:
	void receiveHandler();
	bool waitForActionWithTimeout(std::function<bool()> condition, uint32_t timeoutMs);

	bool sendResetCmd();
	bool sendInitCmd();
	bool sendPageCmd(std::span<const uint8_t> payload);
	bool sendFirmware(std::span<const uint8_t> firmwareData);
	bool sendCheckCRCAndWriteCmd(std::span<const uint8_t> firmwareChunk);
	bool sendBootCmd();
};

#endif