
#ifndef WIN32
#include <chrono>
#include <thread>

#include <linux/spi/spidev.h>
#include <sys/ioctl.h>

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
        m_logger.info("Connecting to SPI at %s", m_path.c_str());
        m_spiFileDescriptor = open(m_path.c_str(), O_RDWR);
        int err;  // for error handling
        if (m_spiFileDescriptor == -1)
        {
            m_logger.error("Failed to find spidev at %s", m_path.c_str());
            return I_CommunicationInterface::Error_t::INITIALIZATION_ERROR;
        }
        err = ioctl(m_spiFileDescriptor, SPI_IOC_WR_MODE, &SPI_MODE);
        if (err != 0)
        {
            m_logger.error("Failed to set SPI mode with code %d", err);
            return I_CommunicationInterface::Error_t::INITIALIZATION_ERROR;
        }
        err = ioctl(m_spiFileDescriptor, SPI_IOC_WR_BITS_PER_WORD, &SPI_BITS_PER_WORD);
        if (err != 0)
        {
            m_logger.error("Failed to set SPI bits per word");
            return I_CommunicationInterface::Error_t::INITIALIZATION_ERROR;
        }
        err = ioctl(m_spiFileDescriptor, SPI_IOC_WR_MAX_SPEED_HZ, &SPI_SPEED);
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
        std::vector<u8> receivedData(expectedReceivedDataSize + spiCRC.getCrcLen() /*CRC bytes*/,
                                     0);
        data.reserve(data.size() + spiCRC.getCrcLen());
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
        else
        {
            m_logger.info("SPI transfer successful!");
        }

        if (expectedReceivedDataSize > 0)
        {
            data.clear();
            data.resize(receivedData.size(), 0);
            m_transferBuffer.len    = 1;
            m_transferBuffer.tx_buf = (std::size_t)data.data();
            m_transferBuffer.rx_buf = (std::size_t)receivedData.data();

            std::chrono::high_resolution_clock::time_point startTime =
                std::chrono::high_resolution_clock::now();
            while (std::chrono::duration_cast<std::chrono::microseconds>(
                       std::chrono::high_resolution_clock::now() - startTime)
                       .count() < timeoutMs * 1000)
            {
                // Try transfer - it only checks first byte for response
                int err = ioctl(m_spiFileDescriptor, SPI_IOC_MESSAGE(1), &m_transferBuffer);
                if (err < 0)
                {
                    m_logger.error("SPI transfer failed!");
                    return std::make_pair(receivedData,
                                          I_CommunicationInterface::Error_t::TRANSMITTER_ERROR);
                }
                if (receivedData[0] != 0)
                {
                    m_logger.info("Received data from SPI device");
                    m_transferBuffer.len    = receivedData.size() - 1;
                    m_transferBuffer.rx_buf = (std::size_t)(receivedData.data() + 1);
                    // Perform the actual transfer (excluding the first byte which was sent earlier)
                    err = ioctl(m_spiFileDescriptor, SPI_IOC_MESSAGE(1), &m_transferBuffer);
                    if (err < 0)
                    {
                        m_logger.error("SPI transfer failed!");
                        return std::make_pair(receivedData,
                                              I_CommunicationInterface::Error_t::TRANSMITTER_ERROR);
                    }
                    // Check CRC
                    if (spiCRC.checkCrcBuf((char*)receivedData.data(), receivedData.size()))
                    {
                        // Remove CRC bytes from the received data
                        receivedData.resize(receivedData.size() - spiCRC.getCrcLen());
                        return std::make_pair(receivedData, I_CommunicationInterface::OK);
                    }
                    else
                    {
                        m_logger.error("CRC check failed");
                        return std::make_pair(receivedData,
                                              I_CommunicationInterface::Error_t::RECEIVER_ERROR);
                    }
                }
            }
            m_logger.error("Timeout while waiting for data from SPI device");
            return std::make_pair(receivedData, I_CommunicationInterface::Error_t::TIMEOUT);
        }

        return std::make_pair(receivedData, I_CommunicationInterface::OK);
    }

    SPI::~SPI()
    {
        disconnect();
    }

}  // namespace mab

#endif  // WIN32
