#pragma once

#include "candle/shared_data/mab_types.hpp"

#include <array>
#include <cctype>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <tuple>
#include <unordered_map>
#include <vector>

// DEFINE ALL OF THE MD REGISTERS HERE
#define REGISTER_LIST                                \
    MD_REG(nullreg, u8, 0x000, RO)                   \
    MD_REG(canID, u32, 0x001, RW)                    \
    MD_REG(canBaudrate, u32, 0x002, RW)              \
    MD_REG(canWatchdog, u16, 0x003, RW)              \
                                                     \
    MD_REG(motorName, char[24], 0x010, RW)           \
    MD_REG(motorPolePairs, u32, 0x011, RW)           \
    MD_REG(motorKt, float, 0x012, RW)                \
    MD_REG(motorIMax, float, 0x016, RW)              \
    MD_REG(motorGearRatio, float, 0x017, RW)         \
    MD_REG(motorTorqueBandwidth, u16, 0x018, RW)     \
    MD_REG(motorResistance, float, 0x01B, RO)        \
    MD_REG(motorInductance, float, 0x01C, RO)        \
    MD_REG(motorKV, u16, 0x01D, RW)                  \
    MD_REG(motorCalibrationMode, u8, 0x01E, RW)      \
    MD_REG(motorThermistorType, u8, 0x01F, RW)       \
                                                     \
    MD_REG(auxEncoder, u8, 0x020, RW)                \
    MD_REG(auxEncoderDir, float, 0x021, RW)          \
    MD_REG(auxEncoderVelocity, float, 0x023, RO)     \
    MD_REG(auxEncoderPosition, float, 0x024, RO)     \
    MD_REG(auxEncoderMode, u8, 0x025, RW)            \
    MD_REG(auxEncoderCalibrationMode, u8, 0x026, RW) \
                                                     \
    MD_REG(mainEncoder, u8, 0x002A, RW)              \
    MD_REG(mainEncoderDir, float, 0x002B, RW)        \
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
    MD_REG(velocity, float, 0x062, RO)               \
    MD_REG(position, float, 0x063, RO)               \
    MD_REG(torque, float, 0x064, RO)                 \
    MD_REG(mainEncoderVelocity, float, 0x062, RO)    \
    MD_REG(mainEncoderPosition, float, 0x063, RO)    \
    MD_REG(motorTorque, float, 0x064, RO)            \
                                                     \
    MD_REG(runSaveCmd, u8, 0x080, WO)                \
    MD_REG(runTestMainEncoderCmd, u8, 0x081, WO)     \
    MD_REG(runTestAuxEncoderCmd, u8, 0x082, WO)      \
    MD_REG(runCalibrateCmd, u8, 0x083, WO)           \
    MD_REG(runCalibrateAuxEncoderCmd, u8, 0x084, WO) \
    MD_REG(runCalibratePiGains, u8, 0x085, WO)       \
    MD_REG(runRestoreFactoryConfig, u8, 0x087, WO)   \
    MD_REG(runReset, u8, 0x088, WO)                  \
    MD_REG(runClearWarnings, u8, 0x089, WO)          \
    MD_REG(runClearErrors, u8, 0x08A, WO)            \
    MD_REG(runBlink, u8, 0x08B, WO)                  \
    MD_REG(runZero, u8, 0x08C, WO)                   \
    MD_REG(runCanReinit, u8, 0x08D, WO)              \
    MD_REG(runTorqueSensorZero, u8, 0x008E, WO)      \
                                                     \
    MD_REG(calAuxEncoderStdDev, float, 0x100, RO)    \
    MD_REG(calAuxEncoderMinE, float, 0x101, RO)      \
    MD_REG(calAuxEncoderMaxE, float, 0x102, RO)      \
    MD_REG(calMainEncoderStdDev, float, 0x103, RO)   \
    MD_REG(calMainEncoderMinE, float, 0x104, RO)     \
    MD_REG(calMainEncoderMaxE, float, 0x105, RO)     \
                                                     \
    MD_REG(maxPosition, float, 0x110, RW)            \
    MD_REG(minPosition, float, 0x111, RW)            \
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
    MD_REG(torqueSensor, u8, 0x0200, RW)             \
    MD_REG(torqueSensorData, f32, 0x0201, RO)        \
                                                     \
    MD_REG(shuntResistance, float, 0x700, RW)        \
    MD_REG(maxDriverCurrent, float, 0x701, RO)       \
                                                     \
    MD_REG(productionDate, char[6], 0x7FB, RO)       \
    MD_REG(productionBatch, char[24], 0x7FC, RO)     \
    MD_REG(uniqueID, char[12], 0x7FD, RO)            \
    MD_REG(hardwareRev, u8, 0x7FE, RO)               \
    MD_REG(hardwareType, u8, 0x7FF, RO)              \
    MD_REG(firmwareBuildDate, u32, 0x800, RO)        \
    MD_REG(firmwareHash, char[8], 0x801, RO)         \
    MD_REG(buildDate, u32, 0x800, RO)                \
    MD_REG(commitHash, char[8], 0x801, RO)           \
    MD_REG(firmwareVersion, u32, 0x802, RO)          \
    MD_REG(legacyHardwareVersion, u8, 0x803, RO)     \
    MD_REG(quickStatus, u16, 0x805, RO)              \
    MD_REG(mosfetTemperature, f32, 0x806, RO)        \
    MD_REG(motorTemperature, f32, 0x807, RO)         \
    MD_REG(motorShutdownTemp, u8, 0x808, RW)         \
                                                     \
    MD_REG(mainEncoderStatus, u32, 0x809, RO)        \
    MD_REG(auxEncoderStatus, u32, 0x80A, RO)         \
    MD_REG(calibrationStatus, u32, 0x80B, RO)        \
    MD_REG(bridgeStatus, u32, 0x80C, RO)             \
    MD_REG(hardwareStatus, u32, 0x80D, RO)           \
    MD_REG(communicationStatus, u32, 0x80E, RO)      \
    MD_REG(motionStatus, u32, 0x810, RO)             \
    MD_REG(dcBusVoltage, f32, 0x811, RO)             \
    MD_REG(miscStatus, u32, 0x813, RO)               \
    MD_REG(configStatus, u32, 0x814, RO)

