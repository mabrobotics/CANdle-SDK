#ifndef MAB_CRC_HPP
#define MAB_CRC_HPP

#include <stdint.h>

namespace mab
{
    uint32_t CalcCRC(uint8_t* pData, uint32_t DataLength);
    uint32_t crc32(const uint8_t* data, uint32_t len);
}

#endif /* MAB_CRC_HPP */
