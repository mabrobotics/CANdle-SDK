#pragma once

#include <string>
#include <exception>
#include <vector>
#include <utility>

#include <mab_types.hpp>
#include <logger.hpp>
#include <I_communication_interface.hpp>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
// /usr/include/libusb-1.0/libusb.h:52:33: error: ISO C++ forbids zero-size array
// ‘dev_capability_data’ [-Werror=pedantic]
//   52 | #define ZERO_SIZED_ARRAY        0       /* [0] - non-standard, but usually working code */
#include <libusb.h>
#pragma GCC diagnostic pop

namespace mab
{
    class LibusbDevice
    {
        libusb_device*            m_dev;
        libusb_device_handle**    m_devHandle;
        libusb_device_descriptor  m_desc;
        Logger                    m_log = Logger(Logger::ProgramLayer_E::BOTTOM, "USB_DEV");
        libusb_config_descriptor* m_config;

        s32 m_inEndpointAddress, m_outEndpointAddress;

      public:
        LibusbDevice() = delete;
        LibusbDevice(libusb_device* device, s32 inEndpointAddress, s32 outEndpointAddress);
        ~LibusbDevice();
        libusb_error transmit(u8* data, size_t length, u32 timeout);
        libusb_error receive(u8* data, size_t length, u32 timeout);
    };

    static constexpr int inEndpointAdr  = 0x81;  ///< CANdle USB input endpoint address.
    static constexpr int outEndpointAdr = 0x01;  ///< CANdle USB output endpoint address.
    class USBv2 : I_CommunicationInterface
    {
      private:
        Logger m_Log = Logger(Logger::ProgramLayer_E::BOTTOM, "USB");

        LibusbDevice m_libusbDevice;

        u16         m_vid, m_pid;
        std::string m_serialNo;

      public:
        explicit USBv2(const u16 vid, const u16 pid, const std::string serialNo = "");
        ~USBv2();

        Error_t connect() override;
        Error_t disconnect() override;

        std::pair<std::vector<u8>, Error_t> transfer(
            std::vector<u8>                         data,
            std::optional<u32>                      timeout,
            std::optional<std::function<Error_t()>> receiverCallback) override;
    };
}  // namespace mab