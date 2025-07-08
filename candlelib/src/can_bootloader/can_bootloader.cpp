#include "can_bootloader.hpp"
#include <array>
#include <span>
#include <string_view>
#include <vector>
#include "candle_bootloader.hpp"
#include "candle.hpp"

namespace mab
{

    CanBootloader::CanBootloader(const canId_t id, Candle* candle) : m_id(id), mp_candle(candle) {};

    CanBootloader::~CanBootloader()
    {
        detachCandle(mp_candle);
        mp_candle = nullptr;
    }

    CanBootloader::Error_t CanBootloader::init(const u32 bootAdress, const u32 appSize) const
    {
        if (!mp_candle)
        {
            m_log.error("Candle not provided!");
            return Error_t::NOT_CONNNECTED;
        }

        std::array<u8, sizeof(bootAdress)> bootAdressData = serializeData(bootAdress);
        std::array<u8, sizeof(appSize)>    appSizeData    = serializeData(appSize);

        std::vector<u8> payload;
        payload.insert(payload.end(), bootAdressData.begin(), bootAdressData.end());
        payload.insert(payload.end(), appSizeData.begin(), appSizeData.end());

        return sendCommand(Command_t::INIT, payload);
    }

    CanBootloader::Error_t CanBootloader::erase(const u32 address, const u32 size) const
    {
        if (!mp_candle)
        {
            m_log.error("Candle not provided!");
            return Error_t::NOT_CONNNECTED;
        }

        // Erase is staged in pages because CAN wdg in Candle fw is too short
        // to erase all at once

        const u32 startAddress     = address;
        const u32 endAddress       = address + size;
        const u32 fullPagesToErase = (endAddress - startAddress) / STM32_PAGE_SIZE;
        for (u32 i = 0; i < fullPagesToErase; i += 1)
        {
            std::vector<u8> payload;

            std::array<u8, sizeof(address)> addressData =
                serializeData(startAddress + i * STM32_PAGE_SIZE);
            std::array<u8, sizeof(size)> sizeData = serializeData(STM32_PAGE_SIZE);

            payload.insert(payload.end(), addressData.begin(), addressData.end());
            payload.insert(payload.end(), sizeData.begin(), sizeData.end());

            m_log.debug("Erasing page %d", i);
            if (sendCommand(Command_t::ERASE, payload) != Error_t::OK)
            {
                m_log.error("Failed to erase page %d", i);
                return CanBootloader::Error_t::DATA_TRANSFER_ERROR;
            }
        }

        m_log.debug("Erasing %d bytes from address %d", size, address);

        std::array<u8, sizeof(address)> addressData =
            serializeData(address + fullPagesToErase * STM32_PAGE_SIZE);
        std::array<u8, sizeof(size)> sizeData =
            serializeData(size - fullPagesToErase * STM32_PAGE_SIZE);

        std::vector<u8> payload;
        payload.insert(payload.end(), addressData.begin(), addressData.end());
        payload.insert(payload.end(), sizeData.begin(), sizeData.end());

        return sendCommand(Command_t::ERASE, payload);
    }

    CanBootloader::Error_t CanBootloader::startTransfer(
        const bool encrypted, const std::span<const u8, 16> initializationVector) const
    {
        std::vector<u8> payload;
        payload.push_back(static_cast<u8>(encrypted));
        payload.insert(payload.end(), initializationVector.begin(), initializationVector.end());

        return sendCommand(Command_t::PROG, payload);
    }

    CanBootloader::Error_t CanBootloader::transferData(
        const std::span<const u8, TRANSFER_SIZE> data, const u32 crc32) const
    {
        std::vector<u8> payload;
        for (size_t i = 0; i < data.size(); i += CHUNK_SIZE)
        {
            payload.clear();
            payload.insert(payload.end(), data.begin() + i, data.begin() + i + CHUNK_SIZE);
            auto err = sendFrame(payload);
            if (err != Error_t::OK)
                return err;
        }
        payload.clear();
        std::array<u8, sizeof(crc32)> crc32data = serializeData(crc32);
        payload.insert(payload.end(), crc32data.begin(), crc32data.end());

        return sendCommand(Command_t::WRITE, payload);
    }

    CanBootloader::Error_t CanBootloader::transferMetadata(
        const bool save, const std::span<const u8, 32> firmwareSHA256) const
    {
        std::vector<u8> payload;
        payload.push_back(static_cast<u8>(save));
        payload.insert(payload.end(), firmwareSHA256.begin(), firmwareSHA256.end());
        payload.resize(63);

        return sendCommand(Command_t::META, payload);
    }

    CanBootloader::Error_t CanBootloader::boot(const u32 bootAddress) const
    {
        std::array<u8, sizeof(bootAddress)> bootAdressData = serializeData(bootAddress);

        std::vector<u8> payload;
        payload.insert(payload.end(), bootAdressData.begin(), bootAdressData.end());

        return sendCommand(Command_t::BOOT, payload);
    }

    CanBootloader::Error_t CanBootloader::sendCommand(const Command_t        command,
                                                      const std::vector<u8>& data) const
    {
        if (!mp_candle)
        {
            m_log.error("Candle not provided!");
            return Error_t::NOT_CONNNECTED;
        }

        std::vector<u8> frame = {static_cast<u8>(command)};
        frame.insert(frame.end(), data.begin(), data.end());

        return sendFrame(frame);
    }

    CanBootloader::Error_t CanBootloader::sendFrame(const std::vector<u8>& frame) const
    {
        if (!mp_candle)
            return Error_t::NOT_CONNNECTED;

        auto result = mp_candle->transferCANFrame(
            m_id, frame, DEFAULT_REPONSE.size(), m_customReponseTimeoutMs.value_or(DEFAULT_TIMOUT));
        if (result.second != candleTypes::Error_t::OK)
        {
            m_log.error("Failed to send frame!");
            return Error_t::DATA_TRANSFER_ERROR;
        }

        std::string_view response(reinterpret_cast<const char*>(result.first.data()),
                                  result.first.size());

        if (response.find(DEFAULT_REPONSE) == std::string_view::npos)
        {
            m_log.error("Invalid response!");
            return Error_t::DATA_TRANSFER_ERROR;
        }

        return Error_t::OK;
    }
}  // namespace mab
