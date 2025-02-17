#pragma once

#include "mab_types.hpp"
#include "md_types.hpp"
#include "candle_v2.hpp"
#include "logger.hpp"

#include <array>
#include <utility>
#include <functional>
#include <tuple>
#include <variant>

namespace mab
{
    class MD
    {
        template <typename... Entries>
        class MDReadFrameSerializer
        {
            std::vector<u8> m_frame;

          public:
            MDReadFrameSerializer(u32 MDId)
            {
                m_frame.reserve(3);
                m_frame.push_back(MDId);
                m_frame.push_back((u8)(MDId >> 8));
                m_frame.push_back(0x0);
            }
            template <typename T, RegisterAccessLevel_E L, u16 address>
            void addRegister(RegisterEntry_S<T, L, address> reg)
            {
                m_frame.reserve(sizeof(address));

                m_frame.push_back(static_cast<u8>(reg.regAddress));
                m_frame.push_back(static_cast<u8>(reg.regAddress >> 8));

                for (size_t byte = 0; byte < reg.getSize(); byte++)
                {
                    // payload is always zeroed when reading data
                    m_frame.push_back(static_cast<u8>(0x0 >> (8 * byte)));
                }
            }

            void
        };

        const u32 m_canId;

        const std::function<std::pair<std::vector<u8>, CandleV2::Error_t>(
            const CandleV2&, const std::vector<u8>, const size_t, const u32)>
            m_transferCAN;

      public:
        enum Error_t
        {
            OK,
            REQUEST_INVALID,
            TRANSFER_FAILED
        };

        MD(u32                                                                   canId,
           std::function<std::pair<std::vector<u8>, CandleV2::Error_t>(
               const CandleV2&, const std::vector<u8>, const size_t, const u32)> transferCANFrame)
            : m_canId(canId), m_transferCAN(transferCANFrame)
        {
        }
    };
}  // namespace mab