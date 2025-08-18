#include <USB.hpp>
#include <cstring>
#include <string>

namespace mab
{
    static std::string translateLibusbError(libusb_error err)
    {
        switch (err)
        {
            case LIBUSB_SUCCESS:
                return "Success (no error)";
            case LIBUSB_ERROR_ACCESS:
                return "Access denied (insufficient permissions)";
            case LIBUSB_ERROR_BUSY:
                return "Resource busy, taken by other process";
            case LIBUSB_ERROR_INTERRUPTED:
                return "System call interrupted (perhaps due to signal)";
            case LIBUSB_ERROR_INVALID_PARAM:
                return "Invalid parameter.";
            case LIBUSB_ERROR_IO:
                return "Success (no error)";
            case LIBUSB_ERROR_NO_DEVICE:
                return "No such device (it may have been disconnected)";
            case LIBUSB_ERROR_NO_MEM:
                return "Insufficient memory.";
            case LIBUSB_ERROR_NOT_FOUND:
                return "Device not found.";
            case LIBUSB_ERROR_OTHER:
                return "Unknown error";
            case LIBUSB_ERROR_OVERFLOW:
                return "Overflow occurred";
            case LIBUSB_ERROR_PIPE:
                return "Control request not supported by the device";
            case LIBUSB_ERROR_NOT_SUPPORTED:
                return "Windows kernel related issue ocurred";
            case LIBUSB_ERROR_TIMEOUT:
                return "Timed out";
            default:
                return "Undefined error";
        }
    }
    //----------------------------LIBUSB-DEVICE-SECTION---------------------------------------------

    LibusbDevice::LibusbDevice(libusb_device* device,
                               const s32      inEndpointAddress,
                               const s32      outEndpointAddress,
                               const bool     peek)
        : m_dev(device),
          m_inEndpointAddress(inEndpointAddress),
          m_outEndpointAddress(outEndpointAddress),
          m_peek(peek)
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
        m_log.debug("Connected device has %d interfaces", m_config->bNumInterfaces);

