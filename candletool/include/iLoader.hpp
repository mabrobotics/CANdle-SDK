#pragma once

#include <stdint.h>
#include "mabFileParser.hpp"

class I_Loader
{
  public:
    enum class Error_E : uint8_t
    {
        OK = 0,
        ERROR_SETUP,  // Could not enter setup mode
        ERROR_ERASE,  // Problem during erasing flash
        ERROR_PROG,   // Could not init frimware transfer
        ERROR_PAGE,   // Error during bulk data transfer
        ERROR_WRITE,  // Error during saving data in FLASH (possible CRC error of whole page)
        ERROR_META,   // Error of checksum or saving configuration in FLASH
        ERROR_BOOT,   // Error in transfer of boot command
        ERROR_UNKNOWN,
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
    static constexpr size_t M_PAGE_SIZE      = 2048U;
    static constexpr size_t M_CAN_CHUNK_SIZE = 64U;
    static constexpr size_t M_USB_CHUNK_SIZE = 2044;
};
