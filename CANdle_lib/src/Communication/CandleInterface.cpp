#include "CandleInterface.hpp"

#include <algorithm>
#include <utility>

CandleInterface::CandleInterface(std::unique_ptr<IBusHandler> busHandler) : busHandler(std::move(busHandler))
{
}

bool CandleInterface::setupInterface(SettingsFrame& settings)
{
	if (!busHandler->init())
		return false;

	IBusHandler::BusFrame usbSettingsFrame{};
	usbSettingsFrame.header.id = 0x04;
	usbSettingsFrame.header.length = sizeof(SettingsFrame);
	serialize(settings, usbSettingsFrame.payload.begin());
	return busHandler->addToFifo(usbSettingsFrame);
}

bool CandleInterface::sendCanFrame(const CANFrame& canFrame)
{
	IBusHandler::BusFrame usbFrame{};
	usbFrame.header.id = 0x01;
	usbFrame.header.length = sizeof(CANFrame::Header) + canFrame.header.length;
	auto canFrameArray = bit_cast_<std::array<uint8_t, sizeof(CANFrame)>>(canFrame);
	std::copy(canFrameArray.begin(), canFrameArray.begin() + usbFrame.header.length, usbFrame.payload.begin());
	return busHandler->addToFifo(usbFrame);
}

std::optional<ICommunication::CANFrame> CandleInterface::receiveCanFrame()
{
	auto frame = busHandler->getFromFifo();

	if (!frame.has_value())
		return std::nullopt;

	if (frame->header.id == 0x01)
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
		std::copy(it, it + 5, canFrame.payload.begin());
		return canFrame;
	}
	else if (frame->header.id == 0x02)
	{
		auto status = deserialize<StatusFrame>(frame->payload.begin());
		// std::cout << "CAN Status: " << (int)status.statistics.averageRxFifoOccupancyPercent << " "
		// 		  << (int)status.statistics.averageTxFifoOccupancyPercent << " "
		// 		  << (int)status.statistics.maxRxFifoOccupancyPercent << " "
		// 		  << (int)status.statistics.maxTxFifoOccupancyPercent << " "
		// 		  << (int)status.busStatus << std::endl;
	}

	return std::nullopt;
}