#include "usbDevice.hpp"

#include <cstring>
#include <iostream>

#include "bus.hpp"
#include <libusb.h>

#define USB_VERBOSE 1

unsigned long hash(const char* str);

UsbDevice::UsbDevice(u16 vid, u16 pid, const std::vector<u32>& idsToIgnore, const std::string& id)
{
    m_log.m_tag                 = "USB";
    m_log.m_layer               = Logger::ProgramLayer_E::BOTTOM;
    busType                     = mab::BusType_E::USB;
    struct libusb_device** devs = nullptr;

    int rc = libusb_init(NULL);
    if (rc < 0)
    {
        m_log << "Failed to init libusb!" << endl;
        throw "Failed to init libusb!";
    }
    int32_t cnt = libusb_get_device_list(NULL, &devs);
    for (int i = 0; i < cnt; i++)
    {
        libusb_device*                  dev = devs[i];
        struct libusb_device_descriptor desc;

        rc = libusb_get_device_descriptor(dev, &desc);
        if (desc.idVendor != vid || desc.idProduct != pid)
            continue;
        rc = libusb_open(dev, &devh);
        u8 sNum[64];
        rc = libusb_get_string_descriptor_ascii(devh, desc.iSerialNumber, sNum, sizeof(sNum));
        u32  hashedId              = hash((char*)sNum);
        u32  hashedRequestedId     = strtoul(id.c_str(), nullptr, 16);
        bool shouldContinue        = false;
        bool shouldBreak           = false;
        bool isSpecificIdRequested = (hashedRequestedId > 0);
        for (u32 ignoreId : idsToIgnore)
        {
            if (ignoreId == hashedRequestedId)
            {
                m_log.error("Device with requested ID: %d is already created! Quitting!", id);
                shouldBreak = true;
                throw("Device with ID " + id + " already created!");
                break;
            }
            if (ignoreId == hashedId)
            {
                shouldContinue = true;
                break;
            }
        }
        if (shouldContinue)
            continue;
        if (shouldBreak)
            break;
        if (isSpecificIdRequested && hashedRequestedId != hashedId)
            continue;

        serialDeviceId = hashedId;
        for (int if_num = 0; if_num < 2; if_num++)
        {
            if (libusb_kernel_driver_active(devh, if_num))
                libusb_detach_kernel_driver(devh, if_num);
            serialDeviceId = hashedId;
            for (int if_num = 0; if_num < 2; if_num++)
            {
                if (libusb_kernel_driver_active(devh, if_num))
                    libusb_detach_kernel_driver(devh, if_num);

                rc = libusb_claim_interface(devh, if_num);
                if (rc < 0)
                {
                    m_log.error("Failed to claim interface !");
                    throw "Failed to claim libusb interface!";
                }
            }
            break;
        }
        libusb_free_device_list(devs, 1);
        if (devh == nullptr)
        {
            m_log.error("CANdle not found on USB bus!");
            return;
        }
        else
        {
            m_isConnected = true;
        }
    }
}

UsbDevice::~UsbDevice()
{
    if (m_isConnected)
    {
        libusb_reset_device(devh);
        libusb_release_interface(devh, 0);
        libusb_close(devh);
        libusb_exit(nullptr);
    }
}

bool UsbDevice::isConnected()
{
    return m_isConnected;
}

bool UsbDevice::reconnect(u16 vid, u16 pid)
{
    (void)vid;
    (void)pid;
    libusb_release_interface(devh, 0);
    libusb_release_interface(devh, 1);
    libusb_close(devh);
    libusb_exit(nullptr);
    devh = nullptr;

    m_isConnected = false;
    int rc        = libusb_init(NULL);
    if (rc < 0)
    {
        m_log.error("Failed to init libusb!");
        throw "Failed to init libusb!";
    }
    struct libusb_device** devs = nullptr;
    int32_t                cnt  = libusb_get_device_list(NULL, &devs);
    for (int i = 0; i < cnt; i++)
    {
        libusb_device*                  dev = devs[i];
        struct libusb_device_descriptor desc;

        rc = libusb_get_device_descriptor(dev, &desc);
        if (desc.idVendor != vid || desc.idProduct != pid)
            continue;
        rc = libusb_open(dev, &devh);
        if (rc < 0)
        {
            m_log.error("Failed to open device!");
            throw "Failed to open device!";
        }
        for (int if_num = 0; if_num < 2; if_num++)
        {
            if (libusb_kernel_driver_active(devh, if_num))
                libusb_detach_kernel_driver(devh, if_num);

            rc = libusb_claim_interface(devh, if_num);
            if (rc < 0)
            {
                m_log.error("Failed to claim interface!");
                throw "Failed to claim libusb interface!";
            }
        }
        break;
    }
    libusb_free_device_list(devs, 1);
    if (devh == nullptr)
    {
        m_log.error("CANdle not found on USB bus!");
        // throw "Device not found in USB bus!";
        return false;
    }
    else
    {
        m_isConnected = true;
    }

    return true;
}

bool UsbDevice::transmit(
    char* buffer, int len, bool waitForResponse, int timeout, int responseLen, bool faultVerbose)
{
    (void)faultVerbose;
    memset(rxBuffer, 0, sizeof(rxBuffer));
    s32 sendLenActual = 0;
    s32 ret = libusb_bulk_transfer(devh, outEndpointAdr, (u8*)buffer, len, &sendLenActual, 200);
    if (ret < LIBUSB_SUCCESS)
    {
        m_log.error("Failed to transmit! [ libusb error %d ]", ret);
        return false;
    }

    if (waitForResponse)
    {
        if (receive(responseLen, timeout))
            return true;
        else
        {
            m_log.warn("USB Receive timeout");
            return false;
        }
    }

    return true;
}

bool UsbDevice::receive(int responseLen, int timeoutMs, bool checkCrc, bool faultVerbose)
{
    (void)checkCrc;
    (void)faultVerbose;
    s32 ret = libusb_bulk_transfer(
        devh, inEndpointAdr, (u8*)rxBuffer, responseLen, &bytesReceived, timeoutMs);
    if (ret < 0)
        return false;
    return true;
}

unsigned long UsbDevice::getId()
{
    return serialDeviceId;
}

unsigned long hash(const char* str)
{
    unsigned long hash = 5381;
    int           c;
    while ((c = *str++))
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    return hash;
}

// Search USB buses for multiple instances of the save (pid, vid) device.
// Returns number of devices found
u32 searchMultipleDevicesOnUSB(u16 pid, u16 vid)
{
    struct libusb_device** devs;

    u32 nDevices = 0;
    libusb_init(nullptr);
    u32 allDevices = libusb_get_device_list(nullptr, &devs);
    for (u32 i = 0; i < allDevices; i++)
    {
        struct libusb_device_descriptor desc;
        u32                             nDevices = 0;
        libusb_init(nullptr);
        u32 allDevices = libusb_get_device_list(nullptr, &devs);
        for (u32 i = 0; i < allDevices; i++)
        {
            struct libusb_device_descriptor desc;

            s32 ret = libusb_get_device_descriptor(devs[i], &desc);
            if (ret < 0)
                continue;
            if (desc.idProduct == pid && desc.idVendor == vid)
                nDevices++;
        }
        libusb_exit(nullptr);
        return nDevices;
        s32 ret = libusb_get_device_descriptor(devs[i], &desc);
        if (ret < 0)
            continue;
        if (desc.idProduct == pid && desc.idVendor == vid)
            nDevices++;
    }
    libusb_exit(nullptr);
    return nDevices;
}
