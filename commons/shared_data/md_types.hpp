#pragma once

#include "mab_types.hpp"

namespace mab
{
    enum mdRegister_E : u16
    {
        null           = 0x0000,
        canId          = 0x001,
        canBaudrate    = 0x002,
        canWatchdog    = 0x003,
        canTermination = 0x004,

        motorName            = 0x010,
        motorPolePairs       = 0x011,
        motorKt              = 0x012,
        motorKt_a            = 0x013,
        motorKt_b            = 0x014,
        motorKt_c            = 0x015,
        motorMaxCurrent      = 0x016,
        motorGearRatio       = 0x017,
        motorTorqueBandwidth = 0x018,
        motorFriction        = 0x019,
        motorStriction       = 0x01A,
        motorResistance      = 0x01B,
        motorInductance      = 0x01C,
        motorKV              = 0x01D,
        motorCalibrationMode = 0x01E,
        motorThermistorType  = 0x01F,

        defaultResponse = 0x00A0,
        defaultCommand  = 0x00B0,

        outputEncoder                = 0x020,
        outputEncoderDir             = 0x021,
        outputEncoderDefaultBaud     = 0x022,
        outputEncoderVelocity        = 0x023,
        outputEncoderPosition        = 0x024,
        outputEncoderMode            = 0x025,
        outputEncoderCalibrationMode = 0x026,

        motorPosPidKp     = 0x030,
        motorPosPidKi     = 0x031,
        motorPosPidKd     = 0x032,
        motorPosPidOutMax = 0x033,
        motorPosPidWindup = 0x034,

        motorVelPidKp     = 0x040,
        motorVelPidKi     = 0x041,
        motorVelPidKd     = 0x042,
        motorVelPidOutMax = 0x043,
        motorVelPidWindup = 0x044,

        motorImpPidKp     = 0x050,
        motorImpPidKd     = 0x051,
        motorImpPidOutMax = 0x052,

        mainEncoderVelocity = 0x062,
        mainEncoderPosition = 0x063,
        motorTorque         = 0x064,

        homingMode      = 0x070,
        homingMaxTravel = 0x071,
        homingVelocity  = 0x072,
        homingTorque    = 0x073,

        runSaveCmd                = 0x080,
        runTestMainEncoderCmd     = 0x081,
        runtestAuxEncoderCmd      = 0x082,
        runCalibrateCmd           = 0x083,
        runCalibrateAuxEncoderCmd = 0x084,
        runCalibratePiGains       = 0x085,
        runHoming                 = 0x086,
        runRestoreFactoryConfig   = 0x087,
        runReset                  = 0x088,
        runClearWarnings          = 0x089,
        runClearErrors            = 0x08A,
        runBlink                  = 0x08B,
        runZero                   = 0x08C,
        runCanReinit              = 0x08D,

        calOutputEncoderStdDev = 0x100,
        calOutputEncoderMinE   = 0x101,
        calOutputEncoderMaxE   = 0x102,
        calMainEncoderStdDev   = 0x103,
        calMainEncoderMinE     = 0x104,
        calMainEncoderMaxE     = 0x105,

        positionLimitMax = 0x110,
        positionLimitMin = 0x111,
        maxTorque        = 0x112,
        maxVelocity      = 0x113,
        maxAcceleration  = 0x114,
        maxDeceleration  = 0x115,

        profileVelocity       = 0x120,
        profileAcceleration   = 0x121,
        profileDeceleration   = 0x122,
        quickStopDeceleration = 0x123,
        positionWindow        = 0x124,
        velocityWindow        = 0x125,

        motionModeCommand = 0x140,
        motionModeStatus  = 0x141,
        state             = 0x142,

        targetPosition = 0x150,
        targetVelocity = 0x151,
        targetTorque   = 0x152,

        // brakeMode = 0x160, <- alias for userGpioConfiguration, legacy support reasons

        userGpioConfiguration = 0x160,
        userGpioState         = 0x161,

        reverseDirection = 0x600,

        shuntResistance = 0x700,

        hardwareType          = 0x7FF,
        buildDate             = 0x800,
        commitHash            = 0x801,
        firmwareVersion       = 0x802,
        legacyHardwareVersion = 0x803,
        bridgeType            = 0x804,
        quickStatus           = 0x805,
        mosfetTemperature     = 0x806,
        motorTemperature      = 0x807,
        motorShutdownTemp     = 0x808,
        mainEncoderStatus     = 0x809,
        auxEncoderStatus      = 0x80A,
        calibrationStatus     = 0x80B,
        bridgeStatus          = 0x80C,
        hardwareStatus        = 0x80D,
        communicationStatus   = 0x80E,
        homingStatus          = 0x80F,
        motionStatus          = 0x810,
        dcBusVoltageReg       = 0x811,

        miscStatus = 0x812,

        bootloaderFixed = 0x820,

        uniqueID = 0x830

    };
}
