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
	CandleInterface(std::unique_ptr<IBusHandler> busHandler);

	bool setupInterface(SettingsFrame& settings) override;
	bool sendCanFrame(const CANFrame& canFrame) override;
	std::optional<CANFrame> receiveCanFrame() override;

   private:
	std::unique_ptr<IBusHandler> busHandler;
};

#endif