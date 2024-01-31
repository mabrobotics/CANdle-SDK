#include "CandleInterface.hpp"

#include <Deserializer.hpp>
#include <algorithm>
#include <chrono>
#include <utility>

CandleInterface::CandleInterface(IBusHandler* busHandler)
{
	this->busHandler = busHandler;
}

bool CandleInterface::init(Settings& settings)
{
	this->settings = settings;

	if (!busHandler->init())
		return false;

	if (!sendSettingsFrame(settings))
		return false;

	if (!sendCommandFrame(Command::GET_FIRMWARE_INFO))
		return false;

	if (!waitForActionWithTimeout([&]() -> bool
								  { return newResponse == false; },
								  10))
		return false;

	return true;
}

bool CandleInterface::deinit()
{
	return busHandler->deinit();
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
	usbFrame.header.length = 1;
	usbFrame.payload[0] = static_cast<uint8_t>(cmd);
	newResponse = false;
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
	else if (frame->header.id == BusFrameId::COMMAND_RESPONSE)
	{
		newResponse = true;
		processCommandResponse(static_cast<Command>(frame->payload[0]), frame->payload.begin() + 1);
	}

	return std::nullopt;
}

template <typename Iterator>
void CandleInterface::processCommandResponse(Command responseForCommand, Iterator it)
{
	switch (responseForCommand)
	{
		case Command::GET_FIRMWARE_INFO:
		{
			firmwareInfo = deserialize<FirmwareInfo>(it);
			break;
		}
		default:
			break;
	}
}

bool CandleInterface::waitForActionWithTimeout(std::function<bool()> condition, uint32_t timeoutMs)
{
	auto start_time = std::chrono::high_resolution_clock::now();
	auto end_time = start_time + std::chrono::milliseconds(timeoutMs);

	while (condition())
	{
		if (std::chrono::high_resolution_clock::now() >= end_time)
			return false;
	}
	return true;
}
