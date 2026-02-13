#include "MDCO.hpp"

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
        Error_t err     = enterConfigMode();
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
        Error_t err = enterConfigMode();
        (*m_od)[0x2004][0x5] = (open_types::BOOLEAN_t)true;
        err = writeSDO((*m_od)[0x2004][0x5]);
        if (err != Error_t::OK)
        {
            m_log.error("Error setting Set Zero");
            return err;
        }
        return err;
    }


    MDCO::Error_t MDCO::readSDO(const EDSEntry& edsEntry) const
    {
        if(edsEntry)

        m_log.debug("Read Open register...");
        std::vector<u8> frame = {
            0x40,
            ((u8)index),
            ((u8)(index >> 8)),
            subindex,
            0,
            0,
            0,
            0,
        };

        //  message sending via transferCanFrame
        auto [response, error] =
            transferCanOpenFrame(SDO_REQUEST_BASE + m_canId, frame, frame.size());

        // data display
        std::stringstream ss;

        ss << "\n\n ---- Received CAN Frame Info ----" << "\n";

        u8 cmd = response[0];
        if ((cmd & 0xF0) != 0x40)
        {
            ss << "Frame not recognized as an SDO Upload Expedited response." << "\n";
            return Error_t::TRANSFER_FAILED;
        }
        else
        {
            // FLAGS Extraction
            u8 n       = (cmd & 0x0C) >> 2;  // bits 1-0
            u8 dataLen = 4 - n;

            // Index and Subindex
            u16 index    = response[2] << 8 | response[1];
            u8  subindex = response[3];

            // data display
            ss << "Index      : 0x" << std::hex << std::setw(4) << std::setfill('0') << index
               << "\n";

            ss << "Subindex   : 0x" << std::hex << std::setw(2) << std::setfill('0')
               << (i16)subindex << "\n";

            ss << "Data (" << std::dec << (i16)dataLen << " byte(s)): 0x";

            for (i16 i = dataLen - 1; i >= 0; --i)
            {
                ss << std::hex << std::setw(2) << std::setfill('0') << (i16)response[4 + i];
            }
            ss << "\n"
               << "------------------------" << "\n";
            m_log.info("%s\n", ss.str().c_str());
        }

        if (error == mab::candleTypes::Error_t::OK)
        {
            return Error_t::OK;
        }
        else
        {
            m_log.error("Error in the register write response!");
            return Error_t::TRANSFER_FAILED;
        }
    }

    MDCO::Error_t MDCO::writeLongOpenRegisters(i16                index,
                                               short              subindex,
                                               const std::string& dataString,
                                               bool               force)
    {
        // check if the object is writable unless force is true
        if (!force)
        {
            if (isWritable(index, subindex) != OK)
            {
                m_log.error("Object 0x%04x:0x%02x is not writable!", index, subindex);
                return REQUEST_INVALID;
            }
        }
        std::string motorName = dataString;

        m_log.debug("Writing Motor Name to 0x2000:0x06 via segmented SDO...");

        // 1. prepare data to send clip data to 20 bytes (Motor Name)
        std::vector<u8> data = std::vector<u8>(motorName.begin(), motorName.end());
        // 2. sending init message of segmented transfer
        std::vector<u8> initFrame = {0x21,  // CCS=1, E=1, S=1
                                     u8(index & 0xFF),
                                     u8(index >> 8),
                                     u8(subindex),
                                     u8(data.size() >> 0),
                                     u8(data.size() >> 8),
                                     u8(data.size() >> 16),
                                     u8(data.size() >> 24)};

        auto [initResponse, initError] =
            transferCanOpenFrame(SDO_REQUEST_BASE + m_canId, initFrame, initFrame.size());

        if (initError != mab::candleTypes::Error_t::OK)
        {
            m_log.error("Failed to initiate segmented SDO download.");
            return Error_t::TRANSFER_FAILED;
        }

        if (initResponse.size() < 1 || (initResponse[0] & 0xE0) != 0x60)
        {
            m_log.error("Unexpected response to initiate download (expected 0x60).");
            return Error_t::TRANSFER_FAILED;
        }

        // 3. sending data segments
        size_t offset = 0;
        u8     toggle = 0;

        while (offset < data.size())
        {
            std::vector<u8> segmentFrame;

            size_t remaining     = data.size() - offset;
            size_t segmentLength = std::min<size_t>(7, remaining);
            u8     emptyBytes    = 7 - segmentLength;
            bool   lastSegment   = (segmentLength == remaining);

            u8 cmdByte = 0x00;
            cmdByte |= (toggle & 0x01) << 4;         // bit 4: toggle
            cmdByte |= (emptyBytes & 0x07) << 1;     // bits 3:1: empty bytes
            cmdByte |= (lastSegment ? 0x01 : 0x00);  // bit 0: C (last segment)

            segmentFrame.push_back(cmdByte);

            // segments data
            for (size_t i = 0; i < segmentLength; ++i)
            {
                segmentFrame.push_back(data[offset + i]);
            }

            // padding with zeros if needed
            for (size_t i = segmentLength; i < 7; ++i)
            {
                segmentFrame.push_back(0x00);
            }

            auto [segResponse, segError] =
                transferCanOpenFrame(SDO_REQUEST_BASE + m_canId, segmentFrame, segmentFrame.size());

            if (segError != mab::candleTypes::Error_t::OK)
            {
                m_log.error("Segmented transfer failed at offset {}", offset);
                return Error_t::TRANSFER_FAILED;
            }

            // Check server response: must be 0x20 | toggle
            if (segResponse.size() < 1 || (segResponse[0] & 0xE0) != 0x20)
            {
                m_log.error("Malformed segment ACK.");
                return Error_t::TRANSFER_FAILED;
            }

            if ((segResponse[0] & 0x10) != (toggle << 4))
            {
                m_log.error("Unexpected toggle bit, corrupted transfer.");
                return Error_t::TRANSFER_FAILED;
            }

            offset += segmentLength;
            toggle ^= 0x01;
        }

        m_log.debug("Motor Name successfully written.");
        return Error_t::OK;
    }

    MDCO::Error_t MDCO::readLongOpenRegisters(i16              index,
                                              short            subindex,
                                              std::vector<u8>& outData,
                                              bool             silent)
    {  // check if the object is readable
        if (isReadable(index, subindex) != OK)
        {
            m_log.error("Object 0x%04x:0x%02x is not readable!", index, subindex);
            return Error_t::REQUEST_INVALID;
        }

        m_log.debug("Read Object (0x%lx:0x%x) via segmented SDO…", index, subindex);

        // ---------- 1) Initiation Request ----------
        std::vector<u8> initReq = {
            0x40, u8(index & 0xFF), u8(index >> 8), u8(subindex), 0x00, 0x00, 0x00, 0x00};

        auto [rspInit, errInit] = transferCanOpenFrame(0x600 + m_canId, initReq, initReq.size());
        if (errInit != mab::candleTypes::Error_t::OK || rspInit.size() < 8)
        {
            m_log.error("Failed to initiate SDO read.");
            return Error_t::TRANSFER_FAILED;
        }

        u8   cmd         = rspInit[0];
        bool isExpedited = cmd & 0x02;
        bool hasSize     = cmd & 0x01;

        // ---------- 2a) Expedited transfer ----------
        if (isExpedited)
        {
            m_log.warn("Data received in expedited mode, probably ≤ 4 bytes.");
            u8 n   = ((cmd >> 2) & 0x03);  // number of unused bytes
            u8 len = 4 - n;

            outData.insert(outData.end(), rspInit.begin() + 4, rspInit.begin() + 4 + len);

            // ---------- 3) Display ----------
            if (dataSizeOfEdsObject(index, subindex) == 0)
            {
                std::string motorName(outData.begin(), outData.end());
                if (!silent)
                    m_log.info("Data received (string): '%s'", motorName.c_str());
            }
            else
            {
                if (!silent)
                    m_log.info("Data received: %s",
                               std::string(outData.begin(), outData.end()).c_str());
            }
            return Error_t::OK;
        }

        // ---------- 2b) Segmented transfer ----------
        u32 totalLen = 0;
        if (hasSize)
        {
            totalLen = rspInit[4] | (rspInit[5] << 8) | (rspInit[6] << 16) | (rspInit[7] << 24);
            outData.reserve(totalLen);
        }

        bool toggle   = false;
        bool finished = false;

        while (!finished)
        {
            std::vector<u8> segReq = {u8(0x60 | (toggle ? 0x10 : 0x00)), 0, 0, 0, 0, 0, 0, 0};
            auto [rspSeg, errSeg] =
                transferCanOpenFrame(SDO_REQUEST_BASE + m_canId, segReq, segReq.size());

            if (errSeg != mab::candleTypes::Error_t::OK || rspSeg.size() < 1)
            {
                m_log.error("Error segment reading");
                return Error_t::TRANSFER_FAILED;
            }

            u8 segCmd = rspSeg[0];
            if ((segCmd & 0x10) != (toggle ? 0x10 : 0x00))
            {
                m_log.error("Bit toggle unexpected, corrupt transfer.");
                return Error_t::TRANSFER_FAILED;
            }

            bool last    = segCmd & 0x01;
            u8   unused  = (segCmd >> 1) & 0x07;
            u8   dataLen = 7 - unused;

            if ((i16)rspSeg.size() < (1 + dataLen))
            {
                m_log.error("Incomplete data in the segment.");
                return Error_t::TRANSFER_FAILED;
            }

            outData.insert(outData.end(), rspSeg.begin() + 1, rspSeg.begin() + 1 + dataLen);
            finished = last;
            toggle   = !toggle;
        }

        if (hasSize && outData.size() != totalLen)
        {
            m_log.warn("Size of data read (%d) ≠ size announced (%d)", outData.size(), totalLen);
        }

        // ---------- 3) Display ----------

        if (dataSizeOfEdsObject(index, subindex) == 0)
        {
            std::string motorName(outData.begin(), outData.end());
            if (!silent)
                m_log.info("Data received (string): '%s'", motorName.c_str());
        }
        else
        {
            if (!silent)
                m_log.info("Data received: %s",
                           std::string(outData.begin(), outData.end()).c_str());
        }

        return Error_t::OK;
    }

    i32 MDCO::getValueFromOpenRegister(i16 index, u8 subindex)
    {
        // check if the object is readable
        if (isReadable(index, subindex) != OK)
        {
            m_log.error("Object 0x%04x:0x%02x is not writable!", index, subindex);
            return Error_t::REQUEST_INVALID;
        }
        m_log.debug("Read Open register...");
        // send sdo upload expedited request
        std::vector<u8> frame = {
            0x40,
            ((u8)index),
            ((u8)(index >> 8)),
            subindex,
            0,
            0,
            0,
            0,
        };

        auto [response, error] =
            transferCanOpenFrame(SDO_REQUEST_BASE + m_canId, frame, frame.size());

        i32 answerValue = 0;
        for (i16 i = 0; i <= 4; i++)
        {
            answerValue += (((i32)response[4 + i]) << (8 * i));
        }

        // data display
        u8 cmd = response[0];

        if ((cmd & 0xF0) != 0x40)
        {
            m_log.error("Frame not recognized as an SDO Upload Expedited response.");
            return -1;
        }
        if (error == mab::candleTypes::Error_t::OK)
        {
            return answerValue;
        }
        else
        {
            m_log.error("Error in the register write response!");
            return -1;
        }
    }

    MDCO::Error_t MDCO::writeOpenRegisters(
        i16 index, short subindex, i32 data, short size, bool force)
    {
        // check if the object is writable unless force is true
        if (!force)
        {
            if (isWritable(index, subindex) != OK)
            {
                m_log.error("Object 0x%04x:0x%02x is not writable!", index, subindex);
                return Error_t::REQUEST_INVALID;
            }
        }
        // if size is 0, then read the size from the EDS file
        if (size == 0)
        {
            size = dataSizeOfEdsObject(index, subindex);
            if (size == -1)
            {
                // size= -1 mean object not found in EDS file
                m_log.error(
                    "Object 0x%04x:0x%02x has an unsupported size (%d)!", index, subindex, size);
                return Error_t::REQUEST_INVALID;
            }
            else if (size == 0 || size > 4)
            {
                // size=0 mean object is string or array, size>4 not supported in expedited transfer
                m_log.error(
                    "Object 0x%04x:0x%02x has an unsupported size (%d), please use an "
                    "Segmented transfer !",
                    index,
                    subindex,
                    size);
                return Error_t::REQUEST_INVALID;
            }
        }
        // send sdo download expedited request
        std::vector<u8> frame;
        frame.reserve(8);
        if (size == 1)
            frame.push_back(0x2F);
        if (size == 2)
            frame.push_back(0x2B);
        if (size == 4)
            frame.push_back(0x23);
        frame.push_back(((u8)index));         // Index LSB
        frame.push_back(((u8)(index >> 8)));  // Index MSB
        frame.push_back(subindex);            // Subindex
        frame.push_back((u8)data);            // data
        frame.push_back((u8)(data >> 8));
        frame.push_back((u8)(data >> 16));
        frame.push_back((u8)(data >> 24));

        auto [response, error] =
            transferCanOpenFrame(SDO_REQUEST_BASE + m_canId, frame, frame.size());

        if (error == mab::candleTypes::Error_t::OK)
        {
            return Error_t::OK;
        }
        else
        {
            m_log.error("Error in the register write response!");
            return Error_t::TRANSFER_FAILED;
        }
    }

    MDCO::Error_t MDCO::writeOpenRegisters(const std::string& name, u32 data, u8 size, bool force)
    {  // search index and subindex from the EDS file corresponding to the name
        u32 index    = 0;
        u8  subIndex = 0;
        if (findObjectByName(name, index, subIndex) != OK)
        {
            m_log.error("%s not found in EDS file", name.c_str());
            return Error_t::UNKNOWN_OBJECT;
        }
        if (!force)
        {
            if (isWritable(index, subIndex) != OK)
            {
                m_log.error("Object 0x%04x:0x%02x is not writable!", index, subIndex);
                return Error_t::REQUEST_INVALID;
            }
        }
        auto err = writeOpenRegisters(index, subIndex, data, size);
        if (m_log.g_m_verbosity == Logger::Verbosity_E::VERBOSITY_3)
            m_log.debug("Error:%d\n", readOpenRegisters(index, subIndex));
        return err;
    }

    MDCO::Error_t MDCO::writeOpenPDORegisters(i16 index, std::vector<u8> data)
    {
        // send PDO write request, PDO is slave/master communication mode so the motor will not
        // respond
        m_log.debug("Writing Open Pdo register...");

        auto [response, error] = transferCanOpenFrameNoRespondExpected(index, data, data.size());

        if (error == mab::candleTypes::Error_t::OK)
        {
            return Error_t::OK;
        }
        else
        {
            m_log.error("Error in the register write response!");
            return Error_t::TRANSFER_FAILED;
        }
    }

    MDCO::Error_t MDCO::sendCustomData(i16 index, std::vector<u8> data)
    {
        m_log.debug("Writing Custom data...");
        transferCanOpenFrameNoRespondExpected(index, data, data.size());
        return Error_t::OK;
    }

    std::vector<canId_t> MDCO::discoverOpenMDs(Candle* candle)
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
            MDCO md(id, candle);
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
