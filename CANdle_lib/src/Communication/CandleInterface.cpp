#include "CandleInterface.hpp"

#include <algorithm>
#include <utility>

CandleInterface::CandleInterface(IBusHandler* busHandler) : busHandler(busHandler)
{
}

bool CandleInterface::setupInterface(Settings& settings)
{
	this->settings = settings;

	if (!busHandler->init())
		return false;

	if (!sendSettingsFrame(settings))
		return false;

	if (!sendCommandFrame(Command::RESET_STATISTICS))
		return false;

	return true;
}

ICommunication::Settings CandleInterface::getSettings() const
{
	return settings;
}

bool CandleInterface::sendCanFrame(const CANFrame& canFrame)
{
	IBusHandler::BusFrame usbFrame{};
	usbFrame.header.id = BusFrameId::CANFRAME;
	usbFrame.header.length = sizeof(CANFrame::Header) + canFrame.header.length;
	auto canFrameArray = bit_cast_<std::array<uint8_t, sizeof(CANFrame)>>(canFrame);
	std::copy(canFrameArray.begin(), canFrameArray.begin() + usbFrame.header.length, usbFrame.payload.begin());
	return busHandler->addToFifo(usbFrame);
}

ICommunication::Status CandleInterface::getStatus() const
{
	return status;
}

bool CandleInterface::reset()
{
	return sendCommandFrame(Command::RESET_STATISTICS);
}

bool CandleInterface::sendSettingsFrame(const Settings& settings_)
{
	IBusHandler::BusFrame usbSettingsFrame{};
	usbSettingsFrame.header.id = BusFrameId::CHANGE_CAN_SETTINGS;
	usbSettingsFrame.header.length = sizeof(Settings);
	serialize(settings_, usbSettingsFrame.payload.begin());
	return busHandler->addToFifo(usbSettingsFrame);
}

bool CandleInterface::sendCommandFrame(Command cmd)
{
	IBusHandler::BusFrame usbFrame{};
	usbFrame.header.id = BusFrameId::COMMAND;
	usbFrame.header.length = static_cast<uint8_t>(cmd);
	return busHandler->addToFifo(usbFrame);
}

std::optional<ICommunication::CANFrame> CandleInterface::receiveCanFrame()
{
	auto frame = busHandler->getFromFifo();

	if (!frame.has_value())
		return std::nullopt;

	if (frame->header.id == BusFrameId::CANFRAME)
	{
		auto it = frame->payload.begin();
		auto canHeader = deserialize<CANFrame::Header>(it);
		it += sizeof(canHeader);

		// std::cout << "ID: " << (int)canHeader.canId << " Len: " << (int)canHeader.length << " Data: ";
		// for (int i = 0; i < canHeader.length; i++)
		// 	std::cout << std::hex << " 0x" << (int)*(it + i) << " ";
		// std::cout << std::endl;

		CANFrame canFrame{};
		canFrame.header = canHeader;
		std::copy(it, it + canHeader.length, canFrame.payload.begin());
		return canFrame;
	}
	else if (frame->header.id == BusFrameId::STATUS)
	{
		status = deserialize<Status>(frame->payload.begin());
		// std::cout << "CAN Status: " << (int)status.statistics.averageRxFifoOccupancyPercent << " "
		// 		  << (int)status.statistics.averageTxFifoOccupancyPercent << " "
		// 		  << (int)status.statistics.maxRxFifoOccupancyPercent << " "
		// 		  << (int)status.statistics.maxTxFifoOccupancyPercent << " "
		// 		  << (int)status.busStatus << std::endl;
	}

	return std::nullopt;
}