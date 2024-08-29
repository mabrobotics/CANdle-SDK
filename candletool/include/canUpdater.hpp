#include "mab_types.hpp"
#include "candle.hpp"

namespace canUpdater
{
	struct mabData
	{
		char tag[16]			= {};
		u8	 iv[16]				= {};
		u8	 checksum[32]		= {};
		u32	 fwStartAddress		= 0x8000000;
		u32	 fwSize				= 0;
		char fwVersion[10]		= "00.00.00";
		u8	 fwData[256 * 1024] = {};
	};
	bool parseMabFile(const char* pathToMabFile, const char* tag, mabData& mabData);
	bool sendHostInit(mab::Candle& candle, logger& log, u16 id, u32 fwStartAdr, u32 fwSize);
	bool sendErase(mab::Candle& candle, logger& log, u16 id, u32 eraseStart, u32 eraseSize);
	bool sendProgStart(mab::Candle& candle, u16 id, bool cipher, u8* iv);
	bool sendWrite(mab::Candle& candle, u16 id, u8* pagePtr, u32 dataSize);
	bool sendSendFirmware(mab::Candle& candle, logger& log, u16 id, u32 fwSize, u8* fwBuffer);
	bool sendBoot(mab::Candle& candle, u16 id, u32 fwStart);
	bool sendMeta(mab::Candle& candle, u16 id, u8* checksum);
}  // namespace canUpdater
