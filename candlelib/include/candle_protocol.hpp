#pragma once

#include <cstdint>
/**
 *
 * @brief This file contains Definitions for communicating between host <-> CANdle.
 * This should not be required by the user during the creation of custom code. *
 */
namespace mab
{
    enum BusFrameId_t : uint8_t
    {
        BUS_FRAME_NONE                   = 0,
        BUS_FRAME_PING_START             = 1,
        BUS_FRAME_CANDLE_CONFIG_BAUDRATE = 2,
        BUS_FRAME_MD80_ADD               = 3,
        BUS_FRAME_MD80_GENERIC_FRAME     = 4,
        BUS_FRAME_MD80_CONFIG_CAN        = 5,
        BUS_FRAME_BEGIN                  = 6,
        BUS_FRAME_END                    = 7,
        BUS_FRAME_UPDATE                 = 8,
        BUS_FRAME_RESET                  = 9,
        USB_FRAME_ENTER_BOOTLOADER       = 10,
    };

    enum BootloaderBusFrameId_E : uint8_t
    {
        BOOTLOADER_FRAME_CHECK_ENTERED = 100,
        BOOTLOADER_FRAME_SEND_PAGE     = 101,
        BOOTLOADER_FRAME_WRITE_PAGE    = 102,
        BOOTLOADER_FRAME_BOOT_TO_APP   = 103,
    };

#pragma pack(push, 1)  // Ensures there in no padding (dummy) bytes in the structures below
    struct GenericMd80Frame64
    {
        uint8_t  frameId;
        uint8_t  canMsgLen = 64;
        uint8_t  timeoutMs = 2;
        uint16_t targetCanId;
        uint8_t  canMsg[64];
    };
#pragma pack(pop)
}  // namespace mab
