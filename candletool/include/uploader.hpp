#include "candle.hpp"

#include <string>
#include <vector>

namespace mab
{
	class FirmwareUploader
	{
	  private:
		mab::Candle& candle;

		enum BootloaderFrameId_t : uint8_t
		{
			CMD_TARGET_RESET = 0x13,
			CMD_HOST_INIT	 = 0xA1,
			CMD_PAGE_PROG	 = 0xA2,
			CMD_BOOT		 = 0xA3,
			CMD_WRITE		 = 0xA4,
		};

		static constexpr size_t	  pageSize	= 2048;
		static constexpr size_t	  chunkSize = 64;
		static constexpr uint32_t address	= 0x08005000;

		size_t	 fileSize;
		size_t	 bytesToUpload;
		size_t	 pagesToUpload;
		uint32_t currentPage;
		uint32_t currentId;

		void sendResetCmd();
		bool sendInitCmd();
		bool sendPageProgCmd();
		bool sendPage();
		bool sendWriteCmd(uint8_t* pPageBuffer, int bufferSize);
		bool sendBootCmd();

	  public:
		FirmwareUploader(Candle& _candle);
		void setVerbosity(bool verbosity);

		bool flashDevice(int id, bool directly);
	};
}  // namespace mab