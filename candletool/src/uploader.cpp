#include "uploader.hpp"
#include "md80.hpp"

#include <iostream>
#include <fstream>
#include <cstring>
#include "unistd.h"
#include "mini/ini.h"
#include "logger.hpp"
#include "canLoader.hpp"
#include "usbLoader.hpp"
// #include <memory>

namespace mab
{

    FirmwareUploader::FirmwareUploader(Candle& _candle, MabFileParser& mabFile, int mdId)
        : m_candle(_candle), m_mabFile(mabFile), m_canId(mdId)
    {
        m_log.m_tag   = "FW Uploader";
        m_log.m_layer = Logger::ProgramLayer_E::MIDDLE;
    }

    FirmwareUploader::ERROR_E FirmwareUploader::flashDevice(bool directly)
    {
        std::unique_ptr<I_Loader> pLoader = nullptr;

        switch (m_mabFile.m_firmwareEntry1.targetDevice)
        {
            case MabFileParser::TargetDevice_E::MD:
                pLoader = std::make_unique<CanLoader>(m_candle, m_mabFile, m_canId);
                break;

            case MabFileParser::TargetDevice_E::CANDLE:
                pLoader = std::make_unique<UsbLoader>(m_candle, m_mabFile);
                break;

            default:
                m_log.error("Unsupported target device!");
                return ERROR_E::ERROR_UNKNOWN;
        }

        /* send reset command to the md80 firmware */
        if (directly == false)
            pLoader->resetDevice();

        if (I_Loader::Error_E::OK != pLoader->enterBootloader())
        {
            m_log.error("Failed to enter bootloader mode!");
            return ERROR_E::ERROR_UNKNOWN;
        }

        /* upload firmware */
        if (I_Loader::Error_E::OK != pLoader->uploadFirmware())
        {
            m_log.error("Failed to upload firmware!");
            return ERROR_E::ERROR_UNKNOWN;
        }

        /* send boot command */
        if (I_Loader::Error_E::OK != pLoader->sendBootCommand())
        {
            m_log.error("Failed to send boot command!");
            return ERROR_E::ERROR_UNKNOWN;
        }

        return ERROR_E::OK;
    }

}  // namespace mab
