#include "bl3Loader.hpp"

#include <cstring>
#include <vector>

namespace mab
{
    namespace
    {
        enum Command : u8
        {
            IDENTIFY = 0xC0,
            START    = 0xC1,
            DATA     = 0xC2,
            END      = 0xC3,
            BOOT     = 0xC4,
            ERASE    = 0xC5,
        };

        constexpr u8  STATUS_OK      = 0;
        constexpr u32 HEADER_SIZE    = 56;  // "MABF" | size | iv | sha256
        constexpr u32 FRAME_SIZE     = 64;
        constexpr u32 DATA_PAYLOAD   = FRAME_SIZE - 3;
        constexpr u32 ATTEMPTS       = 5;    // per request, while the reply is lost
        constexpr u32 IDENTIFY_MS    = 50;   // short: polled while the drive resets
        constexpr u32 TIMEOUT_MS     = 250;  // page erase / SHA-256 of the image; u8 in CANdle
        constexpr u32 PROGRESS_STEP  = 64;   // DATA frames between progress updates
        constexpr u32 CONFIG_ADDRESS = 0x0800C000;  // first Storage page, right after Bl3
        constexpr u32 PAGE_SIZE      = 0x2000;      // one 8 KB page

        Logger log(Logger::ProgramLayer_E::LAYER_2, "BL3 LOADER");

        const char* statusName(u8 status)
        {
            switch (status)
            {
                case 0:
                    return "OK";
                case 1:
                    return "BAD_STATE (no session / download)";
                case 2:
                    return "REFUSED (file header, or erase range)";
                case 3:
                    return "TOO_LONG (more bytes than the header says)";
                case 4:
                    return "TOO_SHORT (END before all the bytes)";
                case 5:
                    return "WRONG_FIRMWARE (wrong key, or firmware for another board)";
                case 6:
                    return "FLASH_FAILED (erase, program or read back)";
                case 7:
                    return "CORRUPT (SHA-256 of the flash does not match)";
                case 0x80:
                    return "UNKNOWN command";
                case 0x81:
                    return "BAD_LENGTH";
                case 0x82:
                    return "BAD_SEQ";
            }
            return "unknown status";
        }

        u32 readU32(const u8* p)
        {
            u32 v;
            memcpy(&v, p, sizeof(v));
            return v;
        }

        /// @brief Send a request until its reply comes, at most ATTEMPTS times. A reply is the one
        /// for this request when it starts with the command (and for DATA carries the seq), a
        /// late reply to an earlier request is skipped and the request sent again.
        /// @return true and the status in *status, false when no reply came
        bool request(Candle* candle, canId_t canId, std::vector<u8> frame, u8* status)
        {
            const u8 command = frame[0];
            if (frame.size() > 1)
                frame.resize(FRAME_SIZE);  // full DLC: the padding is ignored by the bootloader
            for (u32 i = 0; i < ATTEMPTS; i++)
            {
                auto [reply, err] = candle->transferCANFrame(canId, frame, FRAME_SIZE, TIMEOUT_MS);
                if (err != candleTypes::Error_t::OK || reply.size() < 3 || reply[0] != command)
                    continue;
                if (command == DATA && reply[2] != frame[1])
                    continue;
                *status = reply[1];
                return true;
            }
            return false;
        }

        bool step(Candle* candle, canId_t canId, std::vector<u8> frame, const char* name)
        {
            u8 status = 0;
            if (!request(candle, canId, frame, &status))
            {
                log.error("%s: no reply from the bootloader", name);
                return false;
            }
            if (status != STATUS_OK)
            {
                log.error("%s: %s", name, statusName(status));
                return false;
            }
            return true;
        }
    }  // namespace

    bl3Loader::Identify_E bl3Loader::identify(Candle* candle,
                                              canId_t canId,
                                              u32     attempts,
                                              Info*   info)
    {
        const std::vector<u8> frame = {IDENTIFY};
        for (u32 i = 0; attempts == 0 || i < attempts; i++)
        {
            auto [reply, err] = candle->transferCANFrame(canId, frame, FRAME_SIZE, IDENTIFY_MS);
            if (err != candleTypes::Error_t::OK || reply.size() < 13)
                continue;
            if (reply[0] == 'N' && reply[1] == 'O')
                return Identify_E::OTHER;
            if (reply[0] != IDENTIFY || reply[1] != STATUS_OK)
                continue;
            info->protocol       = reply[2];
            info->deviceType     = reply[3];
            info->deviceRevision = reply[4];
            info->blVersion      = readU32(&reply[5]);
            info->maxImageSize   = readU32(&reply[9]);
            return Identify_E::BL3;
        }
        return Identify_E::NO_REPLY;
    }

