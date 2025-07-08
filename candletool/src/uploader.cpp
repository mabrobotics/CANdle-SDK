#include "uploader.hpp"

#include <cstring>
#include "canLoader.hpp"
#include "usbLoader.hpp"

namespace mab
{

    FirmwareUploader::FirmwareUploader(MabFileParser& mabFile, int mdId)
        : m_mabFile(mabFile), m_canId(mdId)
    {
        m_log.m_tag   = "FW LOADER";
        m_log.m_layer = Logger::ProgramLayer_E::LAYER_2;
    }

    bool FirmwareUploader::flashDevice(bool directly)
    {
        std::unique_ptr<I_Loader> pLoader = nullptr;

        switch (m_mabFile.m_fwEntry.targetDevice)
        {
            case MabFileParser::TargetDevice_E::MD:
            case MabFileParser::TargetDevice_E::PDS:
                pLoader = std::make_unique<CanLoader>(m_candle, m_mabFile, m_canId);
                break;

            case MabFileParser::TargetDevice_E::CANDLE:
                pLoader = std::make_unique<UsbLoader>(m_candle, m_mabFile);
                break;

            default:
                m_log.error("Unsupported target device!");
                return false;
        }

        /* send reset command to the md firmware */
        if (directly == false)
            pLoader->resetDevice();

        // Bootloader always communicates with 1M baud
        m_candle.configCandleBaudrate(mab::CANdleBaudrate_E::CAN_BAUD_1M);

        m_log.debug("Entering bootloader");
        if (I_Loader::Error_E::OK != pLoader->enterBootloader())
        {
            m_log.error("Failed to enter bootloader mode!");
            return false;
        }

        /* upload firmware */
        m_log.debug("Starting firmware upload");
        I_Loader::Error_E result = pLoader->uploadFirmware();
        switch (result)
        {
            case I_Loader::Error_E::OK:
                break;
            case I_Loader::Error_E::ERROR_ERASE:
                m_log.error("Failed to erase memory");
                break;
            case I_Loader::Error_E::ERROR_PROG:
            case I_Loader::Error_E::ERROR_PAGE:
                m_log.error("Failed to program FLASH page");
                break;
            case I_Loader::Error_E::ERROR_WRITE:
                m_log.error("Failed to validate page CRC.");
                break;
            default:
                m_log.error("Unexpected error happened. Error code: %d", result);
        }
        if (result != I_Loader::Error_E::OK)
        {
            m_log.error("Failed to upload firmware!");
            return false;
        }
        m_log.debug("Firmware update complete");

        /* send boot command */
        m_log.debug("Sending boot command");
        if (I_Loader::Error_E::OK != pLoader->sendBootCommand())
        {
            m_log.error("Failed to send boot command!");
            return false;
        }
        return true;
    }

}  // namespace mab
