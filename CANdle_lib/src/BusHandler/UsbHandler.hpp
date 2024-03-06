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

class UsbHandler : public IBusHandler
{
   public:
	UsbHandler(std::shared_ptr<spdlog::logger> logger);
	~UsbHandler();

	/**
	 * @brief Initializes the USB device with VID PID
	 *
	 * @return true
	 * @return false
	 */

	bool init() override;
	/**
	 * @brief Deinitializes the USB device and libusb
	 *
	 * @return true
	 * @return false
	 */

	bool deinit() override;
	/**
	 * @brief Get an element from RX fifo
	 *
	 * @return std::optional<IBusHandler::BusFrame>
	 */

	std::optional<IBusHandler::BusFrame> getFromFifo() const override;
	/**
	 * @brief Add new element to TX fifo
	 *
	 * @param busFrame
	 * @return true
	 * @return false
	 */

	bool addToFifo(BusFrame& busFrame) override;
	/**
	 * @brief Flush both member fifos
	 *
	 */
	void resetFifos() override;

	/**
	 * @brief Initialize with VID PID. Opened devices are stored in \ref openedDevices
	 *
	 * @param vid
	 * @param pid
	 * @param manualMode if set to true, the transmit/receive thread is not launched.
	 * @param deviceNotFoundError set to false to avoid errors when the device was not found. Useful when the program is waiting for bootloader/main fw USB stack.
	 * @return true
	 * @return false
	 */
	bool init(uint16_t vid, uint16_t pid, bool manualMode = false, bool deviceNotFoundError = true);

	/**
	 * @brief Send data without the fifo and automatic transmit/receive thread running in the background. Used for legacy USB bootloader in CANdle.
	 *
	 * @param data data to be sent
	 * @return true
	 * @return false
	 */
	bool sendDataDirectly(std::span<uint8_t> data);

	/**
	 * @brief Receive data without the fifo and automatic transmit/receive thread running in the background. Used for legacy USB bootloader in CANdle.
	 *
	 * @param data reference to std::span to which data will be received
	 * @return true
	 * @return false
	 */
	bool receiveDataDirectly(std::span<uint8_t>& data);

   private:
	/**
	 * @brief the 2049 size is to allow for a 1 extra byte so that a transfer is not ended within the USB  1ms cycle see the end of copyFifoToOutputBuffer method
	 *
	 */
	static constexpr uint32_t size = 2049;
	static constexpr uint16_t VID = 0x0069;
	static constexpr uint16_t PID = 0x1000;
	static constexpr int inEndpointAdr = 0x81;
	static constexpr int outEndpointAdr = 0x01;

	static constexpr uint32_t sendTimeout = 100;
	static constexpr uint32_t receiveTimeout = 100;

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

	static std::set<struct libusb_device*> openedDevices;

   private:
	/**
	 * @brief Transmit/receive thread for libusb data
	 *
	 */
	void dataHandler();
	/**
	 * @brief Used to copy individual BusFrame elements from USB buffer to RX fifo.
	 *
	 * @param buf  array of received data
	 * @param receiveLen length of actually received, valid data
	 */
	void copyInputBufToElements(std::array<uint8_t, size>& buf, int receiveLen);

	/**
	 * @brief Used to copy TX fifo elements to USB output buffer.
	 *
	 * @param buf array to be transfered
	 * @param sendLen length of valid data in the buffer
	 */
	void copyElementsToOutputBuf(std::array<uint8_t, size>& buf, uint32_t& sendLen);
};

#endif