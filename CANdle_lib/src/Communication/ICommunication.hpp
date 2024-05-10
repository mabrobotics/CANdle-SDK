#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include <array>
#include <cstdint>
#include <optional>

#include "BusHandler/IBusHandler.hpp"

/**
 * @brief Interface for communicating with CAN dongles
 *
 */
class ICommunication
{
   public:
#pragma pack(push, 4)
	/**
	 * @brief CAN / CANFD frame structure
	 *
	 */
	struct CANFrame
	{
		/**
		 * @brief CAN / CANFD frame header structure
		 *
		 */
		struct Header
		{
			uint16_t canId;
			uint8_t payloadSize;
			uint8_t channel;
			uint32_t reserved1;
			uint32_t reserved2;
		} header;
		std::array<uint8_t, 64> payload;
	};

	enum CANChannel
	{
		CH0 = 0,  /**< Default channel 0 - only this channel can be used for MD80 bootloader recovery mode */
		CH1 = 1,  /**< Channel 1 */
		CH2 = 2,  /**< Channel 2 */
		ALL = 255 /**< All channels simultaneously */
	};

	/**
	 * @brief General CAN dongle status
	 *
	 */
	struct Status
	{
		/**
		 * @brief Communication FIFO statistics
		 *
		 */
		struct Statistics
		{
			uint8_t averageTxFifoOccupancyPercent;
			uint8_t averageRxFifoOccupancyPercent;
			uint8_t maxTxFifoOccupancyPercent;
			uint8_t maxRxFifoOccupancyPercent;
		} statistics;
		uint32_t busStatus;
	};

	/**
	 * @brief CAN dongle settings struct
	 *
	 */
	struct Settings
	{
		uint32_t baudrate;
	};

#pragma pack(pop)

	virtual ~ICommunication() = default;
	virtual bool init(Settings& settings) = 0;
	virtual bool deinit() = 0;
	virtual Settings getSettings() const = 0;
	virtual bool sendCanFrame(const CANFrame& canFrame) = 0;
	virtual std::optional<CANFrame> receiveCanFrame() = 0;
	virtual uint8_t getCanChannels() = 0;

	virtual Status getStatus() const = 0;
	virtual bool reset() = 0;

	virtual uint32_t getFirmwareVersion() const = 0;
	virtual uint32_t getBuildDate() const = 0;
	virtual std::array<uint8_t, 8> getCommitHash() const = 0;
	virtual uint8_t getHardwareVersion() const = 0;
};

#endif