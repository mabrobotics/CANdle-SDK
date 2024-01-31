#ifndef _USBHANDLER_HPP
#define _USBHANDLER_HPP

#include <atomic>
#include <span>
#include <thread>

#include "CircularBuffer.hpp"
#include "IBusHandler.hpp"
#include "spdlog/spdlog.h"

class UsbHandler : public IBusHandler
{
   public:
	UsbHandler(spdlog::logger* logger);
	~UsbHandler();

	bool init() override;
	bool deinit() override;
	std::optional<IBusHandler::BusFrame> getFromFifo() const override;
	bool addToFifo(BusFrame& busFrame) override;

	bool init(uint16_t vid, uint16_t pid, bool manualMode = false);
	bool sendDataDirectly(std::span<uint8_t> data);
	bool receiveDataDirectly(std::span<uint8_t>& data);

   private:
	static constexpr uint16_t VID = 105;
	static constexpr uint16_t PID = 4096;
	static constexpr int inEndpointAdr = 0x81;
	static constexpr int outEndpointAdr = 0x01;
	struct libusb_device_handle* devh = NULL;
	std::atomic<bool> done;
	CircularBuffer<BusFrame, 50> toUsbBuffer;
	CircularBuffer<BusFrame, 50> fromUsbBuffer;
	std::thread handlerThread;
	spdlog::logger* logger;
	bool isInitialized = false;

   private:
	void dataHandler();
	void copyInputBufToElements(std::array<uint8_t, 1025>& buf, int receiveLen);
	void copyElementsToOutputBuf(std::array<uint8_t, 1025>& buf, uint32_t& sendLen);
};

#endif