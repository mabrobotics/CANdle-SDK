#pragma once
#include "mab_types.hpp"
#include <stddef.h>

namespace mab
{
    constexpr u16    BROADCAST_ID    = 9u;
    constexpr u16    CAN_MIN_ID      = 10u;
    constexpr u16    CAN_MAX_ID      = 2000u;
    constexpr size_t CAN_MAX_PAYLOAD = 64u;
}  // namespace mab