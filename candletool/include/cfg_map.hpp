#pragma once

#include "mab_types.hpp"
#include "MDCO.hpp"
#include "edsEntry.hpp"
#include "mini/ini.h"

#include <cstddef>
#include <filesystem>
#include <optional>
#include <string>

namespace mab
{
    /// @brief One parameter of the motor .cfg file and where it lives on each drive type
    ///
    /// The file always holds SI units and enums by name, the same for MD and MDCO. Data types,
    /// enums and CANopen unit scaling are resolved in cfg_map.cpp from mdReg and coIndex.
    struct CfgMap_S
    {
        const char* section;
        const char* key;
        u16         mdReg;       // MD register, 0 when MD has no such register
        u16         coIndex;     // MDCO object in the md_1.2 layout, 0 when MDCO has none
        u8          coSubindex;  // 0 for objects that are not records
    };

    // clang-format off
    // ADD NEW CONFIGURATION PARAMETERS HERE
    inline constexpr CfgMap_S CFG_MAP[] = {
        // section                        key                                    MD register   CANOpen index    CANOpen subindex
        {"motor",           "name",                            0x010, 0x2000, 0x06},
        {"motor",           "pole pairs",                      0x011, 0x2000, 0x02},
        {"motor",           "torque constant",                 0x012, 0x2000, 0x09},
        {"motor",           "max current",                     0x016, 0x6075, 0x00},
        {"motor",           "gear ratio",                      0x017, 0x6091, 0x01},
        {"motor",           "torque bandwidth",                0x018, 0x2000, 0x05},
        {"motor",           "KV",                              0x01D, 0x2000, 0x0A},
        {"motor",           "calibration mode",                0x01E, 0x2000, 0x08},
        {"motor",           "shutdown temp",                   0x808, 0x2000, 0x07},
        {"output encoder",  "output encoder",                  0x020, 0x2002, 0x01},
        {"output encoder", "output encoder dir",              0x021, 0x2002, 0x02},
        {"output encoder", "output encoder mode",             0x025, 0x2002, 0x03},
        {"output encoder", "output encoder calibration mode", 0x026, 0x2002, 0x04},
        {"main encoder",   "main encoder",                    0x02A, 0x2001, 0x01},
        {"main encoder",   "dir",                             0x02B, 0x2001, 0x02},
        {"position PID",   "kp",                              0x030, 0x2010, 0x01},
        {"position PID",   "ki",                              0x031, 0x2010, 0x02},
        {"position PID",   "kd",                              0x032, 0x2010, 0x03},
        {"position PID",   "windup",                          0x034, 0x2010, 0x04},
        {"velocity PID",   "kp",                              0x040, 0x2011, 0x01},
        {"velocity PID",   "ki",                              0x041, 0x2011, 0x02},
        {"velocity PID",   "kd",                              0x042, 0x2011, 0x03},
        {"velocity PID",   "windup",                          0x044, 0x2011, 0x04},
        {"impedance PD",   "kp",                              0x050, 0x2012, 0x01},
        {"impedance PD",   "kd",                              0x051, 0x2012, 0x02},
        {"limits",         "max torque",                      0x112, 0x6076, 0x00},
        {"limits",         "max position",                    0x110, 0x607D, 0x02},
        {"limits",         "min position",                    0x111, 0x607D, 0x01},
        {"limits",         "max velocity",                    0x113, 0x6080, 0x00},
        {"limits",         "max acceleration",                0x114, 0x60C5, 0x00},
        {"limits",         "max deceleration",                0x115, 0x60C6, 0x00},
        {"profile",        "velocity",                        0x120, 0x6081, 0x00},
        {"profile",        "acceleration",                    0x121, 0x6083, 0x00},
        {"profile",        "deceleration",                    0x122, 0x6084, 0x00},
        {"profile",        "quick stop deceleration",         0x123, 0x6085, 0x00},
        {"profile",        "position window",                 0x124, 0x6067, 0x00},
        {"profile",        "velocity window",                 0x125, 0x606D, 0x00},
        {"hardware",       "shunt resistance",                0x700, 0,      0x00},
        {"GPIO",           "mode",                            0x160, 0x2024, 0x01},
        {"sensors",        "torque",                          0x200, 0x2003, 0x01},
    };
    // clang-format on

    inline constexpr size_t CFG_MAP_SIZE = sizeof(CFG_MAP) / sizeof(CFG_MAP[0]);

    /// @brief Values of a .cfg file as written in it, indexed like CFG_MAP, empty when absent
    struct CfgValues_S
    {
        std::string value[CFG_MAP_SIZE];
    };

    /// @brief Read a .cfg file
    bool cfgLoad(const std::filesystem::path& path, CfgValues_S& cfg);

    /// @brief Write a .cfg file, empty values are left out
    bool cfgSave(const std::filesystem::path& path, const CfgValues_S& cfg);

    /// @brief Check values against the min/max, enum and required rules of the schema
    bool cfgVerify(const CfgValues_S& cfg, const mINI::INIStructure& schema);

    /// @brief Value as held by the drive, enums resolved to their number, nullopt when invalid
    std::optional<std::string> cfgToRaw(const CfgMap_S& entry, const std::string& text);

    /// @brief Value as written in the file, enum numbers resolved to their name
    std::string cfgFromRaw(const CfgMap_S& entry, const std::string& raw);

    /// @brief Write the values to an MDCO drive, converted to the units of its .eds
    /// @return false when anything could not be written
    bool cfgUploadMdco(MDCO& mdco, EDSObjectDictionary& od, const CfgValues_S& cfg);

    /// @brief Read the values from an MDCO drive, converted to SI units
    /// @return false when the .eds is not supported, unreadable objects are only reported
    bool cfgDownloadMdco(MDCO& mdco, EDSObjectDictionary& od, CfgValues_S& cfg);
}  // namespace mab