        m_log.debug("Opening communication...");
        libusb_error usbOpenError = static_cast<libusb_error>(libusb_open(m_dev, &m_devHandle));
        if (usbOpenError)
        {
            std::string message;
            message = translateLibusbError(usbOpenError);
            message.insert(0, "On open: ");
            m_log.error(message.c_str());
        }
        if (m_devHandle == nullptr)
        {
            m_log.error("Did not receive a handle from libusb device!");
        }
        if (peek)
            return;
        for (u32 interfaceNo = 1; interfaceNo < m_config->bNumInterfaces; interfaceNo++)
        {
            m_log.debug("Detaching kernel drivers from interface no.: %d", interfaceNo);
            // dont detach any interface other than 0 as it causes errors
            if (libusb_kernel_driver_active(m_devHandle, 0))
                libusb_detach_kernel_driver(m_devHandle, 0);

            m_log.debug("Claiming interface no.: %d", interfaceNo);
            libusb_error usbClaimError =
                static_cast<libusb_error>(libusb_claim_interface(m_devHandle, interfaceNo));
            if (usbClaimError)
            {
                std::string message;
                message = translateLibusbError(usbClaimError);
                message.insert(0, "On claim: ");
                m_log.error(message.c_str());
                return;
            }

            m_connected = true;

            m_log.info(
                "Connected USB device: vid - %d, pid - %d", m_desc.idVendor, m_desc.idProduct);
        }
    }
    LibusbDevice::~LibusbDevice()
    {
        // if device was only used in discovery no interfaces are claimed
        if (!m_peek)
        {
            for (u32 interfaceNo = 1; interfaceNo < m_config->bNumInterfaces; interfaceNo++)
            {
                libusb_error usbReleaseError =
                    static_cast<libusb_error>(libusb_release_interface(m_devHandle, interfaceNo));
                if (usbReleaseError)
                {
                    std::string message;
                    message = translateLibusbError(usbReleaseError);
                    m_log.error(message.c_str());
                }
                if (!libusb_kernel_driver_active(m_devHandle, 0))
                    libusb_attach_kernel_driver(m_devHandle, 0);
            }
        }
        libusb_close(m_devHandle);
        m_log.info(
            "Disconnected USB device: vid - %d, pid - %d", m_desc.idVendor, m_desc.idProduct);
    }

    libusb_error LibusbDevice::transmit(u8* data, const size_t length, const u32 timeout)
    {
        if (data == nullptr)
        {
            std::string message = "Data does not exist!";
            m_log.error(message.c_str());
            throw std::runtime_error(message);
        }
        return static_cast<libusb_error>(
            libusb_bulk_transfer(m_devHandle, m_outEndpointAddress, data, length, NULL, timeout));
    }
    libusb_error LibusbDevice::receive(u8* data, const size_t length, const u32 timeout)
    {
        if (data == nullptr)
        {
            std::string message = "Data does not exist!";
            m_log.error(message.c_str());
            throw std::runtime_error(message);
        }
        if (length == 0)
            m_log.warn("Requesting emtpy receive!");
        std::memset(data, 0, length);

        int actualLen = 0;

        // rx buffer is bigger to protect from overflow condition
        // https://libusb.sourceforge.io/api-1.0/libusb_packetoverflow.html
        libusb_error err = static_cast<libusb_error>(libusb_bulk_transfer(m_devHandle,
                                                                          m_inEndpointAddress,
                                                                          m_rxBuffer.data(),
                                                                          m_rxBuffer.size(),
                                                                          &actualLen,
                                                                          timeout));

        if (actualLen != (int)length && actualLen != 66/*some kind of libusb hack with that frame length, jmatyszczak know more about it*/)
            m_log.warn("Received length of %d does not match expected length of %d bytes",
                       actualLen,
                       length);
        std::memcpy(data, m_rxBuffer.data(), length);

        return err;
    }

    libusb_error LibusbDevice::unclogInput()
    {
        return (libusb_error)libusb_clear_halt(m_devHandle, m_inEndpointAddress);
    }
    libusb_error LibusbDevice::unclogOutput()
    {
        return (libusb_error)libusb_clear_halt(m_devHandle, m_outEndpointAddress);
    }

    std::string LibusbDevice::getSerialNo()
    {
        std::array<u8, 13> serialNo{0};
        libusb_error       err = static_cast<libusb_error>(libusb_get_string_descriptor_ascii(
            m_devHandle, m_desc.iSerialNumber, serialNo.begin(), serialNo.size()));

        if (err < 0)
        {
            m_log.warn(("On getting serial no.: " + translateLibusbError(err)).c_str());
        }

        return std::string(serialNo.begin(), serialNo.end());
    }

    //----------------------------USB-DEVICE-SECTION---------------------------------------------
    USB::USB(const u16 vid, const u16 pid, const std::string serialNo) : m_vid(vid), m_pid(pid)
    {
        if (!serialNo.empty())
            m_serialNo = serialNo;

        if (libusb_init(&m_ctx))
            m_Log.error("Could not init libusb!");
        m_Log.debug("Init libusb for context %d: ", (size_t)m_ctx);
        // libusb_set_option(&m_ctx, libusb_option::LIBUSB_OPTION_LOG_LEVEL,
        // LIBUSB_LOG_LEVEL_DEBUG);
    }
    USB::~USB()
    {
        m_Log.debug("Deinit libusb for context %d: ", (size_t)m_ctx);
        disconnect();
        libusb_exit(m_ctx);
    }

    USB::Error_t USB::connect()
    {
        m_libusbDevice                = nullptr;
        libusb_device** deviceList    = nullptr;
        s32             deviceListLen = libusb_get_device_list(NULL, &deviceList);
        if (deviceListLen == 0)
            m_Log.error("No USB devices detected!");
        else if (deviceListLen < 0)
            m_Log.error("Libusb error while detecting devices!");

        m_Log.debug("Found %d USB devices", deviceListLen);

        m_Log.debug("Looking for VID: %d, PID: %d, Serial: %c", m_vid, m_pid, m_serialNo.c_str());

        for (s32 deviceIndex = 0; deviceIndex < deviceListLen; deviceIndex++)
        {
            libusb_device*           checkedDevice = deviceList[deviceIndex];
            libusb_device_descriptor checkedDescriptor;
            libusb_error             descError = static_cast<libusb_error>(
                libusb_get_device_descriptor(checkedDevice, &checkedDescriptor));

            if (descError)
                m_Log.warn(translateLibusbError(descError).c_str());

            m_Log.debug("Checking device: vid %d, pid %d",
                        checkedDescriptor.idVendor,
                        checkedDescriptor.idProduct);

            if (checkedDescriptor.idVendor == m_vid && checkedDescriptor.idProduct == m_pid)
            {
                m_Log.debug("Found the right device!");
                m_libusbDevice =
                    std::make_unique<LibusbDevice>(checkedDevice, IN_ENDPOINT, OUT_ENDPOINT, true);
                std::string serialNo = m_libusbDevice->getSerialNo();
                m_Log.info("Device with serial %s found", serialNo.c_str());

                if (m_serialNo.compare("") &&
                    m_serialNo.compare(serialNo.substr(0, m_serialNo.size())))
                {
                    m_Log.debug("This is not the device you are looking for");
                    m_Log.debug("%s vs %s",
                                m_serialNo.c_str(),
                                serialNo.substr(0, m_serialNo.size()).c_str());
                    m_libusbDevice = nullptr;
                    continue;
                }
                // without reasigning libusb device libusb library looses handle for some reason,
                // and we use it to claim the interface after scanning
                m_libusbDevice = nullptr;
                m_libusbDevice =
                    std::make_unique<LibusbDevice>(checkedDevice, IN_ENDPOINT, OUT_ENDPOINT);
                break;
            }
        }
        libusb_free_device_list(deviceList, true);
        if (m_libusbDevice == nullptr)
        {
            m_Log.error("Device was not found!");
            return Error_t::NOT_CONNECTED;
        }
        else if (!m_libusbDevice->isConnected())
        {
            m_Log.error("Device is not connected!");
            return Error_t::NOT_CONNECTED;
        }
        else
        {
            m_Log.info("Device connected");
            return Error_t::OK;
        }
    }
    USB::Error_t USB::disconnect()
    {
        if (m_libusbDevice == nullptr)
        {
            m_Log.info("Device already disconnected");
            return Error_t::NOT_CONNECTED;
        }
        m_libusbDevice = nullptr;
        return Error_t::OK;
    }

    USB::Error_t USB::transfer(std::vector<u8> data, const u32 timeoutMs)
    {
        auto ret = transfer(data, timeoutMs, 0);
        return ret.second;
    }

    std::pair<std::vector<u8>, USB::Error_t> USB::transfer(std::vector<u8> data,
                                                           const u32       timeoutMs,
                                                           const size_t    expectedReceivedDataSize)
    {
        if (m_libusbDevice == nullptr)
        {
            m_Log.error("Device not connected!");
            return std::pair(data, Error_t::NOT_CONNECTED);
        }
        if (data.size() > USB_MAX_BUFF_LEN)
        {
            m_Log.error("Data too long!");
            return std::pair(data, Error_t::DATA_TOO_LONG);
        }
        if (data.size() == 0)
        {
            m_Log.error("Data empty!");
            return std::pair(data, Error_t::DATA_EMPTY);
        }
        // This part forces libusb to perform at lesser latency due to usage of microframes
        // TODO: This needs a rework because of the bootloader
        // if (data.size() < 66)
        // {
        //     data.resize(66);
        // }
        libusb_error transmitError = m_libusbDevice->transmit(data.data(), data.size(), timeoutMs);
        if (transmitError != libusb_error::LIBUSB_SUCCESS)
        {
            std::string err = translateLibusbError(transmitError);
            m_Log.error(err.c_str());
            if (transmitError == libusb_error::LIBUSB_ERROR_PIPE)  // pipe clogged and needs a reset
            {
                m_libusbDevice->unclogInput();
            }
            return std::pair(data, Error_t::TRANSMITTER_ERROR);
        }

        if (expectedReceivedDataSize != 0)
        {
            std::vector<u8> recievedData;
            recievedData.resize(expectedReceivedDataSize);
            libusb_error receiveError =
                m_libusbDevice->receive(recievedData.data(), recievedData.size(), timeoutMs);
            if (receiveError != libusb_error::LIBUSB_SUCCESS)
            {
                std::string err = translateLibusbError(receiveError);
                m_Log.error(err.c_str());
                if (receiveError ==
                    libusb_error::LIBUSB_ERROR_PIPE)  // pipe clogged and needs a reset
                {
                    m_libusbDevice->unclogOutput();
                }
                return std::pair(data, Error_t::RECEIVER_ERROR);
            }
            return std::pair(recievedData, Error_t::OK);
        }
        return std::pair(data, Error_t::OK);
    }
}  // namespace mab
