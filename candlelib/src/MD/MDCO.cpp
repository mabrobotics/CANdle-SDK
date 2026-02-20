#include "MDCO.hpp"
#include <unistd.h>
#include <cstddef>
#include <span>
#include <vector>
#include "candle_types.hpp"

namespace mab
{

    MDCO::Error_t MDCO::init()
    {
        return readSDO((*m_od)[0x1000]);
    }

    MDCO::Error_t MDCO::enable()
    {
        // set the mode of operation and enable the driver, log an error message if transfer failed
        Error_t err;
        (*m_od)[0x6060] = (open_types::INTEGER8_t)6;
        err             = writeSDO((*m_od)[0x6060]);
        if (err != Error_t::OK)
        {
            m_log.error("Error sending shutdown cmd!");
            return err;
        }
        (*m_od)[0x6060] = (open_types::INTEGER8_t)15;
        err             = writeSDO((*m_od)[0x6060]);
        if (err != Error_t::OK)
        {
            m_log.error("Error sending switch on and enable cmd!");
            return err;
        }
        return Error_t::OK;
    }

    MDCO::Error_t MDCO::disable()
    {
        // disable the driver, log an error message if transfer failed
        Error_t err;
        (*m_od)[0x6060] = (open_types::INTEGER8_t)7;
        err             = writeSDO((*m_od)[0x6060]);
        if (err != Error_t::OK)
        {
            m_log.error("Error sending disable operation cmd!");
            return err;
        }
        return Error_t::OK;
    }

    MDCO::Error_t MDCO::blink()
    {
        // blink the motor led, log an error message if transfer failed
        Error_t err          = enterConfigMode();
        (*m_od)[0x2004][0x1] = (open_types::BOOLEAN_t)1;
        err                  = writeSDO((*m_od)[0x2004][0x1]);
        if (err != Error_t::OK)
        {
            m_log.error("Error setting Blink LEDs");
            return err;
        }
        return err;
    }

    MDCO::Error_t MDCO::save()
    {
        Error_t err     = MDCO::Error_t::OK;
        (*m_od)[0x6060] = (open_types::INTEGER8_t)6;
        err             = writeSDO((*m_od)[0x6060]);
        if (err != Error_t::OK)
        {
            m_log.error("Error sending shutdown cmd!");
            return err;
        }
        (*m_od)[0x1010][0x1] =
            (open_types::UNSIGNED32_t)0x65766173;  // 0x65766173="save" in ASCII and little endian
        err = writeSDO((*m_od)[0x1010][0x1]);
        if (err != Error_t::OK)
        {
            m_log.error("Error saving all parameters");
            return err;
        }
        return err;
    }

    MDCO::Error_t MDCO::zero()
    {
        // set the motor zero position to the actual position via SDO message, log an error message
        // if transfer failed
        Error_t err          = enterConfigMode();
        (*m_od)[0x2004][0x5] = (open_types::BOOLEAN_t)1;
        err                  = writeSDO((*m_od)[0x2004][0x5]);
        if (err != Error_t::OK)
        {
            m_log.error("Error setting Blink LEDs");
            return err;
        }
        return err;
    }

