#ifndef _USBHANDLER_HPP
#define _USBHANDLER_HPP

#include <atomic>
#include <memory>
#include <span>
#include <thread>

#include "Barrier.hpp"
#include "CircularBuffer.hpp"
#include "IBusHandler.hpp"
#include "spdlog/spdlog.h"

class UsbHandler : public IBusHandler
{
   public:
	UsbHandler(std::shared_ptr<spdlog::logger> logger);
	~UsbHandler();

	bool init() override;
	bool deinit() override;
	std::optional<IBusHandler::BusFrame> getFromFifo() const override;
	bool addToFifo(BusFrame& busFrame) override;
	void resetFifos() override;

	bool init(uint16_t vid, uint16_t pid, bool manualMode = false, bool deviceNotFoundError = true);
	bool sendDataDirectly(std::span<uint8_t> data);
	bool receiveDataDirectly(std::span<uint8_t>& data);

   private:
	static constexpr uint16_t VID = 0x0069;
	static constexpr uint16_t PID = 0x1000;
	static constexpr int inEndpointAdr = 0x81;
	static constexpr int outEndpointAdr = 0x01;

	static constexpr uint32_t sendTimeout = 15;
	static constexpr uint32_t receiveTimeout = 1000;

	struct libusb_device** devs;
	struct libusb_device_handle* devh = NULL;
	std::atomic<bool> done;
	CircularBuffer<BusFrame, 50> toUsbBuffer;
	CircularBuffer<BusFrame, 50> fromUsbBuffer;
	std::thread handlerThread;
	std::shared_ptr<spdlog::logger> logger;
	bool isInitialized = false;
	Barrier syncPoint;
	std::atomic<bool> cycleCompleted = false;

   private:
	void dataHandler();
	void copyInputBufToElements(std::array<uint8_t, 1025>& buf, int receiveLen);
	void copyElementsToOutputBuf(std::array<uint8_t, 1025>& buf, uint32_t& sendLen);
};

#endif