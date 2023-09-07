#include <cstring>
#include <iostream>
#include <memory>
#include <mutex>
#include <thread>
#include <array>
#include "UsbHandler.hpp"
#include "main.hpp"

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

#pragma pack(pop)

std::unique_ptr<IBusHandler> busHandler;

IBusHandler::BusFrame makeBusFrameFromCan(CANFrame &canFrame)
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

			std::cout << "ID: " << (int)canHeader.canId << " Len: " << (int)canHeader.length << " Data: ";

			for (int i = 0; i < canHeader.length; i++)
				std::cout << std::hex << " 0x" << (int)*it++ << " ";
			std::cout << std::endl;
		}
	}
}

void commandDataThread()
{
	CANFrame canFrame;
	canFrame.header.canId = 0x601;
	canFrame.header.length = 8;
	canFrame.payload = {0x40, 0xC5, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00};

	CANFrame canFrame2;
	canFrame2.header.canId = 0x602;
	canFrame2.header.length = 8;
	canFrame2.payload = {0x40, 0x64, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00};

	CANFrame RPDO3_1;
	RPDO3_1.header.canId = 0x402;
	RPDO3_1.header.length = 6;
	RPDO3_1.payload = {0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

	CANFrame RPDO3_2;
	RPDO3_2.header.canId = 0x401;
	RPDO3_2.header.length = 6;
	RPDO3_2.payload = {0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

	while (1)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1));

		auto frame1 = makeBusFrameFromCan(canFrame);
		auto frame2 = makeBusFrameFromCan(canFrame2);

		auto frame3 = makeBusFrameFromCan(RPDO3_1);
		auto frame4 = makeBusFrameFromCan(RPDO3_2);

		busHandler->addToFifo(frame1);
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		busHandler->addToFifo(frame2);
		busHandler->addToFifo(frame3);
		busHandler->addToFifo(frame4);
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
