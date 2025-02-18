#pragma once

#include "mab_types.hpp"
#include "md_types.hpp"
#include "candle_v2.hpp"
#include "logger.hpp"

#include <cstring>

#include <array>
#include <queue>
#include <utility>
#include <functional>
#include <tuple>
#include <vector>

namespace mab
{
    class MD
    {
        enum class FrameType_E
        {
            READ,
            WRITE,
            DEFAULT
        };

        const u32 m_canId;

        const std::function<std::pair<std::vector<u8>, CandleV2::Error_t>(
            const CandleV2&, const u32, const std::vector<u8>, const size_t, const u32)>
            m_transferCAN;

      public:
        MDRegisters_S mdRegisters;

        enum Error_t
        {
            OK,
            REQUEST_INVALID,
            TRANSFER_FAILED
        };

        MD(u32 canId,
           std::function<std::pair<std::vector<u8>, CandleV2::Error_t>(
               const CandleV2&, const u32, const std::vector<u8>, const size_t, const u32)>
               transferCANFrame)
            : m_canId(canId), m_transferCAN(transferCANFrame)
        {
        }

        void blink();

        template <class... T>
        static inline std::vector<u8> serializeMDRegisters(std::tuple<T...>& regs)
        {
            std::vector<u8> serialized;

            std::apply(
                [&](auto&&... args)
                {
                    ((serialized.insert(serialized.end(),
                                        args.getSerializedRegister()->begin(),
                                        args.getSerializedRegister()->end())),
                     ...);
                },
                regs);
            return serialized;
        }

        template <class... T>
        static inline void deserializeMDRegisters(std::vector<u8>& input, std::tuple<T...>& regs)
        {
            std::apply([&](auto&&... args) { (args.setSerializedRegister(input), ...); }, regs);
            return;
        }
    };
}  // namespace mab