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
	explicit CandleInterface(IBusHandler* busHandler);

	bool setupInterface(Settings& settings) override;
	Settings getSettings() const override;
	bool sendCanFrame(const CANFrame& canFrame) override;
	std::optional<CANFrame> receiveCanFrame() override;

   private:
	IBusHandler* busHandler;
	Settings settings;
};

#endif