namespace mab
{
    constexpr u32 MDCanIdMin = 10;
    constexpr u32 MDCanIdMax = 2000;

    enum class MdFrameId_E : u8
    {
        RESTART_LEGACY         = 0x13,
        WRITE_REGISTER_LEGACY  = 0x40,
        READ_REGISTER          = 0x41,
        WRITE_REGISTER         = 0x42,
        READ_REGISTER_CAN_2_0  = 0x43,
        WRITE_REGISTER_CAN_2_0 = 0x44,
        RESPONSE_LEGACY        = 0xA0,
        RESPONSE_ERROR         = 0xA1
    };

    enum MdRegisterAccessErrorCode : i8
    {
        NONE         = 0,
        DEPRECATED   = -1,
        INVALID      = -2,
        UNKNOWN      = -3,
        OUT_OF_RANGE = -4,
        ACCESS       = -5,
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
        REGISTER_LIST REGISTER_LIST_DEV
#undef MD_REG
    };

    template <typename T>
    struct MDRegisterEntry_S
    {
      public:
        T value{};

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

        constexpr size_t getSize() const
        {
            return sizeof(T);
        }

        constexpr size_t getSerializedSize() const
        {
            return sizeof(T) + sizeof(m_regAddress);
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
            if (data.size() < getSerializedSize() || data.data() == nullptr)
                return false;
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
            if constexpr (std::is_class_v<T>)
                value = T();
            else
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

        MDRegisterEntry_S& operator=(const T* otherValue)
        {
            memcpy(value, otherValue, N);

            return *this;
        }

        const T* operator=(MDRegisterEntry_S& reg) const
        {
            return value;
        }

        constexpr size_t getSize() const
        {
            return sizeof(T[N]);
        }

        constexpr size_t getSerializedSize() const
        {
            return sizeof(T[N]) + sizeof(m_regAddress);
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
            if (data.size() < getSerializedSize())
                return false;
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

    // MD Strings
    struct MDCanDatarateValue_S
    {
        static inline const std::map<u32, std::string_view> fromNumericMap{
            {1'000'000, "1M"}, {2'000'000, "2M"}, {5'000'000, "5M"}, {8'000'000, "8M"}};
        static inline const std::map<std::string_view, u32> toNumericMap{
            {"1M", 1'000'000}, {"2M", 2'000'000}, {"5M", 5'000'000}, {"8M", 8'000'000}};

        static std::optional<u32> toNumeric(const std::string_view val)
        {
            auto it = toNumericMap.find(val);
            if (it != toNumericMap.end())
            {
                return it->second;
            }
            return std::nullopt;
        }

        static std::optional<std::string> toReadable(u32 val)
        {
            auto it = fromNumericMap.find(val);
            if (it != fromNumericMap.end())
            {
                return std::string(it->second);
            }
            return std::nullopt;
        }
    };
    struct MDAuxEncoderValue_S
    {
        static inline const std::map<u32, std::string_view> fromNumericMap{
            {0, "NONE"},
            {1, "ME_AM_CENTER"},
            {2, "ME_AM_OFFAXIS"},
            {3, "RLS_17B_RS422"},
            {4, "CM_OFFAXIS"},
            {5, "M24B_CENTER"},
            {6, "M24B_OFFAXIS"},
            {7, "DUAL_ENCODER"},
            {8, "ONBOARD"},
            {9, "RLS_17B_SPI"},
            {10, "RLS_ORBIS_RS422"},
            {11, "CE300"},
            {12, "RLS_MB022"},
            {13, "AR50"},

        };
        static inline const std::map<std::string_view, u32> toNumericMap{
            {"NONE", 0},
            {"ME_AS_CENTER", 1},
            {"ME_AM_CENTER", 1},
            {"ME_AS_OFFAXIS", 2},
            {"ME_AM_OFFAXIS", 2},
            {"RLS_17B_RS422", 3},
            {"MB053SFA17BENT00", 3},  // deprecated
            {"CM_OFFAXIS", 4},
            {"M24B_CENTER", 5},
            {"M24B_OFFAXIS", 6},
            {"DUAL_ENCODER", 7},
            {"ONBOARD", 8},
            {"RLS_17B_SPI", 9},
            {"RLS_ORBIS_RS422", 10},
            {"CE300", 11},
            {"RLS_MB022", 12},
            {"AR50", 13},
        };

        static std::optional<u32> toNumeric(const std::string_view val)
        {
            if (std::isdigit(val[0]))
                return std::stoi(val.data());
            auto it = toNumericMap.find(val);
            if (it != toNumericMap.end())
            {
                return it->second;
            }
            return std::nullopt;
        }

        static std::optional<std::string> toReadable(u32 val)
        {
            auto it = fromNumericMap.find(val);
            if (it != fromNumericMap.end())
            {
                return std::string(it->second);
            }
            return std::nullopt;
        }
    };
    // DIR IS BROKEN! So it is not included here until it is fixed
    struct MDAuxEncoderModeValue_S
    {
        static inline const std::map<u32, std::string_view> fromNumericMap{{0, "NONE"},
                                                                           {1, "STARTUP"},
                                                                           {2, "MOTION"},
                                                                           {3, "REPORT"},
                                                                           {4, "MAIN"},
                                                                           {5, "CALIBRATED_REPORT"},
                                                                           {6, "DUAL"}};
        static inline const std::map<std::string_view, u32> toNumericMap{{"NONE", 0},
                                                                         {"STARTUP", 1},
                                                                         {"MOTION", 2},
                                                                         {"REPORT", 3},
                                                                         {"MAIN", 4},
                                                                         {"CALIBRATED_REPORT", 5},
                                                                         {"DUAL", 6}};

        static std::optional<u32> toNumeric(const std::string_view val)
        {
            if (std::isdigit(val[0]))
                return std::stoi(val.data());
            auto it = toNumericMap.find(val);
            if (it != toNumericMap.end())
            {
                return it->second;
            }
            return std::nullopt;
        }

        static std::optional<std::string> toReadable(u32 val)
        {
            auto it = fromNumericMap.find(val);
            if (it != fromNumericMap.end())
            {
                return std::string(it->second);
            }
            return std::nullopt;
        }
    };
    struct MDMainEncoderCalibrationModeValue_S
    {
        static inline const std::map<u32, std::string_view> fromNumericMap{{0, "FULL"},
                                                                           {1, "NOPPDET"}};
        static inline const std::map<std::string_view, u32> toNumericMap{{"FULL", 0},
                                                                         {"NOPPDET", 1}};

        static std::optional<u32> toNumeric(const std::string_view val)
        {
            if (std::isdigit(val[0]))
                return std::stoi(val.data());
            auto it = toNumericMap.find(val);
            if (it != toNumericMap.end())
            {
                return it->second;
            }
            return std::nullopt;
        }

        static std::optional<std::string> toReadable(u32 val)
        {
            auto it = fromNumericMap.find(val);
            if (it != fromNumericMap.end())
            {
                return std::string(it->second);
            }
            return std::nullopt;
        }
    };
    struct MDAuxEncoderCalibrationModeValue_S
    {
        static inline const std::map<u32, std::string_view> fromNumericMap{{0, "FULL"},
                                                                           {1, "DIRONLY"}};
        static inline const std::map<std::string_view, u32> toNumericMap{{"FULL", 0},
                                                                         {"DIRONLY", 1}};

        static std::optional<u32> toNumeric(const std::string_view val)
        {
            if (std::isdigit(val[0]))
                return std::stoi(val.data());
            auto it = toNumericMap.find(val);
            if (it != toNumericMap.end())
            {
                return it->second;
            }
            return std::nullopt;
        }

        static std::optional<std::string> toReadable(u32 val)
        {
            auto it = fromNumericMap.find(val);
            if (it != fromNumericMap.end())
            {
                return std::string(it->second);
            }
            return std::nullopt;
        }
    };
    struct MDUserGpioConfigurationValue_S
    {
        static inline const std::map<u32, std::string_view> fromNumericMap{
            {0, "OFF"}, {1, "BRAKE"}, {2, "GPIO_INPUT"}};
        static inline const std::map<std::string_view, u32> toNumericMap{
            {"OFF", 0}, {"AUTO_BRAKE", 1}, {"BRAKE", 1}, {"GPIO_INPUT", 2}};

        static std::optional<u32> toNumeric(const std::string_view val)
        {
            if (std::isdigit(val[0]))
                return std::stoi(val.data());
            auto it = toNumericMap.find(val);
            if (it != toNumericMap.end())
            {
                return it->second;
            }
            return std::nullopt;
        }

        static std::optional<std::string> toReadable(u32 val)
        {
            auto it = fromNumericMap.find(val);
            if (it != fromNumericMap.end())
            {
                return std::string(it->second);
            }
            return std::nullopt;
        }
    };
    struct MDTorqueSensorType_S
    {
        static inline const std::map<u32, std::string_view> fromNumericMap{{0, "OFF"}, {1, "XJC"}};
        static inline const std::map<std::string_view, u32> toNumericMap{{"OFF", 0}, {"XJC", 1}};

        static std::optional<u32> toNumeric(const std::string_view val)
        {
            if (std::isdigit(val[0]))
                return std::stoi(val.data());
            auto it = toNumericMap.find(val);
            if (it != toNumericMap.end())
                return it->second;
            return std::nullopt;
        }
        static std::optional<std::string> toReadable(u32 val)
        {
            auto it = fromNumericMap.find(val);
            if (it != fromNumericMap.end())
                return std::string(it->second);
            return std::nullopt;
        }
    };
    struct MDBuildDateValue_S
    {
        static std::optional<u32> toNumeric(const std::string_view val) = delete;

        static std::optional<std::string> toReadable(u32 val)
        {
            return std::to_string(val % 100) + '.' + std::to_string((val / 100) % 100) + '.' +
                   "20" + std::to_string(val / 10000);
        }
    };
    struct FirmwareVersionValue_S
    {
        static std::optional<u32> toNumeric(const std::string_view val) = delete;

        static std::optional<std::string> toReadable(u32 val)
        {
            version_ut  version;
            std::string output;
            version.i = val;
            output = std::to_string(version.s.major) + "." + std::to_string(version.s.minor) + "." +
                     std::to_string(version.s.revision) + version.s.tag;
            return output;
        }
    };
    struct MDLegacyHwVersion_S
    {
        static std::optional<u32> toNumeric(const std::string_view val) = delete;

        static std::optional<std::string> toReadable(u32 val)
        {
            switch (val)
            {
                case 0:
                    return "HV13";
                case 1:
                    return "HW11";
                case 2:
                    return "HW20";
                case 3:
                    return "HW21";
                case 4:
                    return "HW30";
                case 5:
                    return "HD10";
                default:
                    return "UNKNOWN";
            }
        }
    };
    struct MDDeviceRev_S
    {
        static std::optional<u32> toNumeric(const std::string_view val) = delete;

        static std::optional<std::string> toReadable(u8 val)
        {
            int major = val / 10;
            int minor = val % 10;
            return "v" + std::to_string(major) + "." + std::to_string(minor);
        }
    };
    struct MDRegisterAccessError_S
    {
        static std::string toReadable(mab::MdRegisterAccessErrorCode code)
        {
            switch (code)
            {
                case mab::MdRegisterAccessErrorCode::NONE:
                    return "NO ERROR";
                case mab::MdRegisterAccessErrorCode::ACCESS:
                    return "ACCESS (read/write not permitted)";
                case mab::MdRegisterAccessErrorCode::DEPRECATED:
                    return "DEPRECATED (register no longer used)";
                case mab::MdRegisterAccessErrorCode::INVALID:
                    return "INVALID (request/format not valid)";
                case mab::MdRegisterAccessErrorCode::OUT_OF_RANGE:
                    return "OUT OF RANGE";
                case mab::MdRegisterAccessErrorCode::UNKNOWN:
                    return "UNKNOWN (undefined)";
            }
            return "ERROR_CODE_UNKNOWN";
        }
    };

    // MD Status
    struct MDStatus
    {
        struct StatusItem_S
        {
            const std::string name;
            const bool        isError = false;
            mutable bool      m_set   = false;

            StatusItem_S(std::string _name, bool _isError) : name(_name), isError(_isError)
            {
            }

            operator bool() const
            {
                return isSet();
            }

            inline void set(bool _set)
            {
                m_set = _set;
            }

            inline bool isSet() const
            {
                return m_set;
            }
        };

        using bitPos = u8;

        enum class QuickStatusBits : bitPos
        {
            MainEncoderStatus        = 0,
            OutputEncoderStatus      = 1,
            CalibrationEncoderStatus = 2,
            MosfetBridgeStatus       = 3,
            HardwareStatus           = 4,
            CommunicationStatus      = 5,
            MotionStatus             = 6,
            TargetPositionReached    = 15
        };

        std::unordered_map<QuickStatusBits, StatusItem_S> quickStatus = {
            {QuickStatusBits::MainEncoderStatus, StatusItem_S("Main Encoder Status", false)},
            {QuickStatusBits::OutputEncoderStatus, StatusItem_S("Output Encoder Status", false)},
            {QuickStatusBits::CalibrationEncoderStatus,
             StatusItem_S("Calibration Encoder Status", false)},
            {QuickStatusBits::MosfetBridgeStatus, StatusItem_S("Mosfet Bridge Status", false)},
            {QuickStatusBits::HardwareStatus, StatusItem_S("Hardware Status", false)},
            {QuickStatusBits::CommunicationStatus, StatusItem_S("Communication Status", false)},
            {QuickStatusBits::MotionStatus, StatusItem_S("Motion Status", false)},
            {QuickStatusBits::TargetPositionReached,
             StatusItem_S("Target Position Reached", false)}  //
        };

        enum class EncoderStatusBits : bitPos
        {
            ErrorCommunication   = 0,
            ErrorWrongDirection  = 1,
            ErrorEmptyLUT        = 2,
            ErrorFaultyLUT       = 3,
            ErrorCalibration     = 4,
            ErrorPositionInvalid = 5,
            ErrorInitialization  = 6,
            WarningLowAccuracy   = 30
        };

        std::unordered_map<EncoderStatusBits, StatusItem_S> encoderStatus = {
            {EncoderStatusBits::ErrorCommunication, StatusItem_S("Error Communication", true)},
            {EncoderStatusBits::ErrorWrongDirection, StatusItem_S("Error Wrong Direction", true)},
            {EncoderStatusBits::ErrorEmptyLUT, StatusItem_S("Error Empty LUT", true)},
            {EncoderStatusBits::ErrorFaultyLUT, StatusItem_S("Error Faulty LUT", true)},
            {EncoderStatusBits::ErrorCalibration, StatusItem_S("Error Calibration", true)},
            {EncoderStatusBits::ErrorPositionInvalid, StatusItem_S("Error Position Invalid", true)},
            {EncoderStatusBits::ErrorInitialization, StatusItem_S("Error Initialization", true)},
            {EncoderStatusBits::WarningLowAccuracy, StatusItem_S("Warning Low Accuracy", false)}};

        enum class CalibrationStatusBits : bitPos
        {
            ErrorOffsetCalibration = 0,
            ErrorResistance        = 1,
            ErrorInductance        = 2,
            ErrorPolePairDetection = 3,
            ErrorSetup             = 4
        };

        std::unordered_map<CalibrationStatusBits, StatusItem_S> calibrationStatus = {
            {CalibrationStatusBits::ErrorOffsetCalibration,
             StatusItem_S("Error Offset Calibration", true)},
            {CalibrationStatusBits::ErrorResistance, StatusItem_S("Error Resistance", true)},
            {CalibrationStatusBits::ErrorInductance, StatusItem_S("Error Inductance", true)},
            {CalibrationStatusBits::ErrorPolePairDetection,
             StatusItem_S("Error Pole Pair Detection", true)},
            {CalibrationStatusBits::ErrorSetup, StatusItem_S("Error Setup", true)}};

        enum class BridgeStatusBits : bitPos
        {
            ErrorCommunication = 0,
            ErrorOvercurrent   = 1,
            ErrorGeneralFault  = 2
        };

        std::unordered_map<BridgeStatusBits, StatusItem_S> bridgeStatus = {
            {BridgeStatusBits::ErrorCommunication, StatusItem_S("Error Communication", true)},
            {BridgeStatusBits::ErrorOvercurrent, StatusItem_S("Error Overcurrent", true)},
            {BridgeStatusBits::ErrorGeneralFault, StatusItem_S("Error General Fault", true)}};

        enum class HardwareStatusBits : bitPos
        {
            ErrorOverCurrent       = 0,
            ErrorOverVoltage       = 1,
            ErrorUnderVoltage      = 2,
            ErrorMotorTemperature  = 3,
            ErrorMosfetTemperature = 4,
            ErrorADCCurrentOffset  = 5
        };

        std::unordered_map<HardwareStatusBits, StatusItem_S> hardwareStatus = {
            {HardwareStatusBits::ErrorOverCurrent, StatusItem_S("Error Over Current", true)},
            {HardwareStatusBits::ErrorOverVoltage, StatusItem_S("Error Over Voltage", true)},
            {HardwareStatusBits::ErrorUnderVoltage, StatusItem_S("Error Under Voltage", true)},
            {HardwareStatusBits::ErrorMotorTemperature,
             StatusItem_S("Error Motor Temperature", true)},
            {HardwareStatusBits::ErrorMosfetTemperature,
             StatusItem_S("Error Mosfet Temperature", true)},
            {HardwareStatusBits::ErrorADCCurrentOffset,
             StatusItem_S("Error ADC Current Offset", true)}};

        enum class CommunicationStatusBits : bitPos
        {
            WarningCANWatchdog = 30
        };

        std::unordered_map<CommunicationStatusBits, StatusItem_S> communicationStatus = {
            {CommunicationStatusBits::WarningCANWatchdog,
             StatusItem_S("Warning CAN Watchdog", false)}};

        enum class MotionStatusBits : bitPos
        {
            ErrorPositionLimit  = 0,
            ErrorVelocityLimit  = 1,
            WarningAcceleration = 24,
            WarningTorque       = 25,
            WarningVelocity     = 26,
            WarningPosition     = 27
        };

        std::unordered_map<MotionStatusBits, StatusItem_S> motionStatus = {
            {MotionStatusBits::ErrorPositionLimit, StatusItem_S("Error Position Limit", true)},
            {MotionStatusBits::ErrorVelocityLimit, StatusItem_S("Error Velocity Limit", true)},
            {MotionStatusBits::WarningAcceleration,
             StatusItem_S("Warning Acceleration Clipped", false)},
            {MotionStatusBits::WarningTorque, StatusItem_S("Warning Torque Clipped", false)},
            {MotionStatusBits::WarningVelocity,
             StatusItem_S("Warning Velocity Target Clipped", false)},
            {MotionStatusBits::WarningPosition,
             StatusItem_S("Warning Position Target Clipped", false)}};

        enum class MiscStatusBits : bitPos
        {
            ErrorOTPMemoryCorrupted = 0,
            WarnRestartRequired     = 29,
            WarnRoutineInProgress   = 30
        };
        std::unordered_map<MiscStatusBits, StatusItem_S> miscStatus = {
            {MiscStatusBits::ErrorOTPMemoryCorrupted,
             StatusItem_S("Error OTP Memory Corrupted", true)},
            {MiscStatusBits::WarnRestartRequired, StatusItem_S("Restart Required", false)},
            {MiscStatusBits::WarnRoutineInProgress, StatusItem_S("Routine in progress", false)},
        };

        enum class ConfigStatusBits : bitPos
        {
            ErrorSave                 = 1,
            ErrorMotorParams          = 2,
            ErrorLimits               = 3,
            WarnSavingRequired        = 28,
            WarnBootloaderInfoMissing = 29,
            WarnConfigEntryMissing    = 30
        };
        std::unordered_map<ConfigStatusBits, StatusItem_S> configStatus = {
            {ConfigStatusBits::ErrorSave, StatusItem_S("Error during Saving", true)},
            {ConfigStatusBits::ErrorMotorParams, StatusItem_S("Invalid Motor Parameters", true)},
            {ConfigStatusBits::ErrorLimits, StatusItem_S("Invalid Limits", true)},
            {ConfigStatusBits::WarnSavingRequired, StatusItem_S("Saving required", false)},
            {ConfigStatusBits::WarnBootloaderInfoMissing,
             StatusItem_S("Bootloader Information Missing", false)},
            {ConfigStatusBits::WarnConfigEntryMissing,
             StatusItem_S("Missing Config Entry (Default used)", false)},
        };

        static std::vector<std::string> getStatusString(
            std::unordered_map<bitPos, StatusItem_S> errors)
        {
            std::vector<std::string> activeErrors;
            for (const auto& err : errors)
            {
                if (err.second.m_set)
                    activeErrors.push_back(err.second.name);
            }
            return activeErrors;
        }

        template <class T>
        static void decode(u32 bytes, std::unordered_map<T, StatusItem_S>& map)
        {
            for (auto& bit : map)
            {
                bit.second.set(static_cast<bool>(bytes & (1 << static_cast<u8>(bit.first))));
            }
        }
    };

}  // namespace mab
