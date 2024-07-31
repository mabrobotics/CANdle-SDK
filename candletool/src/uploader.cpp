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

namespace mab
{

    FirmwareUploader::FirmwareUploader(Candle& _candle, mabFileParser& mabFile, int mdId)
        : m_candle(_candle), m_mabFile(mabFile), m_canId(mdId)
    {
        m_log.tag = "FW Uploader";
    }

    FirmwareUploader::ERROR_E FirmwareUploader::flashDevice(bool directly)
    {
        iLoader* loader = nullptr;

        switch (m_mabFile.m_firmwareEntry1.targetDevice)
        {
            case mabFileParser::TargetDevice_E::MD:
                loader = new CanLoader(m_candle, m_mabFile, m_canId);
                break;

            case mabFileParser::TargetDevice_E::CANDLE:
                loader = new UsbLoader(m_candle, m_mabFile);
                break;

            default:
                m_log.error("Unsupported target device!");
                return ERROR_E::ERROR_UNKNOWN;
        }

        /* send reset command to the md80 firmware */
        if (directly == false)
            loader->resetDevice();

        if (iLoader::Error_E::OK != loader->enterBootloader())
        {
            m_log.error("Failed to enter bootloader mode!");
            return ERROR_E::ERROR_UNKNOWN;
        }

        /* upload firmware */
        if (iLoader::Error_E::OK != loader->uploadFirmware())
        {
            m_log.error("Failed to upload firmware!");
            return ERROR_E::ERROR_UNKNOWN;
        }

        /* send boot command */
        if (iLoader::Error_E::OK != loader->sendBootCommand())
        {
            m_log.error("Failed to send boot command!");
            return ERROR_E::ERROR_UNKNOWN;
        }

        return ERROR_E::OK;
    }

}  // namespace mab
