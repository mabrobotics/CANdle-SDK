#ifndef CANLOADER_HPP
#define CANLOADER_HPP

#include <array>
#include "iLoader.hpp"
#include "candle.hpp"
#include "logger.hpp"
#include "mab_types.hpp"
#include "can_bootloader.hpp"
namespace mab
{
    class CanLoader
    {
      public:
        CanLoader() = delete;
        CanLoader(mab::Candle* candle, MabFileParser* mabFile, mab::canId_t canId);
        ~CanLoader();
        bool flashAndBoot();

      private:
        const MabFileParser* m_mabFile;
        mab::Candle*         m_candle;
        const mab::canId_t   m_canId;
        Logger               m_log;
    };

}  // namespace mab
#endif /* CANLOADER_HPP */
