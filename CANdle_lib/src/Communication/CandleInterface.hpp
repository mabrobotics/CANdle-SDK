#ifndef CANDLEINTERFACE_HPP
#define CANDLEINTERFACE_HPP

#include <atomic>
#include <functional>
#include <iostream>
#include <memory>
#include <optional>
#include <vector>

#include "Commons/Deserializer.hpp"
#include "IBusHandler.hpp"
#include "ICommunication.hpp"

class CandleInterface : public ICommunication
{
   public:
	enum BusFrameId
	{
		CANFRAME = 1,
		STATUS = 2,
		COMMAND = 3,
		CHANGE_CAN_SETTINGS = 4,
		COMMAND_RESPONSE = 5,
		/* legacy */
		ENTER_BOOTLOADER = 10,
	};

	enum Command : uint8_t
	{
		RESET = 1,
		CLEAR_ERRORS = 2,
		RESET_STATISTICS = 3,
		GET_FIRMWARE_INFO = 4,
	};

	struct FirmwareInfo
	{
		std::array<uint8_t, 8> commitHash;
		uint32_t buildDate;
		uint32_t firmwareVersion;
	};

	explicit CandleInterface(std::unique_ptr<IBusHandler> busHandler);

	bool init(Settings& settings) override;
	bool deinit() override;
	Settings getSettings() const override;
	bool sendCanFrame(const CANFrame& canFrame) override;
	std::optional<CANFrame> receiveCanFrame() override;

	Status getStatus() const override;
	bool reset() override;

	uint32_t getFirmwareVersion() const
	{
		return firmwareInfo.firmwareVersion;
	}
	uint32_t getBuildDate() const
	{
		return firmwareInfo.buildDate;
	}
	std::array<uint8_t, 8> getCommitHash() const
	{
		return firmwareInfo.commitHash;
	}

   private:
	Settings settings{};
	Status status{};
	FirmwareInfo firmwareInfo{};

	std::atomic<bool> newResponse = false;
	std::unique_ptr<IBusHandler> busHandler;

   private:
	bool sendSettingsFrame(const Settings& settings_);
	bool sendCommandFrame(Command cmd);

	template <typename Iterator>
	void processCommandResponse(Command responseForCommand, Iterator it);
	bool waitForActionWithTimeout(std::function<bool()> condition, uint32_t timeoutMs);
};

#endif