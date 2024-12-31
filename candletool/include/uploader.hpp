#include "candle.hpp"
#include "mabFileParser.hpp"

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
         * @return true if flashing succeded
         * @return false if flashing failed
         */
        bool flashDevice(bool directly);
        void setVerbosity(bool verbosity);

      private:
        mab::Candle& m_candle;
        Logger       m_log;

        MabFileParser& m_mabFile;
        uint32_t       m_canId;
    };
}  // namespace mab
