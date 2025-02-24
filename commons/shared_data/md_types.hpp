#pragma once

#include <array>
#include <cstddef>
#include <cstring>
#include <vector>

#include "mab_types.hpp"
#include "manufacturer_data.hpp"

namespace mab
{

    enum class MdFrameId_E : u8
    {
        FRAME_WRITE_REGISTER = 0x40,
        FRAME_READ_REGISTER  = 0x41,
        RESPONSE_DEFAULT     = 0xA0
    };

    enum class RegisterAccessLevel_E : u8
    {
        RO = 0,
        RW = (1 << 1),
        WO = (1 << 2)
    };

    template <typename T>
    struct RegisterEntry_S
    {
      public:
        T value;

        const RegisterAccessLevel_E m_accessLevel;
        const u16                   m_regAddress;

      private:
        std::array<u8, sizeof(value) + sizeof(m_regAddress)> serializedBuffer;

      public:
        RegisterEntry_S(RegisterAccessLevel_E accessLevel, u16 regAddress)
            : m_accessLevel(accessLevel), m_regAddress(regAddress)
        {
        }

        RegisterEntry_S& operator=(T otherValue)
        {
            value = otherValue;

            return *this;
        }

        T operator=(RegisterEntry_S& reg)
        {
            return value;
        }

        constexpr size_t getSize()
        {
            return sizeof(T);
        }

        const std::array<u8, sizeof(value) + sizeof(m_regAddress)>* getSerializedRegister()
        {
            std::memcpy(serializedBuffer.data(), &m_regAddress, sizeof(m_regAddress));
            std::memcpy(serializedBuffer.data() + sizeof(m_regAddress), &value, sizeof(value));

            return &serializedBuffer;
        }

        bool setSerializedRegister(std::vector<u8>& data)
        {
            u16 addressFromSerial = 0;
            std::memcpy(&addressFromSerial, data.data(), sizeof(m_regAddress));
            if (addressFromSerial == m_regAddress)
            {
                std::memcpy(&value, data.data() + sizeof(m_regAddress), sizeof(value));
                data.erase(data.begin(), data.begin() + sizeof(m_regAddress) + sizeof(value));
                return true;
            }
            return false;
        }
    };
    template <typename T, size_t N>
    struct RegisterEntry_S<T[N]>
    {
        T value[N];

        const RegisterAccessLevel_E m_accessLevel;
        const u16                   m_regAddress;

      private:
        std::array<u8, sizeof(value) + sizeof(m_regAddress)> serializedBuffer;

      public:
        RegisterEntry_S(RegisterAccessLevel_E accessLevel, u16 regAddress)
            : m_accessLevel(accessLevel), m_regAddress(regAddress)
        {
        }

        // This is kinda unsafe due to c-array not passing size, use with caution
        RegisterEntry_S& operator=(T* otherValue)
        {
            memcpy(value, otherValue, N);

            return *this;
        }

        T* operator=(RegisterEntry_S& reg)
        {
            return value;
        }

        constexpr size_t getSize() const
        {
            return sizeof(T[N]);
        }

        const std::array<u8, sizeof(value) + sizeof(m_regAddress)>* getSerializedRegister()
        {
            std::memcpy(serializedBuffer.data(), &m_regAddress, sizeof(m_regAddress));
            std::memcpy(serializedBuffer.data() + sizeof(m_regAddress), value, sizeof(value));

            return &serializedBuffer;
        }

        bool setSerializedRegister(std::vector<u8>& data)
        {
            u16 addressFromSerial = 0;
            std::memcpy(&addressFromSerial, data.data(), sizeof(m_regAddress));
            if (addressFromSerial == m_regAddress)
            {
                std::memcpy(value, data.data() + sizeof(m_regAddress), sizeof(value));
                data.erase(data.begin(), data.begin() + sizeof(m_regAddress) + sizeof(value));
                return true;
            }
            return false;
        }
    };

    struct MDRegisters_S
    {
        RegisterAccessLevel_E const RO = RegisterAccessLevel_E::RO;
        RegisterAccessLevel_E const RW = RegisterAccessLevel_E::RW;
        RegisterAccessLevel_E const WO = RegisterAccessLevel_E::WO;
        template <typename T>
        using regE_S = RegisterEntry_S<T>;

        regE_S<u32> canID          = regE_S<u32>(RW, 0x001);
        regE_S<u32> canBaudrate    = regE_S<u32>(RW, 0x002);
        regE_S<u16> canWatchdog    = regE_S<u16>(RW, 0x003);
        regE_S<u8>  canTermination = regE_S<u8>(RW, 0x004);

