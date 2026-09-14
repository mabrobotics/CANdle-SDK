#pragma once

#include <array>

#include "candle/candle.hpp"
#include "candletool/can_bootloader.hpp"

#include "candletool/mabFileParser.hpp"

namespace mab
{
    class CanLoader
    {
      public:
        CanLoader() = delete;
        CanLoader(mab::Candle* candle, MabFileParser* mabFile, mab::canId_t canId);
        ~CanLoader();
        bool flashAndBoot(bool recovery);
        bool forceEraseConfig();

      private:
        const MabFileParser* m_mabFile;
        mab::Candle*         m_candle;
        const mab::canId_t   m_canId;
        Logger               m_log;
    };

}  // namespace mab
