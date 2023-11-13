#ifndef CANDLE_HPP
#define CANDLE_HPP

#include <iostream>
#include <memory>
#include <optional>

#include "Commons/Deserializer.hpp"
#include "IBusHandler.hpp"
#include "ICommunication.hpp"

class Candle : public ICommunication
{
   public:
	Candle(std::unique_ptr<IBusHandler> busHandler);

	bool setupInterface(SettingsFrame& settings) override;
	bool sendCanFrame(const CANFrame& canFrame) override;
	std::optional<CANFrame> receiveCanFrame() override;

   private:
	std::unique_ptr<IBusHandler> busHandler;
};

#endif