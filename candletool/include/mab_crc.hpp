#ifndef MAB_CRC_HPP
#define MAB_CRC_HPP

#include <stdint.h>

namespace mab
{
    uint32_t CalcCRC(uint8_t* pData, uint32_t DataLength);
}

#endif /* MAB_CRC_HPP */