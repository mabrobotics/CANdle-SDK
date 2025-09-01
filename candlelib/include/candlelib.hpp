#pragma once
/*
This file is the main include that should be used by users when building their programs
It provides all modules and API interfaces.
*/

#include <unordered_map>

#include "mab_types.hpp"
#include "candle.hpp"
#include "MD.hpp"
#include "pds.hpp"
#include "USB.hpp"
#include "SPI.hpp"

namespace mab
{
    /// @brief This struct represents one branch of CANdle device + all the connected subdevices
    struct CANdleBranch_S
    {
        std::shared_ptr<CandleBuilder>                    candleBuilder;
        std::shared_ptr<std::unordered_map<canId_t, MD>>  MdMap;
        std::shared_ptr<std::unordered_map<canId_t, Pds>> PdsVector;
    };
}  // namespace mab