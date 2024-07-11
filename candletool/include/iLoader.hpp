#pragma once

#include <stdint.h>
#include "mabFileParser.hpp"

class iLoader
{
  public:
    enum class Error_E : uint8_t
    {
        OK = 0,
        ERROR_UNKNOWN,
    };
    iLoader() = delete;
    iLoader(mabFileParser& mabFile)
        : m_mabFile(mabFile){

          };

    /**
     * @brief Reset the target device
     */
    virtual Error_E resetDevice() = 0;

    /**
     * @brief Enter the bootloader mode of the target device
     * In some devices this step is not necessary so implementation could be empty
     */
    virtual Error_E enterBootloader() = 0;

    /**
     * @brief Upload the firmware to the target device
     */
    virtual Error_E uploadFirmware() = 0;

    /**
     * @brief Send the boot command to the target device
     */
    virtual Error_E sendBootCommand() = 0;

  protected:
    static constexpr size_t   M_PAGE_SIZE      = 2048U;
    static constexpr size_t   M_CAN_CHUNK_SIZE = 64U;
    static constexpr size_t   M_USB_CHUNK_SIZE = 64U;
    static constexpr uint32_t M_BOOT_ADDRESS   = 0x08005000;

    mabFileParser m_mabFile;
};