    MDCO::Error_t MDCO::readSDO(EDSEntry& edsEntry) const
    {
        std::vector<std::byte> result;
        if (!edsEntry.getValueMetaData().has_value())
        {
            return Error_t::UNKNOWN_OBJECT;
        }

        if (edsEntry.valueSize() <= 4)
        {
            // using expedited transfer
            std::vector<u8> transmitFrame = {
                INITIATE_SDO_UPLOAD_REQUEST,
                (u8)edsEntry.getEntryMetaData().address.first,
                (u8)(edsEntry.getEntryMetaData().address.first >> 8),
                (u8)(edsEntry.getEntryMetaData().address.second.value_or(0))};
            transmitFrame.resize(8, 0);

            auto [response, error] = transferCanOpenFrame(
                SDO_REQUEST_BASE + m_canId, transmitFrame, transmitFrame.size());

            if (error != candleTypes::Error_t::OK)
            {
                m_log.error("Failed upload SDO 0x%x", SDO_REQUEST_BASE + m_canId);
                return Error_t::TRANSFER_FAILED;
            }
            m_log.debug("Address: 0x%x", edsEntry.getEntryMetaData().address.first);

            // Verify expedited response (bit 1 == 1)
            if ((response[0] & 0x40) == 0)
            {
                m_log.error("Invalid expedited download response");

                return Error_t::TRANSFER_FAILED;
            }

            // Number of unused bytes (bits 2-3)
            u8     emptyBytes = (response[0] >> 2) & 0x03;
            size_t dataSize   = 4 - emptyBytes;

            result.reserve(dataSize);

            for (size_t i = 0; i < dataSize; ++i)
            {
                result.push_back(static_cast<std::byte>(response[4 + i]));
            }
        }
        else
        {
            // using segmented transfer

            std::vector<u8> transmitFrame = {
                INITIATE_SDO_UPLOAD_REQUEST,
                (u8)edsEntry.getEntryMetaData().address.first,
                (u8)(edsEntry.getEntryMetaData().address.first >> 8),
                (u8)(edsEntry.getEntryMetaData().address.second.value_or(0))};
            transmitFrame.resize(8, 0);

            auto [response, error] = transferCanOpenFrame(
                SDO_REQUEST_BASE + m_canId, transmitFrame, transmitFrame.size());

            if (error != candleTypes::Error_t::OK)
            {
                m_log.error("Failed initiate segmented upload SDO 0x%x",
                            SDO_REQUEST_BASE + m_canId);
                return Error_t::TRANSFER_FAILED;
            }

            // Must NOT be expedited
            if ((response[0] & 0x02) != 0)
            {
                m_log.error("Unexpected expedited response during segmented upload");
                return Error_t::TRANSFER_FAILED;
            }

            std::vector<u8> completeData;
            bool            lastSegment = false;
            u8              toggle      = 0;

            while (!lastSegment)
            {
                std::vector<u8> segmentRequest(8, 0);
                segmentRequest[0] = 0x60 | (toggle << 4);

                auto [segmentResponse, segError] = transferCanOpenFrame(
                    SDO_REQUEST_BASE + m_canId, segmentRequest, segmentRequest.size());

                if (segError != candleTypes::Error_t::OK)
                {
                    m_log.error("Segment upload failed SDO 0x%x", SDO_REQUEST_BASE + m_canId);
                    return Error_t::TRANSFER_FAILED;
                }

                lastSegment   = (segmentResponse[0] & 0x01);
                u8 emptyBytes = (segmentResponse[0] >> 1) & 0x07;

                size_t dataSize = 7 - emptyBytes;

                completeData.insert(completeData.end(),
                                    segmentResponse.begin() + 1,
                                    segmentResponse.begin() + 1 + dataSize);

                toggle ^= 1;
            }

            // Move into result as std::byte
            result.reserve(completeData.size());
            for (u8 b : completeData)
            {
                result.push_back(static_cast<std::byte>(b));
            }
        }
        if (edsEntry.setSerializedValue(result) == EDSEntry::Error_t::OK)
            return Error_t::OK;
        else
        {
            m_log.error("EDS parsing failed with code: %d", edsEntry.setSerializedValue(result));
            return Error_t::REQUEST_INVALID;
        }
    }

