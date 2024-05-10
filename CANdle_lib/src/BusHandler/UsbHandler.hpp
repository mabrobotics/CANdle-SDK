/**
 * @file USBHandler.hpp
 * @author Piotr Wasilewski (piotr.wasilewski@mabrobotics.pl)
 * @brief
 * @version 0.1
 * @date 2024-03-06
 *
 * @copyright Copyright (c) 2024
 *
 */
#ifndef _USBHANDLER_HPP
#define _USBHANDLER_HPP

#include <atomic>
#include <memory>
#include <set>
#include <span>
#include <thread>

#include "Barrier.hpp"
#include "CircularBuffer.hpp"
#include "IBusHandler.hpp"
#include "spdlog/spdlog.h"

/**
 * @brief Handles the low level USB communication using libusb
 *
 */

class UsbHandler : public IBusHandler
{
   public:
	UsbHandler(std::shared_ptr<spdlog::logger> logger);
	~UsbHandler();

	/**
	 * @brief Initializes the USB device with VID PID.
	 *
	 * @return true
	 * @return false
	 */

	bool init() override;
	/**
	 * @brief Deinitializes the USB device and libusb.
	 *
	 * @return true
	 * @return false
	 */

	bool deinit() override;
	/**
	 * @brief Get an element from RX FIFO.
	 *
	 * @return std::optional<IBusHandler::BusFrame>
	 */

	std::optional<IBusHandler::BusFrame> getFromFifo() const override;
	/**
	 * @brief Add new element to TX FIFO.
	 *
	 * @param busFrame new busFrame frame to be added and sent.
	 * @return true
	 * @return false
	 */

	bool addToFifo(BusFrame& busFrame) override;
	/**
	 * @brief Flush both member FIFO.
	 *
	 */
	void resetFifos() override;

	/**
	 * @brief Initialize with VID PID. Opened devices are stored in \ref openedDevices.
	 *
	 * @param vid vendor id of the device.
	 * @param pid product id of the device.
	 * @param manualMode if set to true, the transmit/receive thread is not launched.
	 * @param deviceNotFoundError set to false to avoid errors when the device was not found. Useful when the program is waiting for bootloader/main fw USB stack.
	 * @return true
	 * @return false
	 */
	bool init(uint16_t vid, uint16_t pid, bool manualMode = false, bool deviceNotFoundError = true);

	/**
	 * @brief Send data without the background thread.
	 *
	 * Send data without FIFO and automatic transmit/receive thread running in the background. Used for legacy USB bootloader in CANdle.
	 * @param data data to be sent.
	 * @return true
	 * @return false
	 */
	bool sendDataDirectly(std::span<uint8_t> data);

	/**
	 * @brief Receive data without the background thread.
	 *
	 * Receive data without the FIFO and automatic transmit/receive thread running in the background. Used for legacy USB bootloader in CANdle.
	 * @param data reference to std::span to which data will be received.
	 * @return true
	 * @return false
	 */
	bool receiveDataDirectly(std::span<uint8_t>& data);

   private:
	/**
	 * @brief Max size of the single transfer "bus" buffer.
	 *
	 * The 2049 size is to allow for a 1 extra byte so that a transfer is not ended within the USB 1ms cycle. See the end of copyFifoToOutputBuffer method.
	 */
	static constexpr uint32_t size = 4097;
	static constexpr uint16_t VID = 0x0069;
	static constexpr uint16_t PID = 0x1000;
	static constexpr int inEndpointAdr = 0x81;	 ///< CANdle USB input endpoint address.
	static constexpr int outEndpointAdr = 0x01;	 ///< CANdle USB output endpoint address.

	static constexpr uint32_t sendTimeoutMs = 100;
	static constexpr uint32_t receiveTimeoutMs = 100;

	struct libusb_device** devs;
	struct libusb_device_handle* devh = NULL;
	static std::set<struct libusb_device*> openedDevices;

	CircularBuffer<BusFrame, 50> toUsbBuffer;	 ///< transmit FIFO.
	CircularBuffer<BusFrame, 50> fromUsbBuffer;	 ///< receive FIFO.

	bool isInitialized = false;
	Barrier syncPoint;

	std::atomic<bool> done;
	std::atomic<bool> cycleCompleted = false;
	std::thread handlerThread;

	std::shared_ptr<spdlog::logger> logger;

   private:
	/**
	 * @brief Transmit/receive thread for libusb data.
	 *
	 */
	void dataHandler();
	/**
	 * @brief Used to copy individual BusFrame elements from USB buffer to RX FIFO.
	 *
	 * @param buf array of received data.
	 * @param receiveLen length of actually received, valid data.
	 */
	void copyInputBufToElements(std::array<uint8_t, size>& buf, int receiveLen);

	/**
	 * @brief Used to copy TX FIFO elements to USB output buffer.
	 *
	 * @param buf array to be transfered.
	 * @param sendLen length of valid data in the buffer.
	 */
	void copyElementsToOutputBuf(std::array<uint8_t, size>& buf, uint32_t& sendLen);
};

#endif
