/*
 * Checksum.hpp
 *
 *  Created on: Oct 18, 2023
 *      Author: Piotr Wasilewski
 */

#ifndef MAB_CHECKSUM_H_
#define MAB_CHECKSUM_H_

#include <cstdint>
#include <cstdio>

namespace Checksum
{
uint32_t crc32(const uint8_t* data, size_t len);
}  // namespace Checksum

#endif /* MAB_CHECKSUM_H_ */