        regE_S<char[24]> motorName            = regE_S<char[24]>(RW, 0x010);
        regE_S<u32>      motorPolePairs       = regE_S<u32>(RW, 0x011);
        regE_S<float>    motorKt              = regE_S<float>(RW, 0x012);
        regE_S<float>    motorKtPhaseA        = regE_S<float>(RW, 0x013);
        regE_S<float>    motorKtPhaseB        = regE_S<float>(RW, 0x014);
        regE_S<float>    motorKtPhaseC        = regE_S<float>(RW, 0x015);
        regE_S<float>    motorIMax            = regE_S<float>(RW, 0x016);
        regE_S<float>    motorGearRatio       = regE_S<float>(RW, 0x017);
        regE_S<u16>      motorTorqueBandwidth = regE_S<u16>(RW, 0x018);
        regE_S<float>    motorFriction        = regE_S<float>(RW, 0x019);

        regE_S<float> motorStiction        = regE_S<float>(RW, 0x01A);
        regE_S<float> motorResistance      = regE_S<float>(RO, 0x01B);
        regE_S<float> motorInductance      = regE_S<float>(RO, 0x01C);
        regE_S<u16>   motorKV              = regE_S<u16>(RW, 0x01D);
        regE_S<u8>    motorCalibrationMode = regE_S<u8>(RW, 0x01E);
        regE_S<u8>    motorThermistorType  = regE_S<u8>(RW, 0x01F);

        regE_S<u8>    outputEncoder                = regE_S<u8>(RW, 0x020);
        regE_S<float> outputEncoderDir             = regE_S<float>(RW, 0x021);
        regE_S<u32>   outputEncoderDefaultBaud     = regE_S<u32>(RW, 0x022);
        regE_S<float> outputEncoderVelocity        = regE_S<float>(RO, 0x023);
        regE_S<float> outputEncoderPosition        = regE_S<float>(RO, 0x024);
        regE_S<u8>    outputEncoderMode            = regE_S<u8>(RW, 0x025);
        regE_S<u8>    outputEncoderCalibrationMode = regE_S<u8>(RW, 0x026);

        regE_S<float> motorPosPidKp     = regE_S<float>(RW, 0x030);
        regE_S<float> motorPosPidKi     = regE_S<float>(RW, 0x031);
        regE_S<float> motorPosPidKd     = regE_S<float>(RW, 0x032);
        regE_S<float> motorPosPidWindup = regE_S<float>(RW, 0x034);

        regE_S<float> motorVelPidKp     = regE_S<float>(RW, 0x040);
        regE_S<float> motorVelPidKi     = regE_S<float>(RW, 0x041);
        regE_S<float> motorVelPidKd     = regE_S<float>(RW, 0x042);
        regE_S<float> motorVelPidWindup = regE_S<float>(RW, 0x044);

        regE_S<float> motorImpPidKp = regE_S<float>(RW, 0x050);
        regE_S<float> motorImpPidKd = regE_S<float>(RW, 0x051);

        regE_S<float> mainEncoderVelocity = regE_S<float>(RO, 0x062);
        regE_S<float> mainEncoderPosition = regE_S<float>(RO, 0x063);
        regE_S<float> motorTorque         = regE_S<float>(RO, 0x064);

        regE_S<u8>    homingMode      = regE_S<u8>(RW, 0x070);
        regE_S<float> homingMaxTravel = regE_S<float>(RW, 0x071);
        regE_S<float> homingVelocity  = regE_S<float>(RW, 0x072);
        regE_S<float> homingTorque    = regE_S<float>(RW, 0x073);

        regE_S<u8> runSaveCmd                   = regE_S<u8>(WO, 0x080);
        regE_S<u8> runTestMainEncoderCmd        = regE_S<u8>(WO, 0x081);
        regE_S<u8> runTestOutputEncoderCmd      = regE_S<u8>(WO, 0x082);
        regE_S<u8> runCalibrateCmd              = regE_S<u8>(WO, 0x083);
        regE_S<u8> runCalibrateOutputEncoderCmd = regE_S<u8>(WO, 0x084);
        regE_S<u8> runCalibratePiGains          = regE_S<u8>(WO, 0x085);
        regE_S<u8> runHoming                    = regE_S<u8>(WO, 0x086);
        regE_S<u8> runRestoreFactoryConfig      = regE_S<u8>(WO, 0x087);
        regE_S<u8> runReset                     = regE_S<u8>(WO, 0x088);
        regE_S<u8> runClearWarnings             = regE_S<u8>(WO, 0x089);
        regE_S<u8> runClearErrors               = regE_S<u8>(WO, 0x08A);
        regE_S<u8> runBlink                     = regE_S<u8>(WO, 0x08B);
        regE_S<u8> runZero                      = regE_S<u8>(WO, 0x08C);
        regE_S<u8> runCanReinit                 = regE_S<u8>(WO, 0x08D);

