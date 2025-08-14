#pragma once

#include <string>
#include <exception>
#include <vector>
#include <memory>
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
    /// @brief RAII implementation of libusb devices. m_dev dereferenced on destruction so it
    /// becomes invalid.
    class LibusbDevice
    {
        libusb_device*            m_dev;
        libusb_device_handle*     m_devHandle;
        libusb_device_descriptor  m_desc;
        Logger                    m_log = Logger(Logger::ProgramLayer_E::BOTTOM, "USB_DEV");
        libusb_config_descriptor* m_config;

        s32        m_inEndpointAddress, m_outEndpointAddress;
        const bool m_peek;

        bool m_connected = false;

        std::array<u8, 512> m_rxBuffer = {0};

      public:
        LibusbDevice(libusb_device* device,
                     const s32      inEndpointAddress,
                     const s32      outEndpointAddress,
                     const bool     peek = false);
        ~LibusbDevice();

        libusb_error transmit(u8* data, const size_t length, const u32 timeout);
        libusb_error receive(u8* data, const size_t length, const u32 timeout);

        libusb_error unclogInput();
        libusb_error unclogOutput();

        bool isConnected() const
        {
            return m_connected;
        }

        std::string getSerialNo();
    };

    // TODO: those should be args
    static constexpr int IN_ENDPOINT  = 0x81;  ///< CANdle USB input endpoint address.
    static constexpr int OUT_ENDPOINT = 0x01;  ///< CANdle USB output endpoint address.
    class USB : public I_CommunicationInterface
    {
      private:
        Logger m_Log = Logger(Logger::ProgramLayer_E::BOTTOM, "USB");

        std::unique_ptr<LibusbDevice> m_libusbDevice = nullptr;

        u16         m_vid, m_pid;
        std::string m_serialNo = "";

        static constexpr size_t USB_MAX_BUFF_LEN = 16'000'000;  // bytes
        static constexpr size_t DEFAULT_TIMEOUT  = 100;         // ms

        libusb_context* m_ctx;

      public:
        /// @brief Initialize USB interface
        /// @param vid vid of the target device
        /// @param pid pid of the target device
        /// @param serialNo serial number of the target device. If empty than first device from
        /// the list becomes active device.
        explicit USB(const u16 vid, const u16 pid, const std::string serialNo = "");
        ~USB();

        Error_t connect() override;
        Error_t disconnect() override;

        Error_t transfer(std::vector<u8> data, const u32 timeoutMs) override;
        std::pair<std::vector<u8>, Error_t> transfer(
            std::vector<u8> data,
            const u32       timeoutMs,
            const size_t    expectedReceivedDataSize) override;
    };
}  // namespace mab
