#include <USB_v2.hpp>

namespace mab
{
    //----------------------------LIBUSB-DEVICE-SECTION---------------------------------------------

    LibusbDevice::LibusbDevice(libusb_device* device, s32 inEndpointAddress, s32 outEndpointAddress)
        : m_dev(device),
          m_inEndpointAddress(inEndpointAddress),
          m_outEndpointAddress(outEndpointAddress)
    {
        if (m_dev == nullptr)
        {
            std::string message = "Empty libusb device provided to handler!";
            m_log.error(message.c_str());
            throw std::runtime_error(message);
        }
        if (libusb_get_device_descriptor(m_dev, &m_desc))
        {
            m_log.error("Error while getting USB descriptor from the device!");
        }
        if (libusb_get_active_config_descriptor(m_dev, &m_config))
        {
            m_log.error("Error while getting USB config from the device!");
        }

        m_log.info("Connected device has %d interfaces", m_config->bNumInterfaces);

        libusb_error usbOpenError = static_cast<libusb_error>(libusb_open(m_dev, m_devHandle));
        if (usbOpenError)
        {
            std::string message;
            switch (usbOpenError)
            {
                case LIBUSB_ERROR_NO_MEM:
                    message = "Could not allocate memory to USB device!";
                    break;

                case LIBUSB_ERROR_ACCESS:
                    message = "Invalid permissions to access the USB device!";
                    break;

                case LIBUSB_ERROR_NO_DEVICE:
                    message = "USB device not connected!";
                    break;
                default:
                    message = "Unknown error";
                    break;
            }
            m_log.error(message.c_str());
        }

        for (u32 interfaceNo = 0; interfaceNo < m_config->bNumInterfaces; interfaceNo++)
        {
            if (libusb_kernel_driver_active(*m_devHandle, interfaceNo))
                libusb_detach_kernel_driver(*m_devHandle, interfaceNo);

            libusb_error usbClaimError =
                static_cast<libusb_error>(libusb_claim_interface(*m_devHandle, interfaceNo));
            if (usbClaimError)
            {
                std::string message;
                switch (usbClaimError)
                {
                    case LIBUSB_ERROR_NOT_FOUND:
                        message = "Requested interface does not exist!";
                        break;
                    case LIBUSB_ERROR_BUSY:
                        message = "Another driver has claimed this interface!";
                        break;
                    case LIBUSB_ERROR_NO_DEVICE:
                        message = "USB device has been disconnected!";
                    default:
                        message = "Unknown error";
                        break;
                }
                m_log.error(message.c_str());
            }
        }

        m_log.info("Connected USB device");
    }
    LibusbDevice::~LibusbDevice()
    {
        for (u32 interfaceNo = 0; interfaceNo < m_config->bNumInterfaces; interfaceNo++)
        {
            libusb_error usbReleaseError =
                static_cast<libusb_error>(libusb_release_interface(*m_devHandle, interfaceNo));
            if (usbReleaseError)
            {
                std::string message;
                switch (usbReleaseError)
                {
                    case LIBUSB_ERROR_NOT_FOUND:
                        message = "Requested interface does not exist!";
                        break;
                    case LIBUSB_ERROR_NO_DEVICE:
                        message = "USB device has been disconnected!";
                    default:
                        message = "Unknown error";
                        break;
                }
                m_log.error(message.c_str());
            }
            libusb_attach_kernel_driver(*m_devHandle, interfaceNo);
        }
        libusb_close(*m_devHandle);
        m_log.info("Disconnected USB device");
    }

    libusb_error LibusbDevice::transmit(u8* data, size_t length, u32 timeout)
    {
        if (data == nullptr)
        {
            std::string message = "Data does not exist!";
            m_log.error(message.c_str());
            throw std::runtime_error(message);
        }
        return static_cast<libusb_error>(
            libusb_bulk_transfer(*m_devHandle, m_outEndpointAddress, data, length, NULL, timeout));
    }
    libusb_error LibusbDevice::receive(u8* data, size_t length, u32 timeout)
    {
        if (data == nullptr)
        {
            std::string message = "Data does not exist!";
            m_log.error(message.c_str());
            throw std::runtime_error(message);
        }
        if (length == 0)
            m_log.warn("Requesting emtpy receive!");
        static_cast<libusb_error>(
            libusb_bulk_transfer(*m_devHandle, m_inEndpointAddress, data, length, NULL, timeout));
    }

    //----------------------------USB-DEVICE-SECTION---------------------------------------------
    USBv2::USBv2(const u16 vid, const u16 pid, const std::string serialNo)
        : m_vid(vid), m_pid(pid), m_serialNo(serialNo)
    {
    }
    USBv2::~USBv2()
    {
        disconnect();
    }

    USBv2::Error_t USBv2::connect()
    {
        if (libusb_init(NULL))
            m_Log.error("Could not init libusb!");

        libusb_device** deviceList    = nullptr;
        s32             deviceListLen = libusb_get_device_list(NULL, &deviceList);
        if (deviceListLen == 0)
            m_Log.error("No USB devices detected!");
        else if (deviceListLen < 0)
            m_Log.error("Libusb error while detecting devices!");

        // TODO: add multiple device handling
        for (s32 deviceIndex; deviceIndex < deviceListLen; deviceIndex++)
        {
            libusb_device*           device = deviceList[deviceIndex];
            libusb_device_descriptor usbDescriptor;
            if (libusb_get_device_descriptor(device, &usbDescriptor))
                m_Log.warn("Error while reading USB device descriptor!");
            if (usbDescriptor.idVendor == m_vid && usbDescriptor.idProduct == m_pid)
            {
                u8 serialNo[64];
                // TODO: wrap it under the device handler
                if (libusb_get_string_descriptor_ascii(
                        device, usbDescriptor.iSerialNumber, serialNo, sizeof(serialNo)))
                    m_Log.warn("Error while reading USB device serial number!");
                m_Log.info("Found right device %s", serialNo)
            }
        }

        libusb_free_device_list(deviceList, true);
    }
    USBv2::Error_t USBv2::disconnect()
    {
    }
}  // namespace mab