        regE_S<float> calOutputEncoderStdDev = regE_S<float>(RO, 0x100);
        regE_S<float> calOutputEncoderMinE   = regE_S<float>(RO, 0x101);
        regE_S<float> calOutputEncoderMaxE   = regE_S<float>(RO, 0x102);
        regE_S<float> calMainEncoderStdDev   = regE_S<float>(RO, 0x103);
        regE_S<float> calMainEncoderMinE     = regE_S<float>(RO, 0x104);
        regE_S<float> calMainEncoderMaxE     = regE_S<float>(RO, 0x105);

        regE_S<float> positionLimitMax = regE_S<float>(RW, 0x110);
        regE_S<float> positionLimitMin = regE_S<float>(RW, 0x111);
        regE_S<float> maxTorque        = regE_S<float>(RW, 0x112);
        regE_S<float> maxVelocity      = regE_S<float>(RW, 0x113);
        regE_S<float> maxAcceleration  = regE_S<float>(RW, 0x114);
        regE_S<float> maxDeceleration  = regE_S<float>(RW, 0x115);

        regE_S<float> profileVelocity       = regE_S<float>(RW, 0x120);
        regE_S<float> profileAcceleration   = regE_S<float>(RW, 0x121);
        regE_S<float> profileDeceleration   = regE_S<float>(RW, 0x122);
        regE_S<float> quickStopDeceleration = regE_S<float>(RW, 0x123);
        regE_S<float> positionWindow        = regE_S<float>(RW, 0x124);
        regE_S<float> velocityWindow        = regE_S<float>(RW, 0x125);

        regE_S<u8>  motionModeCommand = regE_S<u8>(WO, 0x140);
        regE_S<u8>  motionModeStatus  = regE_S<u8>(RO, 0x141);
        regE_S<u16> state             = regE_S<u16>(RW, 0x142);

        regE_S<float> targetPosition = regE_S<float>(RW, 0x150);
        regE_S<float> targetVelocity = regE_S<float>(RW, 0x151);
        regE_S<float> targetTorque   = regE_S<float>(RW, 0x152);

        regE_S<u8>  userGpioConfiguration = regE_S<u8>(RW, 0x160);
        regE_S<u16> userGpioState         = regE_S<u16>(RO, 0x161);

        regE_S<u8> reverseDirection = regE_S<u8>(RW, 0x600);

        regE_S<float> shuntResistance = regE_S<float>(RO, 0x700);

        regE_S<hardwareType_S> hardwareType          = regE_S<hardwareType_S>(RO, 0x7FF);
        regE_S<u32>            buildDate             = regE_S<u32>(RO, 0x800);
        regE_S<char[8]>        commitHash            = regE_S<char[8]>(RO, 0x801);
        regE_S<u32>            firmwareVersion       = regE_S<u32>(RO, 0x802);
        regE_S<u8>             legacyHardwareVersion = regE_S<u8>(RO, 0x803);
        regE_S<u8>             bridgeType            = regE_S<u8>(RO, 0x804);
        regE_S<u16>            quickStatus           = regE_S<u16>(RO, 0x805);
        regE_S<float>          mosfetTemperature     = regE_S<float>(RO, 0x806);
        regE_S<float>          motorTemperature      = regE_S<float>(RO, 0x807);
        regE_S<float>          motorShutdownTemp     = regE_S<float>(RO, 0x808);
        regE_S<u32>            mainEncoderErrors     = regE_S<u32>(RO, 0x809);
        regE_S<u32>            outputEncoderErrors   = regE_S<u32>(RO, 0x80A);
        regE_S<u32>            calibrationErrors     = regE_S<u32>(RO, 0x80B);
        regE_S<u32>            bridgeErrors          = regE_S<u32>(RO, 0x80C);
        regE_S<u32>            hardwareErrors        = regE_S<u32>(RO, 0x80D);
        regE_S<u32>            communicationErrors   = regE_S<u32>(RO, 0x80E);
        regE_S<u32>            homingErrors          = regE_S<u32>(RO, 0x80F);
        regE_S<u32>            motionErrors          = regE_S<u32>(RO, 0x810);

        regE_S<float> dcBusVoltage    = regE_S<float>(RO, 0x811);
        regE_S<u8>    bootloaderFixed = regE_S<u8>(RO, 0x812);
    };

}  // namespace mab