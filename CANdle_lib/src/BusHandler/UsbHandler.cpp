#include "UsbHandler.hpp"
#include "Commons/BitCast.hpp"
#include <algorithm>
#include <iostream>

UsbHandler::~UsbHandler()
{
    done = true;
    if (handlerThread.joinable())
        handlerThread.join();

    libusb_release_interface(devh, 0);
    if (devh)
        libusb_close(devh);
    libusb_exit(NULL);
}

bool UsbHandler::init()
{
    int rc = libusb_init(NULL);
    if (rc < 0)
    {
        fprintf(stderr, "Error initializing libusb: %s\n", libusb_error_name(rc));
        return false;
    }

    devh = libusb_open_device_with_vid_pid(NULL, VID, PID);

    if (!devh)
    {
        fprintf(stderr, "Error finding USB device\n");
        return false;
    }

    for (int if_num = 0; if_num < 2; if_num++)
    {
        if (libusb_kernel_driver_active(devh, if_num))
            libusb_detach_kernel_driver(devh, if_num);

        rc = libusb_claim_interface(devh, if_num);

        if (rc < 0)
            fprintf(stderr, "Error claiming interface: %s\n", libusb_error_name(rc));
    }

    handlerThread = std::thread(&UsbHandler::dataHandler, this);

    return true;
}

std::optional<IBusHandler::BusFrame> UsbHandler::getFromFifo() const
{
    return fromUsbBuffer.get();
}

bool UsbHandler::addToFifo(BusFrame &busFrame)
{
    if (toUsbBuffer.full())
        return false;
    toUsbBuffer.put(busFrame);
    return true;
}

void UsbHandler::dataHandler()
{
    static constexpr uint32_t size = 1025;
    std::array<uint8_t, size> txBuf;
    std::array<uint8_t, size> rxBuf;

    uint32_t sendLen = 0;
    int receivedLen = 0;
    int sendLenActual = 0;

    while (!done)
    {
        copyElementsToOutputBuf(txBuf, sendLen);

        if (int ret = libusb_bulk_transfer(devh, outEndpointAdr, txBuf.data(), sendLen, &sendLenActual, 10) < 0)
            fprintf(stderr, "Error while sending %d \n", ret);

        if (int ret = libusb_bulk_transfer(devh, inEndpointAdr, rxBuf.data(), size, &receivedLen, 1000) < 0)
            fprintf(stderr, "Error while receiving %d  transferred %d\n", ret, receivedLen);
        else
            copyInputBufToElements(rxBuf, receivedLen);
    }
}

void UsbHandler::copyInputBufToElements(std::array<uint8_t, 1025> &buf, int receiveLen)
{
    auto it = buf.begin();

    while (*it != 0x00 && (it - buf.begin()) < receiveLen)
    {
        BusFrame usbEntry{};
        /* prepare and copy USB header*/
        std::array<uint8_t, sizeof(BusFrame::Header)> usbHeaderArray;
        std::copy(it, it + usbHeaderArray.size(), usbHeaderArray.begin());
        usbEntry.header = bit_cast<BusFrame::Header>(usbHeaderArray);
        it += usbHeaderArray.size();
        /* prepare and copy USB payload */
        std::copy(it, it + usbEntry.header.length, usbEntry.payload.begin());
        it += usbEntry.header.length;

        fromUsbBuffer.put(usbEntry);
    }

    buf.fill(0);
}

void UsbHandler::copyElementsToOutputBuf(std::array<uint8_t, 1025> &buf, uint32_t &sendLen)
{
    auto it = buf.begin();
    buf.fill(0);

    while (it < buf.end())
    {
        auto elem = toUsbBuffer.get();
        if (!elem.has_value())
            break;

        auto usbEntry = elem.value();
        auto usbFrameArray = bit_cast<std::array<uint8_t, sizeof(BusFrame)>>(usbEntry);

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