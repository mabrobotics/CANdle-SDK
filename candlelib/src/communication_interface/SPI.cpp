
#include <sys/poll.h>
#ifndef WIN32

#include <linux/spi/spidev.h>
#include <sys/ioctl.h>
#include <poll.h>

#include "I_communication_interface.hpp"
#include "logger.hpp"
#include "SPI.hpp"

namespace mab
{
    SPI::SPI(const std::string_view path) : m_path(path)
    {
    }

    I_CommunicationInterface::Error_t SPI::connect()
    {
        m_spiFileDescriptor = open(m_path.c_str(), O_RDWR);
        int err;  // for error handling
        if (m_spiFileDescriptor == -1)
        {
            m_logger.error("Failed to find spidev at %s", m_path.c_str());
            return I_CommunicationInterface::Error_t::INITIALIZATION_ERROR;
        }
        err = ioctl(m_spiFileDescriptor, SPI_IOC_WR_MODE, SPI_MODE_0);
        if (err != 0)
        {
            m_logger.error("Failed to set SPI mode");
            return I_CommunicationInterface::Error_t::INITIALIZATION_ERROR;
        }
        err = ioctl(m_spiFileDescriptor, SPI_IOC_WR_BITS_PER_WORD, SPI_BITS_PER_WORD);
        if (err != 0)
        {
            m_logger.error("Failed to set SPI bits per word");
            return I_CommunicationInterface::Error_t::INITIALIZATION_ERROR;
        }
        err = ioctl(m_spiFileDescriptor, SPI_IOC_WR_MAX_SPEED_HZ, SPI_SPEED);
        if (err != 0)
        {
            m_logger.error("Failed to set SPI speed");
            return I_CommunicationInterface::Error_t::INITIALIZATION_ERROR;
        }

        m_transferBuffer.bits_per_word = SPI_BITS_PER_WORD;
        m_transferBuffer.speed_hz      = SPI_SPEED;
        return I_CommunicationInterface::OK;
    }

    I_CommunicationInterface::Error_t SPI::disconnect()
    {
        if (m_spiFileDescriptor != -1)
        {
            close(m_spiFileDescriptor);
            m_spiFileDescriptor = -1;
        }
        return I_CommunicationInterface::OK;
    }

    I_CommunicationInterface::Error_t SPI::transfer(std::vector<u8> data, const u32 timeoutMs)
    {
        return transfer(data, timeoutMs, 0).second;
    }

    std::pair<std::vector<u8>, I_CommunicationInterface::Error_t> SPI::transfer(
        std::vector<u8> data, const u32 timeoutMs, const size_t expectedReceivedDataSize)
    {
        std::vector<u8> receivedData(expectedReceivedDataSize, 0);
        data.reserve(data.size() + 4);
        auto crc = spiCRC.calcCrc((char*)data.data(), data.size());

        // Assign the CRC bytes to the data vector
        data.push_back(crc);
        data.push_back(crc >> 8);
        data.push_back(crc >> 16);
        data.push_back(crc >> 24);

        m_transferBuffer.tx_buf = (std::size_t)data.data();
        m_transferBuffer.rx_buf = (std::size_t)receivedData.data();
        m_transferBuffer.len    = data.size();

        if (m_spiFileDescriptor == -1)
        {
            m_logger.error("Unconnected SPI!");
            return std::make_pair(receivedData,
                                  I_CommunicationInterface::Error_t::INITIALIZATION_ERROR);
        }

        int err = ioctl(m_spiFileDescriptor, SPI_IOC_MESSAGE(1), &m_transferBuffer);
        if (err < 0)
        {
            m_logger.error("SPI transfer failed!");
            return std::make_pair(receivedData,
                                  I_CommunicationInterface::Error_t::TRANSMITTER_ERROR);
        }

        if (expectedReceivedDataSize > 0)
        {
            struct pollfd pfd = {0};
            pfd.fd            = m_spiFileDescriptor;
            pfd.events        = POLLIN;
            int err           = poll(&pfd, 1, timeoutMs);
            if (err < 0)
            {
                m_logger.error("SPI receive failed!");
                return std::make_pair(receivedData,
                                      I_CommunicationInterface::Error_t::RECEIVER_ERROR);
            }
            else if (err == 0)
            {
                m_logger.error("SPI receive timeout!");
                return std::make_pair(receivedData, I_CommunicationInterface::Error_t::TIMEOUT);
            }
            else if (pfd.revents & POLLIN)
            {
                ssize_t bytesRead =
                    read(m_spiFileDescriptor, receivedData.data(), expectedReceivedDataSize);
                if (bytesRead < 0)
                {
                    m_logger.error("SPI read failed!");
                    return std::make_pair(receivedData,
                                          I_CommunicationInterface::Error_t::RECEIVER_ERROR);
                }
                else if (bytesRead != (ssize_t)expectedReceivedDataSize)
                {
                    m_logger.error("SPI read incomplete!");
                    return std::make_pair(receivedData,
                                          I_CommunicationInterface::Error_t::DATA_EMPTY);
                }
                else
                {
                    m_logger.info("SPI read successful!");
                }
            }
            else
            {
                m_logger.error("Undefined polling state");
                return std::make_pair(receivedData,
                                      I_CommunicationInterface::Error_t::UNKNOWN_ERROR);
            }
        }

        return std::make_pair(receivedData, I_CommunicationInterface::OK);
    }

    SPI::~SPI()
    {
        disconnect();
    }

}  // namespace mab

#endif  // WIN32
