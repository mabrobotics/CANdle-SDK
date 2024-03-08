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
		GET_HARDWARE_INFO = 5,
	};

	struct FirmwareInfo
	{
		std::array<uint8_t, 8> commitHash;
		uint32_t buildDate;
		uint32_t firmwareVersion;
	};

	struct HardwareInfo
	{
		std::array<uint8_t, 24> UUID;
		std::array<uint8_t, 24> chipId;
		uint32_t flashSize;
		uint8_t packageType;
		uint8_t hardwareVersion;
	};

	explicit CandleInterface(std::unique_ptr<IBusHandler> busHandler);

	/**
	 * @brief Inits the Candle interface with given settings.
	 *
	 * @param settings \ref Settings object.
	 * @return true
	 * @return false
	 */
	bool init(Settings& settings) override;
	/**
	 * @brief Deinitializes busHandler.
	 *
	 * @return true
	 * @return false
	 */
	bool deinit() override;
	/**
	 * @brief Returns Settings object
	 *
	 * @return Settings
	 */
	Settings getSettings() const override;
	/**
	 * @brief Sends CAN frame using busHandler
	 *
	 * @param canFrame
	 * @return true
	 * @return false
	 */
	bool sendCanFrame(const CANFrame& canFrame) override;

	/**
	 * @brief Receives CANFrame using busHandler
	 *
	 * @return std::optional<CANFrame>
	 */
	std::optional<CANFrame> receiveCanFrame() override;

	/**
	 * @brief Get number of active CAN channels
	 *
	 * @return uint8_t number of channels
	 */
	uint8_t getCanChannels() override;

	/**
	 * @brief Get the Candle status
	 *
	 * @return Status
	 */
	Status getStatus() const override;

	/**
	 * @brief reset Candle statistics
	 *
	 * @return true
	 * @return false
	 */
	bool reset() override;

	uint32_t getFirmwareVersion() const override
	{
		return firmwareInfo.firmwareVersion;
	}
	uint32_t getBuildDate() const override
	{
		return firmwareInfo.buildDate;
	}
	std::array<uint8_t, 8> getCommitHash() const override
	{
		return firmwareInfo.commitHash;
	}
	uint8_t getHardwareVersion() const override
	{
		return hardwareInfo.hardwareVersion;
	}
	std::array<uint8_t, 24> getUUID() const
	{
		return hardwareInfo.UUID;
	}

   private:
	Settings settings{};
	Status status{};
	FirmwareInfo firmwareInfo{};
	HardwareInfo hardwareInfo{};

	std::atomic<bool> newResponse = false;
	std::unique_ptr<IBusHandler> busHandler;

   private:
	/**
	 * @brief Send settings frame to the Candle device.
	 *
	 * @param settings_
	 * @return true
	 * @return false
	 */
	bool sendSettingsFrame(const Settings& settings_);
	/**
	 * @brief Send command frame to the Candle device.
	 *
	 * @param cmd \ref Command
	 * @return true
	 * @return false
	 */
	bool sendCommandFrame(Command cmd);

	/**
	 * @brief Processes the Candle device response
	 *
	 * @tparam Iterator
	 * @param responseForCommand \ref Command ID to which the response is made
	 * @param it iterator for the data buffer
	 */
	template <typename Iterator>
	void processCommandResponse(Command responseForCommand, Iterator it);

	/**
	 * @brief Wait for a specific function to complete with timeout.
	 *
	 * @param condition std::function returning bool. While it returns false, the function is blocking.
	 * @param timeoutMs timeout in milliseconds.
	 * @return true
	 * @return false
	 */
	bool waitForActionWithTimeout(std::function<bool()> condition, uint32_t timeoutMs);
};

#endif