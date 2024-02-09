#include "UsbHandler.hpp"

#include <algorithm>
#include <iostream>

#include "Commons/BitCast.hpp"
#include "libusb.h"

UsbHandler::UsbHandler(std::shared_ptr<spdlog::logger> logger) : logger(logger), syncPoint(2)
{
}

UsbHandler::~UsbHandler()
{
	deinit();
}

bool UsbHandler::init(uint16_t vid, uint16_t pid, bool manualMode, bool deviceNotFoundError)
{
	if (isInitialized)
		return true;

	int rc = libusb_init(NULL);
	if (rc < 0)
	{
		logger->error("Error initializing libusb: {}", libusb_error_name(rc));
		return false;
	}

	devh = libusb_open_device_with_vid_pid(NULL, vid, pid);

	if (!devh)
	{
		if (deviceNotFoundError)
			logger->error("Error finding USB device");
		return false;
	}

	for (int if_num = 0; if_num < 2; if_num++)
	{
		if (libusb_kernel_driver_active(devh, if_num))
			libusb_detach_kernel_driver(devh, if_num);

		rc = libusb_claim_interface(devh, if_num);

		if (rc < 0)
			logger->error("Error claiming interface: {}", libusb_error_name(rc));
	}

	if (!manualMode)
	{
		handlerThread = std::thread(&UsbHandler::dataHandler, this);
		syncPoint.wait();
	}

	isInitialized = true;
	return true;
}

bool UsbHandler::init()
{
	return init(VID, PID);
}

bool UsbHandler::deinit()
{
	if (!isInitialized)
		return true;

	done = true;
	if (handlerThread.joinable())
		handlerThread.join();

	if (devh)
	{
		libusb_release_interface(devh, 0);
		libusb_close(devh);
	}

	libusb_exit(NULL);

	isInitialized = false;
	return true;
}

std::optional<IBusHandler::BusFrame> UsbHandler::getFromFifo() const
{
	return fromUsbBuffer.get();
}

bool UsbHandler::addToFifo(BusFrame& busFrame)
{
	if (toUsbBuffer.full())
		return false;

	toUsbBuffer.put(busFrame);
	return true;
}

void UsbHandler::resetFifos()
{
	toUsbBuffer.reset();
	fromUsbBuffer.reset();
}

void UsbHandler::dataHandler()
{
	static constexpr uint32_t size = 1025;
	std::array<uint8_t, size> txBuf;
	std::array<uint8_t, size> rxBuf;

	uint32_t sendLen = 0;
	int receivedLen = 0;
	int sendLenActual = 0;

	syncPoint.wait();

	while (!done)
	{
		copyElementsToOutputBuf(txBuf, sendLen);

		int ret = libusb_bulk_transfer(devh, outEndpointAdr, txBuf.data(), sendLen, &sendLenActual, sendTimeout);

		if (ret < 0)
			logger->error("Error while sending: {}", libusb_strerror(static_cast<libusb_error>(ret)));

		ret = libusb_bulk_transfer(devh, inEndpointAdr, rxBuf.data(), size, &receivedLen, receiveTimeout);

		if (ret < 0)
			logger->error("Error while receiving {}  transferred {}", libusb_strerror(static_cast<libusb_error>(ret)), receivedLen);
		else
			copyInputBufToElements(rxBuf, receivedLen);
	}
}

void UsbHandler::copyInputBufToElements(std::array<uint8_t, 1025>& buf, int receiveLen)
{
	auto it = buf.begin();

	while (*it != 0x00 && (it - buf.begin()) < receiveLen)
	{
		BusFrame usbEntry{};
		/* prepare and copy USB header*/
		std::array<uint8_t, sizeof(BusFrame::Header)> usbHeaderArray;
		std::copy(it, it + usbHeaderArray.size(), usbHeaderArray.begin());
		usbEntry.header = bit_cast_<BusFrame::Header>(usbHeaderArray);
		it += usbHeaderArray.size();
		/* prepare and copy USB payload */
		std::copy(it, it + usbEntry.header.length, usbEntry.payload.begin());
		it += usbEntry.header.length;

		fromUsbBuffer.put(usbEntry);
	}

	buf.fill(0);
}

void UsbHandler::copyElementsToOutputBuf(std::array<uint8_t, 1025>& buf, uint32_t& sendLen)
{
	auto it = buf.begin();
	buf.fill(0);

	while (it < buf.end())
	{
		auto elem = toUsbBuffer.get();
		if (!elem.has_value())
			break;

		auto usbEntry = elem.value();
		auto usbFrameArray = bit_cast_<std::array<uint8_t, sizeof(BusFrame)>>(usbEntry);

		uint8_t usbFrameSize = usbEntry.header.length + sizeof(BusFrame::Header);
		auto copied = std::copy(usbFrameArray.begin(), usbFrameArray.begin() + usbFrameSize, it);
		it += std::distance(it, copied);
	}

	sendLen = std::distance(buf.begin(), it);

	if (sendLen < 64)
		sendLen = 65;
	else if (sendLen % 64 == 0)
		sendLen++;
}

bool UsbHandler::sendDataDirectly(std::span<uint8_t> data)
{
	/* TODO check sendLenActual */
	int sendLenActual = 0;
	int ret = libusb_bulk_transfer(devh, outEndpointAdr, data.data(), data.size(), &sendLenActual, sendTimeout);

	if (ret < 0)
	{
		logger->error("Error while sending: {}", libusb_strerror(static_cast<libusb_error>(ret)));
		return false;
	}
	return true;
}

bool UsbHandler::receiveDataDirectly(std::span<uint8_t>& data)
{
	/* TODO check receivedLen */
	int receivedLen = 0;
	int ret = libusb_bulk_transfer(devh, inEndpointAdr, data.data(), data.size(), &receivedLen, receiveTimeout);

	if (ret < 0)
	{
		logger->error("Error while receiving {}  transferred {}", libusb_strerror(static_cast<libusb_error>(ret)), receivedLen);
		return false;
	}
	return true;
}