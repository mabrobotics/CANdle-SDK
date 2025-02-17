#pragma once

#include <cstddef>

#include "mab_types.hpp"
#include "manufacturer_data.hpp"

namespace mab
{

    enum class MdFrameId_E : uint8_t
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

    constexpr auto RO = RegisterAccessLevel_E::RO;
    constexpr auto RW = RegisterAccessLevel_E::RW;
    constexpr auto WO = RegisterAccessLevel_E::WO;

    template <typename T, RegisterAccessLevel_E L, u16 address>
    struct RegisterEntry_S
    {
        T value;

        const RegisterAccessLevel_E accessLevel = L;
        const u16                   regAddress  = address;

        RegisterEntry_S& operator=(T otherValue)
        {
            value = otherValue;

            return *this;
        }

        constexpr size_t getSize()
        {
            return sizeof(T);
        }
    };

    struct MDRegisters_S
    {
        template <typename T, RegisterAccessLevel_E L, u16 address>
        using regE_S = RegisterEntry_S<T, L, address>;

        regE_S<u32, RW, 0x001> canID;
        regE_S<u32, RW, 0x002> canBaudrate;
        regE_S<u16, RW, 0x003> canWatchdog;
        regE_S<u8, RW, 0x004>  canTermination;

        regE_S<char[24], RW, 0x010> motorName;
        regE_S<u32, RW, 0x011>      motorPolePairs;
        regE_S<float, RW, 0x012>    motorKt;
        regE_S<float, RW, 0x013>    motorKtPhaseA;
        regE_S<float, RW, 0x014>    motorKtPhaseB;
        regE_S<float, RW, 0x015>    motorKtPhaseC;
        regE_S<float, RW, 0x016>    motorIMax;
        regE_S<float, RW, 0x017>    motorGearRatio;
        regE_S<u16, RW, 0x018>      motorTorqueBandwidth;
        regE_S<float, RW, 0x019>    motorFriction;

        regE_S<float, RW, 0x01A> motorStiction;
        regE_S<float, RO, 0x01B> motorResistance;
        regE_S<float, RO, 0x01C> motorInductance;
        regE_S<u16, RW, 0x01D>   motorKV;
        regE_S<u8, RW, 0x01E>    motorCalibrationMode;
        regE_S<u8, RW, 0x01F>    motorThermistorType;

        regE_S<u8, RW, 0x020>    outputEncoder;
        regE_S<float, RW, 0x021> outputEncoderDir;
        regE_S<u32, RW, 0x022>   outputEncoderDefaultBaud;
        regE_S<float, RO, 0x023> outputEncoderVelocity;
        regE_S<float, RO, 0x024> outputEncoderPosition;
        regE_S<u8, RW, 0x025>    outputEncoderMode;
        regE_S<u8, RW, 0x026>    outputEncoderCalibrationMode;

        regE_S<float, RW, 0x030> motorPosPidKp;
        regE_S<float, RW, 0x031> motorPosPidKi;
        regE_S<float, RW, 0x032> motorPosPidKd;
        regE_S<float, RW, 0x034> motorPosPidWindup;

        regE_S<float, RW, 0x040> motorVelPidKp;
        regE_S<float, RW, 0x041> motorVelPidKi;
        regE_S<float, RW, 0x042> motorVelPidKd;
        regE_S<float, RW, 0x044> motorVelPidWindup;

        regE_S<float, RW, 0x050> motorImpPidKp;
        regE_S<float, RW, 0x051> motorImpPidKd;

        regE_S<float, RO, 0x062> mainEncoderVelocity;
        regE_S<float, RO, 0x063> mainEncoderPosition;
        regE_S<float, RO, 0x064> motorTorque;

        regE_S<u8, RW, 0x070>    homingMode;
        regE_S<float, RW, 0x071> homingMaxTravel;
        regE_S<float, RW, 0x072> homingVelocity;
        regE_S<float, RW, 0x073> homingTorque;

