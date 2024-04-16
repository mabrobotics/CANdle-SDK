/*
 * Checksum.hpp
 *
 *  Created on: Oct 18, 2023
 *      Author: Piotr Wasilewski
 */

#ifndef MAB_CHECKSUM_H_
#define MAB_CHECKSUM_H_

#include <cstddef>
#include <cstdint>
#include <cstdio>

namespace Checksum
{
	uint32_t crc32(const uint8_t* data, size_t len);

	typedef unsigned char BYTE; // 8-bit byte
	typedef unsigned int WORD;	// 32-bit word, change to "long" for 16-bit machines
	class SHA256
	{
	  public:
		SHA256();
		void update(const BYTE data[], size_t len);
		void final(BYTE hash[]);

	  private:
		const WORD blockSize = 32;
		typedef struct
		{
			BYTE data[64];
			WORD datalen;
			unsigned long long bitlen;
			WORD state[8];
		} SHA256_CTX;
		SHA256_CTX ctx;
		void transform(const BYTE data[]);
	};

} // namespace Checksum

#endif /* MAB_CHECKSUM_H_ */
