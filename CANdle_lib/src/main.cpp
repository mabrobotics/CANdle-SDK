#include <cstring>
#include <iostream>
#include <memory>
#include <mutex>
#include <thread>
#include <algorithm>
#include <array>
#include "UsbHandler.hpp"
#include "main.hpp"
#include "Commons/Deserializer.hpp"

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

struct SettingsFrame
{
	uint32_t baudrate;
	uint32_t fdFormat;
	uint32_t bitRateSwitch;
};

#pragma pack(pop)

std::unique_ptr<IBusHandler> busHandler;

IBusHandler::BusFrame makeBusFrameFromCan(const CANFrame &canFrame)
{
	IBusHandler::BusFrame usbFrame{};
	usbFrame.header.id = 0x01;
	usbFrame.header.length = sizeof(CANFrame::Header) + canFrame.header.length;
	auto canFrameArray = std::bit_cast<std::array<uint8_t, sizeof(CANFrame)>>(canFrame);
	std::copy(canFrameArray.begin(), canFrameArray.begin() + usbFrame.header.length, usbFrame.payload.begin());
	return usbFrame;
}

void processDataThread()
{

	std::array<uint32_t, 10> stats{1};

	while (1)
	{
		auto frame = busHandler->getFromFifo();

		if (!frame.has_value())
			continue;

		if (frame->header.id == 0x01)
		{
			auto it = frame->payload.begin();
			auto canHeader = getHeaderFromArray<CANFrame::Header>(it);
			it += sizeof(canHeader);

			// std::cout << "ID: " << (int)canHeader.canId << " Len: " << (int)canHeader.length << " Data: ";
			// for (int i = 0; i < canHeader.length; i++)
			// 	std::cout << std::hex << " 0x" << (int)*it++ << " ";
			// std::cout << std::endl;
		}
		else if (frame->header.id == 0x02)
		{
			auto status = deserialize<StatusFrame>(frame->payload.begin());
			std::cout << "CAN Status: " << (int)status.statistics.averageRxFifoOccupancyPercent << " "
					  << (int)status.statistics.averageTxFifoOccupancyPercent << " "
					  << (int)status.statistics.maxRxFifoOccupancyPercent << " "
					  << (int)status.statistics.maxTxFifoOccupancyPercent << " "
					  << (int)status.busStatus << std::endl;
		}
	}
}

void commandDataThread()
{
	CANFrame SYNC{};
	SYNC.header.canId = 0x080;
	SYNC.header.length = 0;

	CANFrame canFrame{};
	canFrame.header.canId = 0x601;
	canFrame.header.length = 8;
	canFrame.payload = {0x40, 0xC5, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00};

	CANFrame canFrame2{};
	canFrame2.header.canId = 0x602;
	canFrame2.header.length = 8;
	canFrame2.payload = {0x40, 0x64, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00};

	std::array<CANFrame, 12> RPDOs{};
	std::array<IBusHandler::BusFrame, 12> RPDOFrames{};

	for (uint32_t i = 0; i < RPDOs.size(); i++)
	{
		RPDOs[i].header.canId = 0x400 + i + 1;
		RPDOs[i].header.length = 12;

		RPDOFrames[i] = makeBusFrameFromCan(RPDOs[i]);
	}

	IBusHandler::BusFrame usbFrame{};
	usbFrame.header.id = 0x03;
	usbFrame.header.length = 1;
	usbFrame.payload.at(0) = 0x03;
	busHandler->addToFifo(usbFrame);

	IBusHandler::BusFrame usbSettingsFrame{};
	usbSettingsFrame.header.id = 0x04;
	usbSettingsFrame.header.length = sizeof(SettingsFrame);
	SettingsFrame settings{};
	settings.baudrate = 8000000;
	settings.bitRateSwitch = 0x00100000U;
	settings.fdFormat = 0x00200000U;
	serialize(settings, usbSettingsFrame.payload.begin());
	busHandler->addToFifo(usbSettingsFrame);

	auto startTime = std::chrono::steady_clock::now();
	const std::chrono::microseconds interval(2000);

	auto sync = makeBusFrameFromCan(SYNC);

	while (1)
	{
		auto elapsedTime = std::chrono::steady_clock::now() - startTime;

		if (elapsedTime > interval)
		{
			startTime = std::chrono::steady_clock::now();

			for (uint32_t i = 0; i < 12; i++)
				busHandler->addToFifo(RPDOFrames[i]);

			busHandler->addToFifo(sync);
		}
	}
}

int main(int argc, char **argv)
{

	busHandler = std::make_unique<UsbHandler>();
	busHandler->init();

	std::thread process(processDataThread);
	std::thread command(commandDataThread);

	while (1)
	{
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}

	process.join();
	command.join();

	return 0;
}
