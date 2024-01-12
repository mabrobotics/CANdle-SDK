#ifndef CANDLEINTERFACE_HPP
#define CANDLEINTERFACE_HPP

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
		CHANGE_CAN_SETTINGS = 4
	};

	enum Command : uint8_t
	{
		RESET = 1,
		CLEAR_ERRORS = 2,
		RESET_STATISTICS = 3,
	};

	explicit CandleInterface(IBusHandler* busHandler);

	bool setupInterface(Settings& settings) override;
	Settings getSettings() const override;
	bool sendCanFrame(const CANFrame& canFrame) override;
	std::optional<CANFrame> receiveCanFrame() override;

	Status getStatus() const override;
	bool reset() override;

   private:
	IBusHandler* busHandler;
	Settings settings;
	Status status;

   private:
	bool sendSettingsFrame(const Settings& settings_);
	bool sendCommandFrame(Command cmd);
};

#endif