    bl3Loader::Result_E bl3Loader::flashAndBoot(Candle*              candle,
                                                canId_t              canId,
                                                const MabFileParser& mabFile,
                                                const Info&          info)
    {
        const MabFileParser::FirmwareEntry& fw = mabFile.m_fwEntry;
        version_ut                          bl = {.i = info.blVersion};
        log.info("G5 bootloader v%d.%d.%d, protocol %d, device type %d rev %d",
                 bl.s.major,
                 bl.s.minor,
                 bl.s.revision,
                 info.protocol,
                 info.deviceType,
                 info.deviceRevision);
        if (info.protocol != PROTOCOL)
        {
            log.error("Bootloader protocol %d is not supported, this candletool speaks %d",
                      info.protocol,
                      PROTOCOL);
            return Result_E::FAILED;
        }
        if (fw.size > info.maxImageSize)
        {
            log.error("Firmware of %u bytes does not fit, the drive takes up to %u bytes",
                      fw.size,
                      info.maxImageSize);
            return Result_E::FAILED;
        }

        // START: the file header, the bootloader checks it before anything is erased
        std::vector<u8> frame = {START, 'M', 'A', 'B', 'F'};
        frame.resize(1 + HEADER_SIZE);
        memcpy(&frame[5], &fw.size, sizeof(fw.size));
        memcpy(&frame[9], fw.aes_iv, sizeof(fw.aes_iv));
        memcpy(&frame[25], fw.checksum, sizeof(fw.checksum));
        if (!step(candle, canId, frame, "START"))
            return Result_E::FAILED;

        // DATA: the firmware as in the .mab file, seq wraps at 255
        const u8* data = fw.data->data();
        u8        seq  = 0;
        for (u32 done = 0; done < fw.size; seq++)
        {
            const u32 n = fw.size - done < DATA_PAYLOAD ? fw.size - done : DATA_PAYLOAD;
            frame       = {DATA, seq, (u8)n};
            frame.insert(frame.end(), data + done, data + done + n);
            if (!step(candle, canId, frame, "DATA"))
            {
                log.error("Transfer stopped at byte %u of %u", done, fw.size);
                return Result_E::FAILED;
            }
            done += n;
            if (seq % PROGRESS_STEP == 0)
                log.progress((double)done / fw.size);
        }
        log.progress(1.0);

        // END: the bootloader checks the SHA-256 of the image in flash
        if (!step(candle, canId, {END}, "END"))
            return Result_E::FAILED;

        // BOOT: the application starts ~2 ms after the reply, which can get lost on the way
        u8 status = 0;
        if (!request(candle, canId, {BOOT}, &status))
        {
            log.warn("BOOT: no reply from the bootloader");
            return Result_E::BOOT_UNCONFIRMED;
        }
        if (status != STATUS_OK)
        {
            log.error("BOOT: %s", statusName(status));
            return Result_E::FAILED;
        }
        log.success("Flashing successful!");
        return Result_E::OK;
    }

    bool bl3Loader::eraseConfig(Candle* candle, canId_t canId)
    {
        std::vector<u8> frame = {ERASE};
        frame.resize(9);
        memcpy(&frame[1], &CONFIG_ADDRESS, sizeof(CONFIG_ADDRESS));
        memcpy(&frame[5], &PAGE_SIZE, sizeof(PAGE_SIZE));
        if (!step(candle, canId, frame, "ERASE"))
            return false;

        // BOOT: the application starts with a default config; the erase is done either way
        u8 status = 0;
        if (!request(candle, canId, {BOOT}, &status))
            log.warn("BOOT: no reply from the bootloader");
        else if (status != STATUS_OK)
            log.warn("BOOT: %s, the drive stays in the bootloader", statusName(status));
        return true;
    }
}  // namespace mab
