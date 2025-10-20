#pragma once
#include "mab_def.hpp"
#include "mab_types.hpp"
#include <cstring>
#include <type_traits>
namespace mab
{
    /// @brief DTO struct to be sent via CANdle buses
    struct CANdleFrameDTO
    {
        canId_t canId          = 0;
        u16     timeout        = 0;  // 100 us
        u8      length         = 0;  // data length in bytes
        u8      sequenceNumber = 0;  // start at 1 for the first frame in the package
                                     // and increment by 1 for each frame
        u8 data[64] = {0};
    };

    /// @brief Helper for dealing with CANdle frame DTO
    struct CANdleFrame
    {
      private:
        CANdleFrameDTO frameDTO = {0};

      public:
        static constexpr u8     DTO_PARSE_ID    = 0x0F;
        static constexpr u8     DATA_MAX_LENGTH = 0x40;
        static constexpr size_t DTO_SIZE =
            sizeof(CANdleFrameDTO::canId) + sizeof(CANdleFrameDTO::timeout) +
            sizeof(CANdleFrameDTO::length) + sizeof(CANdleFrameDTO::sequenceNumber) +
            (sizeof(CANdleFrameDTO::data) / sizeof(*CANdleFrameDTO::data));

        enum class Error_t
        {
            UNKNOWN,
            OK,
            MSG_TOO_LONG
        };
        inline void clear()
        {
            frameDTO = {0};
        }
        inline void init(canId_t                           canId,
                         decltype(frameDTO.sequenceNumber) sequenceNumber,
                         decltype(frameDTO.timeout)        timeout)
        {
            clear();
            frameDTO.canId          = canId;
            frameDTO.sequenceNumber = sequenceNumber;
            frameDTO.timeout        = timeout;
        }
        inline bool isValid() const
        {
            return frameDTO.canId != 0 && frameDTO.length != 0 &&
                   frameDTO.length <= sizeof(frameDTO.data);
        }

        inline bool isValid(decltype(frameDTO.sequenceNumber) expectedSeqNum) const
        {
            return frameDTO.canId != 0 && frameDTO.length != 0 &&
                   frameDTO.length <= sizeof(frameDTO.data) &&
                   frameDTO.sequenceNumber == expectedSeqNum;
        }

        inline void serialize(void* buffer) const
        {
            *(decltype(frameDTO.canId)*)buffer          = frameDTO.canId;
            buffer                                      = (u8*)buffer + sizeof(frameDTO.canId);
            *(decltype(frameDTO.timeout)*)buffer        = frameDTO.timeout;
            buffer                                      = (u8*)buffer + sizeof(frameDTO.timeout);
            *(decltype(frameDTO.length)*)buffer         = frameDTO.length;
            buffer                                      = (u8*)buffer + sizeof(frameDTO.length);
            *(decltype(frameDTO.sequenceNumber)*)buffer = frameDTO.sequenceNumber;
            buffer = (u8*)buffer + sizeof(frameDTO.sequenceNumber);
            std::memcpy(buffer, frameDTO.data, frameDTO.length);
        }
        inline void deserialize(void* buffer)
        {
            clear();
            frameDTO.canId          = *(decltype(frameDTO.canId)*)buffer;
            buffer                  = (u8*)buffer + sizeof(frameDTO.canId);
            frameDTO.timeout        = *(decltype(frameDTO.timeout)*)buffer;
            buffer                  = (u8*)buffer + sizeof(frameDTO.timeout);
            frameDTO.length         = *(decltype(frameDTO.length)*)buffer;
            buffer                  = (u8*)buffer + sizeof(frameDTO.length);
            frameDTO.sequenceNumber = *(decltype(frameDTO.sequenceNumber)*)buffer;
            buffer                  = (u8*)buffer + sizeof(frameDTO.sequenceNumber);
            std::memcpy(frameDTO.data, buffer, frameDTO.length);
        }
        inline Error_t addData(const void* data, decltype(frameDTO.length) size)
        {
            if (frameDTO.length + size > sizeof(frameDTO.data))
                return Error_t::MSG_TOO_LONG;
            std::memcpy(frameDTO.data + frameDTO.length, data, size);
            frameDTO.length += size;
            return Error_t::OK;
        }
        inline void clearData()
        {
            std::memset(frameDTO.data, 0, sizeof(frameDTO.data));
            frameDTO.length = 0;
        }
        inline const std::remove_cvref_t<decltype(frameDTO.data[0])>* data() const
        {
            return &frameDTO.data[0];
        }
        inline decltype(frameDTO.sequenceNumber) sequenceNo() const
        {
            return frameDTO.sequenceNumber;
        }
        inline decltype(frameDTO.canId) canId() const
        {
            return frameDTO.canId;
        }
        inline decltype(frameDTO.length) length() const
        {
            return frameDTO.length;
        }
        inline decltype(frameDTO.timeout) timeout() const
        {
            return frameDTO.timeout;
        }
    };
}  // namespace mab