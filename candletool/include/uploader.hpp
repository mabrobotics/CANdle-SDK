#include "candle.hpp"
#include "mabFileParser.hpp"

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
        enum class ERROR_E : uint8_t
        {
            OK = 0,
            ERROR_FILE,
            ERROR_CHECKSUM,
            ERROR_INIT,
            ERROR_PAGE_PROG,
            ERROR_WRITE,
            ERROR_BOOT,
            ERROR_RESET,
            ERROR_TIMEOUT,
            ERROR_UNKNOWN,
        };

        /**
         * @brief Construct a new Firmware Uploader object
         *
         */
        FirmwareUploader() = delete;

        /**
         * @brief Constructs a FirmwareUploader object.
         * @param _candle The reference to the Candle object.
         * @param mabFile The path to the MAB file containing the firmware.
         */
        FirmwareUploader(Candle& _candle, MabFileParser& mabFile, int mdId = 0);

        /**
         * @brief Performs the firmware update.
         *
         * @param id The ID of the device to update.
         * @param directly If true, connect to bootloader directly. If false, connect to bootloader
         *
         * via the application.
         * @return true
         * @return false
         */
        ERROR_E flashDevice(bool directly);
        void    setVerbosity(bool verbosity);

      private:
        mab::Candle& m_candle;
        logger       m_log;

        MabFileParser& m_mabFile;
        uint32_t       m_canId;
    };
}  // namespace mab