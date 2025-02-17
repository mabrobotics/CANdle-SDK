#pragma once

#include <stdint.h>
#include <string.h>

#include <type_traits>

#include "md_types.hpp"
#include "mab_types.hpp"
#include "manufacturer_data.hpp"

namespace mab
{
    /* forward declaration to deal with cyclic dependency*/
    class Candle;

    /* adding a new field:
    1. add a new field in the mdRegister_E enum (must be uniform with the same enum on MD80 side)
    2. add it to the "switch case" in register.cpp in getType() this switch is also used for the
    size
    3. add it to either RO/RW structs */

    /* READ ONLY PARAMS */
    typedef struct
    {
        uint32_t            firmwareVersion;
        uint8_t             legacyHardwareVersion;
        mab::hardwareType_S hardwareType;
        uint32_t            buildDate;
        char                commitHash[8];
        uint8_t             bridgeType;
        float               resistance;
        float               inductance;
        uint16_t            quickStatus;
        float               mosfetTemperature;
        float               motorTemperature;
        float               mainEncoderVelocity;
        float               mainEncoderPosition;
        float               motorTorque;
        float               outputEncoderVelocity;
        float               outputEncoderPosition;
        float               calOutputEncoderStdDev;
        float               calOutputEncoderMinE;
        float               calOutputEncoderMaxE;
        float               calMainEncoderStdDev;
        float               calMainEncoderMinE;
        float               calMainEncoderMaxE;
        uint32_t            mainEncoderErrors;
        uint32_t            outputEncoderErrors;
        uint32_t            calibrationErrors;
        uint32_t            bridgeErrors;
        uint32_t            hardwareErrors;
        uint32_t            communicationErrors;
        uint32_t            homingErrors;
        uint32_t            motionErrors;
        u32                 miscStatus;
        float               shuntResistance;
    } regRO_st;

    /* READ WRITE PARAMS */
    typedef struct
    {
        char                       motorName[24];
        uint32_t                   canId;
        uint32_t                   canBaudrate;
        uint16_t                   canWatchdog;
        uint8_t                    canTermination;
        uint32_t                   polePairs;
        uint16_t                   motorKV;
        uint8_t                    motorCalibrationMode;
        uint8_t                    motorThermistorType;
        float                      motorKt;
        float                      motorKt_a;
        float                      motorKt_b;
        float                      motorKt_c;
        float                      iMax;
        float                      gearRatio;
        uint8_t                    outputEncoder;
        uint8_t                    outputEncoderMode;
        uint8_t                    outputEncoderCalibrationMode;
        float                      outputEncoderDir;
        uint16_t                   torqueBandwidth;
        uint32_t                   outputEncoderDefaultBaud;
        float                      friction;
        float                      stiction;
        uint8_t                    motorShutdownTemp;
        ImpedanceControllerGains_t impedancePdGains;
        PidControllerGains_t       velocityPidGains;
        PidControllerGains_t       positionPidGains;
        uint8_t                    homingMode;
        float                      homingMaxTravel;
        float                      homingVelocity;
        float                      homingTorque;
        float                      positionLimitMax;
        float                      positionLimitMin;
        float                      maxAcceleration;
        float                      maxDeceleration;
        float                      maxTorque;
        float                      maxVelocity;
        float                      profileAcceleration;
        float                      profileDeceleration;
        float                      profileVelocity;
        float                      quickStopDeceleration;
        float                      positionWindow;
        float                      velocityWindow;
        float                      targetPosition;
        float                      targetVelocity;
        float                      targetTorque;
        uint8_t                    motionMode;
        uint16_t                   state;
        uint8_t                    reverseDirection;
        uint8_t                    brakeMode;
    } regRW_st;

    typedef struct
    {
        regRO_st RO;
        regRW_st RW;
    } regRead_st;

    typedef struct
    {
        regRW_st RW;
    } regWrite_st;

    class Register
    {
      public:
        enum class type
        {
            UNKNOWN = 0,
            U8      = 1,
            I8      = 2,
            U16     = 3,
            I16     = 4,
            U32     = 5,
            I32     = 6,
            F32     = 7,
            STR     = 8,
            REGARR  = 9
        };

        /**
        @brief Register object constructor
        @param candle Candle object pointer
        */
        Register(Candle* candle) : candle(candle)
        {
        }
        /**
        @brief reads single-field registers
        @param canId ID of the drive
        @param regId first register's ID
        @param value first reference to a variable where the read value should be stored
        @param ...	remaining regId-value pairs to be read
        @return true if register was read
        */
        template <typename T2, typename... Ts>
        bool read(uint16_t canId, mdRegister_E regId, T2& regValue, const Ts&... vs)
        {
            /* prepare and send the request frame */
            if (!prepare(canId, mab::Md80FrameId_E::FRAME_READ_REGISTER, regId, regValue, vs...))
                return false;
            /* interpret the frame */
            return interpret(canId, regId, regValue, vs...);
        }

        /**
        @brief writes single-field registers
        @param canId ID of the drive
        @param regId first register's ID
        @param value first reference to a value that should be written
        @param ...	remaining regId-value pairs to be written
        @return true if register was written
        */
        template <typename T2, typename... Ts>
        bool write(uint16_t canId, mdRegister_E regId, const T2& regValue, const Ts&... vs)
        {
            return prepare(canId, mab::Md80FrameId_E::FRAME_WRITE_REGISTER, regId, regValue, vs...);
        }

        /**
        @brief returns the size of a register based on it's id
        @param regId register's ID
        @return register size in bytes
        */
        static uint16_t getSize(uint16_t regId);

        /**
        @brief returns the type of a register field based on it's id
        @param regId register's ID
        @return register type (Register::type enum class)
        */
        static type getType(uint16_t regId);

      private:
        Candle* candle;

        static const uint32_t maxCanFramelen = 64;
        char                  regTxBuffer[maxCanFramelen];
        char                  regRxBuffer[maxCanFramelen];
        char*                 regTxPtr = nullptr;
        char*                 regRxPtr = nullptr;

        uint32_t pack(uint16_t regId, char* value, char* buffer);
        uint32_t unPack(uint16_t regId, char* value, char* buffer);
        uint32_t copy(char* dest, char* source, uint32_t size, uint32_t freeSpace);
        bool     prepareFrame(mab::Md80FrameId_E frameId, mdRegister_E regId, char* value);
        bool     interpret(uint16_t canId);
        bool     prepare(uint16_t canId, mab::Md80FrameId_E frameType);

        template <typename T2, typename... Ts>
        bool interpret(uint16_t canId, mdRegister_E regId, const T2& regValue, const Ts&... vs)
        {
            /* if new frame */
            if (regRxPtr == nullptr)
                regRxPtr = &regRxBuffer[2];

            uint32_t offset = unPack(regId, (char*)&regValue, regRxPtr);
            if (offset == 0)
                return false;

            regRxPtr += offset;
            return interpret(canId, vs...);
        }

        template <typename T2, typename... Ts>
        bool prepare(uint16_t           canId,
                     mab::Md80FrameId_E frameType,
                     mdRegister_E          regId,
                     const T2&          regValue,
                     const Ts&... vs)
        {
            static_assert(!std::is_same<double, T2>::value,
                          "register value should be float not double");
            if (!prepareFrame(frameType, regId, (char*)&regValue))
                return false;
            return prepare(canId, frameType, vs...);
        }
    };

}  // namespace mab
