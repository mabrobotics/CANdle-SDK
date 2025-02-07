#pragma once

#include <string>
#include <exception>

#include <mab_types.hpp>
#include <logger.hpp>
#include <I_communication_interface.hpp>
#include <libusb.h>

namespace mab
{
    class LibusbDevice
    {
        libusb_device*           m_dev;
        libusb_device_handle**   m_devHandle;
        libusb_device_descriptor m_desc;
        Logger                   m_Log = Logger(Logger::ProgramLayer_E::BOTTOM, "LIBUSB");

      public:
        LibusbDevice(libusb_device* device) : m_dev(device)
        {
            if (device == nullptr)
            {
                std::string message = "Empty libusb device provided to handler!";
                m_Log.error(message.c_str());
                throw std::runtime_error(message);
            }
            if (libusb_get_device_descriptor(m_dev, &m_desc))
            {
                m_Log.error("Error while getting USB descriptor from the device!");
            }
            libusb_error usbOpenError = libusb_open(m_dev, m_devHandle);
            if (usbOpenError)
            {
                std::string message;
                switch (usbOpenError)
                {
                    case LIBUSB_ERROR_NO_MEM:
                        message = "Could not allocate memory to USB device!";
                        m_Log.error(message.c_str());
                        break;

                    case LIBUSB_ERROR_ACCESS:
                        message = "Invalid permissions to access the USB device!";
                        m_Log.error(message.c_str());
                        break;

                    case LIBUSB_ERROR_NO_DEVICE:
                        message = "USB device not connected!";
                        m_Log.error(message.c_str());
                        break;
                    default:
                        break;
                }
            }
        }
    };

    static constexpr int inEndpointAdr  = 0x81;  ///< CANdle USB input endpoint address.
    static constexpr int outEndpointAdr = 0x01;  ///< CANdle USB output endpoint address.
    class USBv2 : I_CommunicationInterface
    {
      private:
        Logger m_Log = Logger(Logger::ProgramLayer_E::BOTTOM, "USB");

        struct libusb_device_handle* devh = nullptr;

      public:
        explicit USBv2(const u16 vid, const u16 pid, const std::string requestID);
        ~USBv2();

        Error_t connect() override;
        Error_t disconnect() override;

        std::pair<std::vector<u8>, Error_t> transfer(
            std::vector<u8>                         data,
            std::optional<u32>                      timeout,
            std::optional<std::function<Error_t()>> receiverCallback) override;
    };
}  // namespace mab