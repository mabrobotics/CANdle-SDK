#ifndef MAB_CHECKSUM_H_
#define MAB_CHECKSUM_H_

#include "mab_types.hpp"

namespace Checksum
{
	u32 crc32(const u8* data, u32 len);
}  // namespace Checksum

#endif /* MAB_CHECKSUM_H_ */
