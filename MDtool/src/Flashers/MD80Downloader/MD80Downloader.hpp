#ifndef MD80DOWNLOADER_H
#define MD80DOWNLOADER_H

#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>
#include <span>
#include <thread>

#include "Communication/CandleInterface.hpp"
#include "IBusHandler.hpp"
#include "spdlog/spdlog.h"

class MD80Downloader
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

	enum class Mode : uint8_t
	{
		UNSAFE = 0,
		SAFE = 1,
	};

	MD80Downloader(std::shared_ptr<ICommunication> interface, std::shared_ptr<spdlog::logger> logger);
	~MD80Downloader();

	Status doLoad(std::span<const uint8_t>&& firmwareData, uint32_t id, uint8_t channel, bool recover, uint32_t address, bool secondaryBootloader);

   private:
	static constexpr uint32_t BASE_CMD_ID = 0x680;
	static constexpr uint32_t BASE_RESP_ID = 0x780;
	static constexpr uint32_t BACKDOOR_CMD_ID = 0x002;
	static constexpr uint32_t BACKDOOR_RESP_ID = 0x003;
	static constexpr uint32_t maxFirmwareSizeSafeUpdate = 20480;

	uint32_t deviceId = 1;
	uint32_t bootAddress = 0x8005000;
	uint8_t channel = 0;

	std::shared_ptr<ICommunication> interface;
	std::shared_ptr<spdlog::logger> logger;

	uint32_t canIdCommand = BASE_CMD_ID;
	uint32_t canIdResponse = BASE_RESP_ID;

	std::atomic<bool> done = false;
	std::thread receiveThread;

	std::atomic<Response> lastResponse = Response::NONE;

   private:
	void receiveHandler();
	bool waitForActionWithTimeout(std::function<bool()> condition, uint32_t timeoutMs);
	bool sendFrameWaitForResponse(ICommunication::CANFrame& frame, Response expectedResponse, uint32_t timeout);

	bool sendResetCmd();
	bool sendInitCmd(Command initCommand);
	bool doSendFirmware(std::span<const uint8_t> firmwareData);
	bool sendFirmware(std::span<const uint8_t> firmwareData);
	bool sendWriteCmd();
	bool sendBootCmd();
};

#endif