    MDCO::Error_t MDCO::writeSDO(EDSEntry& edsEntry) const
    {
        if (!edsEntry.getValueMetaData().has_value())
        {
            return Error_t::UNKNOWN_OBJECT;
        }

        const std::vector<std::byte>& data        = edsEntry.getSerializedValue();
        const size_t                  payloadSize = edsEntry.valueSize();
        const size_t                  size        = data.size();

        if (payloadSize <= 4)
        {
            // -------- Expedited download --------
            std::vector<u8> transmitFrame(8, 0);

            // Command specifier:
            // 0x20 = initiate download
            // 0x02 = expedited
            // 0x01 = size indicated
            // bits 2-3 = number of unused bytes
            // if (payloadSize == 0x1)
            //     transmitFrame[0] = 0x2F;
            // else if (payloadSize == 0x2)
            //     transmitFrame[0] = 0x2B;
            // else if (payloadSize == 0x3)
            //     transmitFrame[0] = 0x27;
            // else
            //     transmitFrame[0] = 0x23;
            transmitFrame[0] = INITIATE_SDO_DOWNLOAD_REQUEST;
            transmitFrame[1] = (u8)edsEntry.getEntryMetaData().address.first;
            transmitFrame[2] = (u8)(edsEntry.getEntryMetaData().address.first >> 8);
            transmitFrame[3] = (u8)(edsEntry.getEntryMetaData().address.second.value_or(0));

            for (size_t i = 0; i < size; ++i)
            {
                transmitFrame[4 + i] = static_cast<u8>(data[i]);
            }

            auto [response, error] = transferCanOpenFrame(
                SDO_REQUEST_BASE + m_canId, transmitFrame, transmitFrame.size());

            if (error != candleTypes::Error_t::OK)
            {
                m_log.error("Failed expedited download SDO 0x%x", SDO_REQUEST_BASE + m_canId);
                return Error_t::TRANSFER_FAILED;
            }
            m_log.debug("Address: 0x%x", edsEntry.getEntryMetaData().address.first);
            m_log.debug("Lenght: 0x%x", payloadSize);

            // Expect initiate download response (0x60)
            if ((response[0] & 0xE0) != 0x60)
            {
                m_log.error("Invalid expedited download response");
                return Error_t::TRANSFER_FAILED;
            }
        }
        else
        {
            // -------- Segmented download --------

            // ---- Initiate segmented download ----
            std::vector<u8> transmitFrame(8, 0);

            // 0x20 = initiate download (no expedited, no size indicated here)
            transmitFrame[0] = 0x20;
            transmitFrame[1] = (u8)edsEntry.getEntryMetaData().address.first;
            transmitFrame[2] = (u8)(edsEntry.getEntryMetaData().address.first >> 8);
            transmitFrame[3] = (u8)(edsEntry.getEntryMetaData().address.second.value_or(0));

            auto [response, error] = transferCanOpenFrame(
                SDO_REQUEST_BASE + m_canId, transmitFrame, transmitFrame.size());

            if (error != candleTypes::Error_t::OK)
            {
                m_log.error("Failed initiate segmented download SDO 0x%x",
                            SDO_REQUEST_BASE + m_canId);
                return Error_t::TRANSFER_FAILED;
            }

            if ((response[0] & 0xE0) != 0x60)
            {
                m_log.error("Invalid initiate segmented download response");
                return Error_t::TRANSFER_FAILED;
            }

            // ---- Send segments ----
            size_t offset      = 0;
            u8     toggle      = 0;
            bool   lastSegment = false;

            while (!lastSegment)
            {
                std::vector<u8> segmentFrame(8, 0);

                size_t remaining = size - offset;
                size_t chunkSize = (remaining > 7) ? 7 : remaining;

                lastSegment = (remaining <= 7);

                u8 emptyBytes = static_cast<u8>(7 - chunkSize);

                // 0x00 = download segment
                // bit 4 = toggle
                // bit 0 = last segment
                // bits 1-3 = number of unused bytes (only valid for last segment)
                segmentFrame[0] = (toggle << 4) | (lastSegment ? 0x01 : 0x00) |
                                  (lastSegment ? (emptyBytes << 1) : 0x00);

                for (size_t i = 0; i < chunkSize; ++i)
                {
                    segmentFrame[1 + i] = static_cast<u8>(data[offset + i]);
                }

                auto [segmentResponse, segError] = transferCanOpenFrame(
                    SDO_REQUEST_BASE + m_canId, segmentFrame, segmentFrame.size());

                if (segError != candleTypes::Error_t::OK)
                {
                    m_log.error("Segment download failed SDO 0x%x", SDO_REQUEST_BASE + m_canId);
                    return Error_t::TRANSFER_FAILED;
                }

                // Expect segment response (0x20 | toggle<<4)
                if ((segmentResponse[0] & 0xE0) != 0x20)
                {
                    m_log.error("Invalid segment download response");
                    return Error_t::TRANSFER_FAILED;
                }

                offset += chunkSize;
                toggle ^= 1;
            }
        }

        return Error_t::OK;
    }

    MDCO::Error_t MDCO::writePDO(EDSEntry& edsEntry) const
    {
        // TODO: PDO Mapping todo
        return MDCO::Error_t::UNKNOWN_OBJECT;
    }

    std::vector<canId_t> MDCO::discoverOpenMDs(Candle*                              candle,
                                               std::shared_ptr<EDSObjectDictionary> od)
    {
        constexpr canId_t MIN_VALID_ID = 0x01;  // ids less than that are reserved for special
        constexpr canId_t MAX_VALID_ID = 0x7F;  // 0x600-0x580=0x7F

        std::vector<u8> frame = {
            0x40,  // Command: initiate upload
            0x00,  // Index LSB
            0x10,  // Index MSB
            0,     // Subindex
            0,     // Padding
            0,
            0,
            0,
        };

        Logger               log(Logger::ProgramLayer_E::TOP, "MD_DISCOVERY");
        std::vector<canId_t> ids;

        if (candle == nullptr)
        {
            log.error("Candle is empty!");
            return std::vector<canId_t>();
        }

        log.info("Looking for MDs");

        for (canId_t id = MIN_VALID_ID; id < MAX_VALID_ID; id++)
        {
            log.debug("Trying to bind MD with id %d", id);
            log.progress(float(id) / float(MAX_VALID_ID));
            // workaround for ping error spam
            Logger::Verbosity_E prevVerbosity =
                Logger::g_m_verbosity.value_or(Logger::Verbosity_E::VERBOSITY_1);
            Logger::g_m_verbosity = Logger::Verbosity_E::SILENT;
            MDCO md(id, candle, od);
            auto [response, error] = md.transferCanOpenFrame(0x600 + id, frame, frame.size());

            if (response[4] == 0x92)
                ids.push_back(id);

            Logger::g_m_verbosity = prevVerbosity;
        }
        for (canId_t id : ids)
        {
            log.info("Discovered MD device with ID: %d", id);
        }
        if (ids.size() > 0)
            return ids;

        log.warn("Have not found any MD devices on the CAN bus!");
        return ids;
    }
}  // namespace mab
