#include "candle.hpp"
#include "BinaryParser.hpp"

#include <string>
#include <vector>

namespace mab
{

	/**
	 * @class FirmwareUploader
	 * @brief Class for uploading firmware to a device.
	 *
	 * The FirmwareUploader class provides functionality to upload firmware to a device.
	 * It is used in conjunction with the Candle class to perform firmware updates.
	 */
	class FirmwareUploader
	{
	  public:
		/**
		 * @brief Constructs a FirmwareUploader object.
		 * @param _candle The reference to the Candle object.
		 * @param mabFile The path to the MAB file containing the firmware.
		 */
		FirmwareUploader(Candle& _candle, const std::string& mabFile);

		/**
		 * @brief Performs the firmware update.
		 *
		 * @param id The ID of the device to update.
		 * @param directly If true, connect to bootloader directly. If false, connect to bootloader
		 * via the application.
		 * @return true
		 * @return false
		 */
		bool flashDevice(int id, bool directly);
		void setVerbosity(bool verbosity);

	  private:
		mab::Candle& candle;
		logger		 log;

		enum BootloaderFrameId_t : uint8_t
		{
			CMD_TARGET_RESET = 0x13,
			CMD_HOST_INIT	 = 0xA1,
			CMD_PAGE_PROG	 = 0xA2,
			CMD_BOOT		 = 0xA3,
			CMD_WRITE		 = 0xA4,
		};

		static constexpr size_t	  M_PAGE_SIZE	 = 2048;
		static constexpr size_t	  M_CHUNK_SIZE	 = 64;
		static constexpr uint32_t M_BOOT_ADDRESS = 0x08005000;

		BinaryParser m_mabFile;
		size_t		 m_fileSize;
		size_t		 m_bytesToUpload;
		size_t		 m_pagesToUpload;
		uint32_t	 m_currentPage;
		uint32_t	 m_currentId;

		void sendResetCmd();
		bool sendInitCmd();
		bool sendPageProgCmd();
		bool sendPage();
		bool sendWriteCmd(uint8_t* pPageBuffer, int bufferSize);
		bool sendBootCmd();
	};
}  // namespace mab