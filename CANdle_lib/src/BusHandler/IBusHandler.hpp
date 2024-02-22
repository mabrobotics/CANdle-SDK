#ifndef _IBUSHANDLER_HPP
#define _IBUSHANDLER_HPP

#include <array>
#include <cstdint>
#include <optional>

class IBusHandler
{
   public:
#pragma pack(push, 4)
	struct BusFrame
	{
		struct Header
		{
			uint8_t id;
			uint8_t payloadSize;
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