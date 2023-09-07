#ifndef _USBHANDLER_HPP
#define _USBHANDLER_HPP

#include "IBusHandler.hpp"
#include "libusb.h"
#include "CircularBuffer.hpp"
#include <atomic>
#include <thread>

class UsbHandler : public IBusHandler
{

public:
    ~UsbHandler();
    bool init() override;
    std::optional<IBusHandler::BusFrame> getFromFifo() const override;
    bool addToFifo(BusFrame &busFrame) override;

private:
    static constexpr int VID = 105;
    static constexpr int PID = 4096;
    static constexpr int inEndpointAdr = 0x81;
    static constexpr int outEndpointAdr = 0x01;

    std::atomic<bool> done;
    struct libusb_device_handle *devh = NULL;
    CircularBuffer<BusFrame, 50> toUsbBuffer;
    CircularBuffer<BusFrame, 50> fromUsbBuffer;
    std::thread handlerThread;

private:
    void dataHandler();
    void copyInputBufToElements(std::array<uint8_t, 1025> &buf, int receiveLen);
    void copyElementsToOutputBuf(std::array<uint8_t, 1025> &buf, uint32_t &sendLen);
};

#endif