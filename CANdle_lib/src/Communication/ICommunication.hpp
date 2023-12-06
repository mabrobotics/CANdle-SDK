#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include <array>
#include <cstdint>
#include <optional>

class ICommunication
{
   public:
#pragma pack(push, 4)
	struct CANFrame
	{
		struct Header
		{
			uint16_t canId;
			uint8_t length;
		} header;
		std::array<uint8_t, 64> payload;
	};

	struct StatusFrame
	{
		struct Statistics
		{
			uint8_t averageTxFifoOccupancyPercent;
			uint8_t averageRxFifoOccupancyPercent;
			uint8_t maxTxFifoOccupancyPercent;
			uint8_t maxRxFifoOccupancyPercent;
		} statistics;
		uint32_t busStatus;
	};

	struct Settings
	{
		uint32_t baudrate;
		uint32_t fdFormat;
		uint32_t bitRateSwitch;
	};
#pragma pack(pop)
	virtual ~ICommunication() = default;
	virtual bool setupInterface(Settings& settings) = 0;
	virtual Settings getSettings() const = 0;
	virtual bool sendCanFrame(const CANFrame& canFrame) = 0;
	virtual std::optional<CANFrame> receiveCanFrame() = 0;
};

#endif