        regE_S<u8, WO, 0x080> runSaveCmd;
        regE_S<u8, WO, 0x081> runTestMainEncoderCmd;
        regE_S<u8, WO, 0x082> runTestOutputEncoderCmd;
        regE_S<u8, WO, 0x083> runCalibrateCmd;
        regE_S<u8, WO, 0x084> runCalibrateOutputEncoderCmd;
        regE_S<u8, WO, 0x085> runCalibratePiGains;
        regE_S<u8, WO, 0x086> runHoming;
        regE_S<u8, WO, 0x087> runRestoreFactoryConfig;
        regE_S<u8, WO, 0x088> runReset;
        regE_S<u8, WO, 0x089> runClearWarnings;
        regE_S<u8, WO, 0x08A> runClearErrors;
        regE_S<u8, WO, 0x08B> runBlink;
        regE_S<u8, WO, 0x08C> runZero;
        regE_S<u8, WO, 0x08D> runCanReinit;

        regE_S<float, RO, 0x100> calOutputEncoderStdDev;
        regE_S<float, RO, 0x101> calOutputEncoderMinE;
        regE_S<float, RO, 0x102> calOutputEncoderMaxE;
        regE_S<float, RO, 0x103> calMainEncoderStdDev;
        regE_S<float, RO, 0x104> calMainEncoderMinE;
        regE_S<float, RO, 0x105> calMainEncoderMaxE;

        regE_S<float, RW, 0x110> positionLimitMax;
        regE_S<float, RW, 0x111> positionLimitMin;
        regE_S<float, RW, 0x112> maxTorque;
        regE_S<float, RW, 0x113> maxVelocity;
        regE_S<float, RW, 0x114> maxAcceleration;
        regE_S<float, RW, 0x115> maxDeceleration;

        regE_S<float, RW, 0x120> profileVelocity;
        regE_S<float, RW, 0x121> profileAcceleration;
        regE_S<float, RW, 0x122> profileDeceleration;
        regE_S<float, RW, 0x123> quickStopDeceleration;
        regE_S<float, RW, 0x124> positionWindow;
        regE_S<float, RW, 0x125> velocityWindow;

        regE_S<u8, WO, 0x140>  motionModeCommand;
        regE_S<u8, RO, 0x141>  motionModeStatus;
        regE_S<u16, RW, 0x142> state;

        regE_S<float, RW, 0x150> targetPosition;
        regE_S<float, RW, 0x151> targetVelocity;
        regE_S<float, RW, 0x152> targetTorque;

        regE_S<u8, RW, 0x160>  userGpioConfiguration;
        regE_S<u16, RO, 0x161> userGpioState;

        regE_S<u8, RW, 0x600> reverseDirection;

        regE_S<float, RO, 0x700> shuntResistance;

        regE_S<hardwareType_S, RO, 0x7FF> hardwareType;
        regE_S<u32, RO, 0x800>            buildDate;
        regE_S<char[8], RO, 0x801>        commitHash;
        regE_S<u32, RO, 0x802>            firmwareVersion;
        regE_S<u8, RO, 0x803>             legacyHardwareVersion;
        regE_S<u8, RO, 0x804>             bridgeType;
        regE_S<u16, RO, 0x805>            quickStatus;
        regE_S<float, RO, 0x806>          mosfetTemperature;
        regE_S<float, RO, 0x807>          motorTemperature;
        regE_S<float, RO, 0x808>          motorShutdownTemp;
        regE_S<u32, RO, 0x809>            mainEncoderErrors;
        regE_S<u32, RO, 0x80A>            outputEncoderErrors;
        regE_S<u32, RO, 0x80B>            calibrationErrors;
        regE_S<u32, RO, 0x80C>            bridgeErrors;
        regE_S<u32, RO, 0x80D>            hardwareErrors;
        regE_S<u32, RO, 0x80E>            communicationErrors;
        regE_S<u32, RO, 0x80F>            homingErrors;
        regE_S<u32, RO, 0x810>            motionErrors;

        regE_S<float, RO, 0x811> dcBusVoltage;
        regE_S<u8, RO, 0x812>    bootloaderFixed;
    };

}  // namespace mab