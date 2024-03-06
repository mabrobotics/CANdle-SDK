/**
 * @file IBusHandler.hpp
 * @author Piotr Wasilewski (piotr.wasilewski@mabrobotics.pl)
 * @brief
 * @version 0.1
 * @date 2024-03-06
 *
 * @copyright Copyright (c) 2024
 *
 */
#ifndef _IBUSHANDLER_HPP
#define _IBUSHANDLER_HPP

#include <array>
#include <cstdint>
#include <optional>

class IBusHandler
{
   public:
#pragma pack(push, 4)

	/**
	 * @brief BusFrame describes a single FIFO frame that is then sent using the "bus" (currently only USB).
	 * The size of the payload should be large enough to hold a single CANFrame. Only a single frame, either a status, command or can frame can be placed in a single BusFrame.
	 */
	struct BusFrame
	{
		struct Header
		{
			uint8_t id;
			uint8_t payloadSize;
			uint32_t reserved1;
			uint32_t reserved2;
		} header;
		std::array<uint8_t, 100> payload;
	};
#pragma pack(pop)

	virtual ~IBusHandler() = default;

	virtual bool init() = 0;
	virtual bool deinit() = 0;
	virtual std::optional<BusFrame> getFromFifo() const = 0;
	virtual bool addToFifo(BusFrame& busFrame) = 0;
	virtual void resetFifos() = 0;
};

#endif