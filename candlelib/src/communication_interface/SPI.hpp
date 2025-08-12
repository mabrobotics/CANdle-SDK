#pragma once
#ifndef WIN32
#include <chrono>

#include <fcntl.h>
#include <linux/ioctl.h>
#include <linux/spi/spidev.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string>
#include <thread>

#include "mab_types.hpp"
#include "I_communication_interface.hpp"
#include "logger.hpp"
#include "crc.hpp"

namespace mab
{
    class SPI final : public I_CommunicationInterface
    {
        static constexpr u8  SPI_MODE          = SPI_MODE_0;
        static constexpr u32 SPI_SPEED         = 20'000'000;  // 20 MHz
        static constexpr u8  SPI_BITS_PER_WORD = 8;
        static constexpr u32 MAX_TRANSFER_SIZE = 2048;

      public:
        SPI(const std::string_view path = "/dev/spidev0.0");
        virtual ~SPI() override;

        virtual Error_t connect() override;

        virtual Error_t disconnect() override;

        virtual Error_t transfer(std::vector<u8> data, const u32 timeoutMs) override;

        virtual std::pair<std::vector<u8>, Error_t> transfer(
            std::vector<u8> data,
            const u32       timeoutMs,
            const size_t    expectedReceivedDataSize) override;

      private:
        Logger            m_logger            = Logger(Logger::ProgramLayer_E::BOTTOM, "SPI");
        int               m_spiFileDescriptor = -1;
        spi_ioc_transfer  m_transferBuffer{0};
        const std::string m_path;

        Crc spiCRC;
    };
}  // namespace mab

#endif  // WIN32
