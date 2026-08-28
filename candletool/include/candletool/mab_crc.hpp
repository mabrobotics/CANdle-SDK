#ifndef MAB_CRC_HPP
#define MAB_CRC_HPP

#include <stdint.h>
#include <cstddef>

namespace mab
{
    uint32_t crc32(const uint8_t* buf, uint32_t len);
    namespace candleCRC
    {
        uint32_t crc32(const uint8_t* buf,
                       size_t         len);  // TODO: this must be unified when fw will be changed
    }
}  // namespace mab

#endif /* MAB_CRC_HPP */
