#include "can_bootloader.hpp"
#include "candle_v2.hpp"

namespace mab
{

    CanBootloader::CanBootloader(const canId_t id, CandleV2* candle)
        : m_id(id), mp_candle(candle){};

    CanBootloader::~CanBootloader()
    {
        detachCandle(mp_candle);
        mp_candle = nullptr;
    }

    CanBootloader::Error_t CanBootloader::init(const u32 bootAdress, const u32 appSize)
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

    CanBootloader::Error_t CanBootloader::sendCommand(const Command_t        command,
                                                      const std::vector<u8>& data)
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

    CanBootloader::Error_t CanBootloader::sendFrame(const std::vector<u8>& frame)
    {
        if (!mp_candle)
            return Error_t::NOT_CONNNECTED;

        auto result = mp_candle->transferCANFrame(m_id, frame, DEFAULT_REPONSE.size());
        if (result.second != candleTypes::Error_t::OK)
        {
            m_log.error("Failed to send frame!");
            return Error_t::DATA_TRANSFER_ERROR;
        }
        return Error_t::OK;
    }
}  // namespace mab
