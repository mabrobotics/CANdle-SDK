#pragma once

#include <array>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <tuple>
#include <vector>

#include "mab_types.hpp"
#include "manufacturer_data.hpp"

// DEFINE ALL OF THE MD REGISTERS HERE
#define REGISTER_LIST                                \
    MD_REG(nullreg, u8, 0x000, RO)                   \
    MD_REG(canID, u32, 0x001, RW)                    \
    MD_REG(canBaudrate, u32, 0x002, RW)              \
    MD_REG(canWatchdog, u16, 0x003, RW)              \
    MD_REG(canTermination, u8, 0x004, RW)            \
                                                     \
    MD_REG(motorName, char[24], 0x010, RW)           \
    MD_REG(motorPolePairs, u32, 0x011, RW)           \
    MD_REG(motorKt, float, 0x012, RW)                \
    MD_REG(motorKtPhaseA, float, 0x013, RW)          \
    MD_REG(motorKtPhaseB, float, 0x014, RW)          \
    MD_REG(motorKtPhaseC, float, 0x015, RW)          \
    MD_REG(motorIMax, float, 0x016, RW)              \
    MD_REG(motorGearRatio, float, 0x017, RW)         \
    MD_REG(motorTorqueBandwidth, u16, 0x018, RW)     \
    MD_REG(motorFriction, float, 0x019, RW)          \
    MD_REG(motorStiction, float, 0x01A, RW)          \
    MD_REG(motorResistance, float, 0x01B, RO)        \
    MD_REG(motorInductance, float, 0x01C, RO)        \
    MD_REG(motorKV, u16, 0x01D, RW)                  \
    MD_REG(motorCalibrationMode, u8, 0x01E, RW)      \
    MD_REG(motorThermistorType, u8, 0x01F, RW)       \
                                                     \
    MD_REG(auxEncoder, u8, 0x020, RW)                \
    MD_REG(auxEncoderDir, float, 0x021, RW)          \
    MD_REG(auxEncoderDefaultBaud, u32, 0x022, RW)    \
    MD_REG(auxEncoderVelocity, float, 0x023, RO)     \
    MD_REG(auxEncoderPosition, float, 0x024, RO)     \
    MD_REG(auxEncoderMode, u8, 0x025, RW)            \
    MD_REG(auxEncoderCalibrationMode, u8, 0x026, RW) \
                                                     \
    MD_REG(motorPosPidKp, float, 0x030, RW)          \
    MD_REG(motorPosPidKi, float, 0x031, RW)          \
    MD_REG(motorPosPidKd, float, 0x032, RW)          \
    MD_REG(motorPosPidWindup, float, 0x034, RW)      \
                                                     \
    MD_REG(motorVelPidKp, float, 0x040, RW)          \
    MD_REG(motorVelPidKi, float, 0x041, RW)          \
    MD_REG(motorVelPidKd, float, 0x042, RW)          \
    MD_REG(motorVelPidWindup, float, 0x044, RW)      \
                                                     \
    MD_REG(motorImpPidKp, float, 0x050, RW)          \
    MD_REG(motorImpPidKd, float, 0x051, RW)          \
                                                     \
    MD_REG(mainEncoderVelocity, float, 0x062, RO)    \
    MD_REG(mainEncoderPosition, float, 0x063, RO)    \
    MD_REG(motorTorque, float, 0x064, RO)            \
                                                     \
    MD_REG(homingMode, u8, 0x070, RW)                \
    MD_REG(homingMaxTravel, float, 0x071, RW)        \
    MD_REG(homingVelocity, float, 0x072, RW)         \
    MD_REG(homingTorque, float, 0x073, RW)           \
                                                     \
    MD_REG(runSaveCmd, u8, 0x080, WO)                \
    MD_REG(runTestMainEncoderCmd, u8, 0x081, WO)     \
    MD_REG(runTestAuxEncoderCmd, u8, 0x082, WO)      \
    MD_REG(runCalibrateCmd, u8, 0x083, WO)           \
    MD_REG(runCalibrateAuxEncoderCmd, u8, 0x084, WO) \
    MD_REG(runCalibratePiGains, u8, 0x085, WO)       \
    MD_REG(runHoming, u8, 0x086, WO)                 \
    MD_REG(runRestoreFactoryConfig, u8, 0x087, WO)   \
    MD_REG(runReset, u8, 0x088, WO)                  \
    MD_REG(runClearWarnings, u8, 0x089, WO)          \
    MD_REG(runClearErrors, u8, 0x08A, WO)            \
    MD_REG(runBlink, u8, 0x08B, WO)                  \
    MD_REG(runZero, u8, 0x08C, WO)                   \
    MD_REG(runCanReinit, u8, 0x08D, WO)              \
                                                     \
    MD_REG(calAuxEncoderStdDev, float, 0x100, RO)    \
    MD_REG(calAuxEncoderMinE, float, 0x101, RO)      \
    MD_REG(calAuxEncoderMaxE, float, 0x102, RO)      \
    MD_REG(calMainEncoderStdDev, float, 0x103, RO)   \
    MD_REG(calMainEncoderMinE, float, 0x104, RO)     \
    MD_REG(calMainEncoderMaxE, float, 0x105, RO)     \
                                                     \
    MD_REG(positionLimitMax, float, 0x110, RW)       \
    MD_REG(positionLimitMin, float, 0x111, RW)       \
    MD_REG(maxTorque, float, 0x112, RW)              \
    MD_REG(maxVelocity, float, 0x113, RW)            \
    MD_REG(maxAcceleration, float, 0x114, RW)        \
    MD_REG(maxDeceleration, float, 0x115, RW)        \
                                                     \
    MD_REG(profileVelocity, f32, 0x120, RW)          \
    MD_REG(profileAcceleration, f32, 0x121, RW)      \
    MD_REG(profileDeceleration, f32, 0x122, RW)      \
    MD_REG(quickStopDeceleration, f32, 0x123, RW)    \
    MD_REG(positionWindow, f32, 0x124, RW)           \
    MD_REG(velocityWindow, f32, 0x125, RW)           \
                                                     \
    MD_REG(motionModeCommand, u8, 0x140, WO)         \
    MD_REG(motionModeStatus, u8, 0x141, RO)          \
    MD_REG(state, u16, 0x142, RW)                    \
                                                     \
    MD_REG(targetPosition, float, 0x150, RW)         \
    MD_REG(targetVelocity, float, 0x151, RW)         \
    MD_REG(targetTorque, float, 0x152, RW)           \
                                                     \
    MD_REG(userGpioConfiguration, u8, 0x160, RW)     \
    MD_REG(userGpioState, u16, 0x161, RO)            \
                                                     \
    MD_REG(reverseDirection, u8, 0x600, RW)          \
                                                     \
    MD_REG(shuntResistance, float, 0x700, RO)        \
                                                     \
    MD_REG(uniqueID, char[12], 0x7FE, RO)            \
    MD_REG(hardwareType, hardwareType_S, 0x7FF, RO)  \
    MD_REG(buildDate, u32, 0x800, RO)                \
    MD_REG(commitHash, char[8], 0x801, RO)           \
    MD_REG(firmwareVersion, u32, 0x802, RO)          \
    MD_REG(legacyHardwareVersion, u8, 0x803, RO)     \
    MD_REG(bridgeType, u8, 0x804, RO)                \
    MD_REG(quickStatus, u16, 0x805, RO)              \
    MD_REG(mosfetTemperature, f32, 0x806, RO)        \
    MD_REG(motorTemperature, f32, 0x807, RO)         \
    MD_REG(motorShutdownTemp, u8, 0x808, RW)         \
    MD_REG(mainEncoderStatus, u32, 0x809, RO)        \
    MD_REG(auxEncoderStatus, u32, 0x80A, RO)         \
    MD_REG(calibrationStatus, u32, 0x80B, RO)        \
    MD_REG(bridgeStatus, u32, 0x80C, RO)             \
    MD_REG(hardwareStatus, u32, 0x80D, RO)           \
    MD_REG(communicationStatus, u32, 0x80E, RO)      \
    MD_REG(homingStatus, u32, 0x80F, RO)             \
    MD_REG(motionStatus, u32, 0x810, RO)             \
    MD_REG(dcBusVoltage, f32, 0x811, RO)             \
    MD_REG(bootloaderFixed, u8, 0x812, RO)           \
    MD_REG(miscStatus, u32, 0x813, RO)

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

    enum class MDRegisterAddress_E : u16
    {
#define MD_REG(name, type, addr, access) name = addr,
        REGISTER_LIST
#undef MD_REG
    };

    template <typename T>
    struct MDRegisterEntry_S
    {
      public:
        T value;

        const RegisterAccessLevel_E m_accessLevel;
        const u16                   m_regAddress;
        const std::string_view      m_name;

      private:
        std::array<u8, sizeof(value) + sizeof(m_regAddress)> serializedBuffer;

      public:
        constexpr MDRegisterEntry_S(RegisterAccessLevel_E accessLevel,
                                    u16                   regAddress,
                                    std::string_view      name)
            : m_accessLevel(accessLevel), m_regAddress(regAddress), m_name(name)
        {
        }

        constexpr MDRegisterEntry_S(const MDRegisterEntry_S& otherReg)
            : m_accessLevel(otherReg.m_accessLevel),
              m_regAddress(otherReg.m_regAddress),
              m_name(otherReg.m_name)
        {
            value = otherReg.value;
        }

        MDRegisterEntry_S& operator=(T otherValue)
        {
            value = otherValue;

            return *this;
        }

        T operator=(MDRegisterEntry_S& reg)
        {
            return reg.value;
        }

        constexpr size_t getSize()
        {
            return sizeof(T);
        }

        const std::array<u8, sizeof(value) + sizeof(m_regAddress)>* getSerializedRegister()
        {
            // Frame layout <8bits per chunk> [LSB address, MSB address, Payload ...]
            std::memcpy(serializedBuffer.data(), &m_regAddress, sizeof(m_regAddress));
            std::memcpy(serializedBuffer.data() + sizeof(m_regAddress), &value, sizeof(value));

            return &serializedBuffer;
        }

        bool setSerializedRegister(std::vector<u8>& data)
        {
            // Frame layout <8bits per chunk> [LSB address, MSB address, Payload ...]
            u16 addressFromSerial = 0;
            std::memcpy(&addressFromSerial, data.data(), sizeof(m_regAddress));
            if (addressFromSerial == m_regAddress)
            {
                std::memcpy(&value, data.data() + sizeof(m_regAddress), sizeof(value));
                if (data.size() > sizeof(m_regAddress) + sizeof(value))
                    data.erase(data.begin(), data.begin() + sizeof(m_regAddress) + sizeof(value));
                return true;
            }
            return false;
        }

        void clear()
        {
            memset(&value, 0, sizeof(value));
        }
    };
    template <typename T, size_t N>
    struct MDRegisterEntry_S<T[N]>
    {
        T value[N];

        const RegisterAccessLevel_E m_accessLevel;
        const u16                   m_regAddress;
        const std::string_view      m_name;

      private:
        std::array<u8, sizeof(value) + sizeof(m_regAddress)> serializedBuffer;

      public:
        constexpr MDRegisterEntry_S(RegisterAccessLevel_E accessLevel,
                                    u16                   regAddress,
                                    std::string_view      name)
            : m_accessLevel(accessLevel), m_regAddress(regAddress), m_name(name)
        {
        }
        constexpr MDRegisterEntry_S(const MDRegisterEntry_S& otherReg)
            : m_accessLevel(otherReg.m_accessLevel),
              m_regAddress(otherReg.m_regAddress),
              m_name(otherReg.m_name)
        {
            static_assert(std::is_same_v<decltype(otherReg.value), decltype(value)>);
            std::memcpy(&value, &otherReg.value, sizeof(value));
        }

        // This is kinda unsafe due to c-array not passing size, use with caution
        MDRegisterEntry_S& operator=(T* otherValue)
        {
            memcpy(value, otherValue, N);

            return *this;
        }

        T* operator=(MDRegisterEntry_S& reg) const
        {
            return value;
        }

        constexpr size_t getSize() const
        {
            return sizeof(T[N]);
        }

        const std::array<u8, sizeof(value) + sizeof(m_regAddress)>* getSerializedRegister()
        {
            // Frame layout <8bits per chunk> [LSB address, MSB address, Payload ...]
            std::memcpy(serializedBuffer.data(), &m_regAddress, sizeof(m_regAddress));
            std::memcpy(serializedBuffer.data() + sizeof(m_regAddress), value, sizeof(value));

            return &serializedBuffer;
        }

        bool setSerializedRegister(std::vector<u8>& data)
        {
            // Frame layout <8bits per chunk> [LSB address, MSB address, Payload ...]
            u16 addressFromSerial = 0;
            std::memcpy(&addressFromSerial, data.data(), sizeof(m_regAddress));
            if (addressFromSerial == m_regAddress)
            {
                std::memcpy(value, data.data() + sizeof(m_regAddress), sizeof(value));
                if (data.size() > sizeof(m_regAddress) + sizeof(value))
                    data.erase(data.begin(), data.begin() + sizeof(m_regAddress) + sizeof(value));
                return true;
            }
            return false;
        }
        void clear()
        {
            memset(&value, 0, sizeof(value));
        }
    };

    struct MDRegisters_S
    {
        RegisterAccessLevel_E const RO = RegisterAccessLevel_E::RO;
        RegisterAccessLevel_E const RW = RegisterAccessLevel_E::RW;
        RegisterAccessLevel_E const WO = RegisterAccessLevel_E::WO;
        template <class T>
        using regE_S = MDRegisterEntry_S<T>;

#define MD_REG(name, type, addr, access) regE_S<type> name = regE_S<type>(access, addr, #name);
        REGISTER_LIST
#undef MD_REG
        constexpr auto getAllRegisters()
        {
            return std::tie(
#define MD_REG(name, type, addr, access) , name
// Just a macro to remove empty argument, no fancy here
#define REMOVE_FIRST(X, ...)         __VA_ARGS__
#define EXPAND_AND_REMOVE_FIRST(...) REMOVE_FIRST(__VA_ARGS__)
                EXPAND_AND_REMOVE_FIRST(REGISTER_LIST)
#undef EXPAND_AND_REMOVE_FIRST
#undef REMOVE_FIRST
#undef MD_REG
            );
        }

        template <class F>
        void forEachRegister(F&& func)
        {
            std::apply([&](auto&&... regs) { (func(regs), ...); }, getAllRegisters());
        }

        template <class F>
        constexpr void compileTimeForEachRegister(F&& func)
        {
            std::apply([&](auto&&... regs) { (func(regs), ...); }, getAllRegisters());
        }
    };

}  // namespace mab
