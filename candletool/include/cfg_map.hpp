#pragma once

#include "logger.hpp"
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
    enum CfgFlags_E : u8
    {
        CFG_NORMAL    = 0,       // missing value is replaced by the default
        CFG_CRITICAL  = 1 << 0,  // missing value stops the upload
        CFG_SKIP_ZERO = 1 << 1,  // 0 means not set, nothing is written to the drive
    };

    struct CfgMap_S
    {
        const char* section;
        const char* key;
        u16         mdReg;         // MD register, 0 when MD has no such register
        u16         coIndex;       // MDCO object in the md_1.2 layout, 0 when MDCO has none
        u8          coSubindex;    // 0 for objects that are not records
        u8          flags;         // CfgFlags_E
        const char* defaultValue;  // used when the key is missing, nullptr to write nothing
    };

    // clang-format off
    // ADD NEW CONFIGURATION PARAMETERS HERE
    // KV is listed before the torque constant so it is written first and the torque constant,
    // when set, takes priority on the drive
    inline constexpr CfgMap_S CFG_MAP[] = {
        // section          key                                MD reg  CO index CO sub  flags          default
        {"motor",           "name",                            0x010,  0x2000,  0x06,   CFG_NORMAL,    "MD"},
        {"motor",           "pole pairs",                      0x011,  0x2000,  0x02,   CFG_CRITICAL,  nullptr},
        {"motor",           "KV",                              0x01D,  0x2000,  0x0A,   CFG_SKIP_ZERO, "0"},
        {"motor",           "torque constant",                 0x012,  0x2000,  0x09,   CFG_SKIP_ZERO, "0.0"},
        // MDCO only for now, CANopen torque and current permille objects are relative to them
        {"motor",           "rated current",                   0,      0x6075,  0x00,   CFG_SKIP_ZERO, "0.0"},
        {"motor",           "rated torque",                    0,      0x6076,  0x00,   CFG_SKIP_ZERO, "0.0"},
        {"motor",           "max current",                     0x016,  0x6073,  0x00,   CFG_CRITICAL,  nullptr},
        {"motor",           "gear ratio",                      0x017,  0x6091,  0x01,   CFG_CRITICAL,  nullptr},
        {"motor",           "torque bandwidth",                0x018,  0x2000,  0x05,   CFG_CRITICAL,  nullptr},
        {"motor",           "calibration mode",                0x01E,  0x2000,  0x08,   CFG_NORMAL,    "FULL"},
        {"motor",           "shutdown temp",                   0x808,  0x2000,  0x07,   CFG_NORMAL,    "80"},
        {"output encoder",  "output encoder",                  0x020,  0x2002,  0x01,   CFG_NORMAL,    "NONE"},
        {"output encoder",  "output encoder dir",              0x021,  0x2002,  0x02,   CFG_NORMAL,    "1.0"},
        {"output encoder",  "output encoder mode",             0x025,  0x2002,  0x03,   CFG_NORMAL,    "NONE"},
        {"output encoder",  "output encoder calibration mode", 0x026,  0x2002,  0x04,   CFG_NORMAL,    "FULL"},
        {"main encoder",    "main encoder",                    0x02A,  0x2001,  0x01,   CFG_NORMAL,    "ONBOARD"},
        {"main encoder",    "dir",                             0x02B,  0x2001,  0x02,   CFG_SKIP_ZERO, "0.0"},
        {"position PID",    "kp",                              0x030,  0x2010,  0x01,   CFG_NORMAL,    "0.0"},
        {"position PID",    "ki",                              0x031,  0x2010,  0x02,   CFG_NORMAL,    "0.0"},
        {"position PID",    "kd",                              0x032,  0x2010,  0x03,   CFG_NORMAL,    "0.0"},
        {"position PID",    "windup",                          0x034,  0x2010,  0x04,   CFG_NORMAL,    "0.0"},
        {"velocity PID",    "kp",                              0x040,  0x2011,  0x01,   CFG_NORMAL,    "0.0"},
        {"velocity PID",    "ki",                              0x041,  0x2011,  0x02,   CFG_NORMAL,    "0.0"},
        {"velocity PID",    "kd",                              0x042,  0x2011,  0x03,   CFG_NORMAL,    "0.0"},
        {"velocity PID",    "windup",                          0x044,  0x2011,  0x04,   CFG_NORMAL,    "0.0"},
        {"impedance PD",    "kp",                              0x050,  0x2012,  0x01,   CFG_NORMAL,    "0.0"},
        {"impedance PD",    "kd",                              0x051,  0x2012,  0x02,   CFG_NORMAL,    "0.0"},
        {"limits",          "max torque",                      0x112,  0x6072,  0x00,   CFG_CRITICAL,  nullptr},
        {"limits",          "max position",                    0x110,  0x607D,  0x02,   CFG_NORMAL,    "0.0"},
        {"limits",          "min position",                    0x111,  0x607D,  0x01,   CFG_NORMAL,    "0.0"},
        {"limits",          "max velocity",                    0x113,  0x6080,  0x00,   CFG_CRITICAL,  nullptr},
        {"limits",          "max acceleration",                0x114,  0x60C5,  0x00,   CFG_NORMAL,    "100.0"},
        {"limits",          "max deceleration",                0x115,  0x60C6,  0x00,   CFG_NORMAL,    "100.0"},
        // legacy, written only when the file sets it
        {"profile",         "velocity",                        0x120,  0x6081,  0x00,   CFG_NORMAL,    nullptr},
        {"profile",         "acceleration",                    0x121,  0x6083,  0x00,   CFG_NORMAL,    "100.0"},
        {"profile",         "deceleration",                    0x122,  0x6084,  0x00,   CFG_NORMAL,    "100.0"},
        {"profile",         "quick stop deceleration",         0x123,  0x6085,  0x00,   CFG_NORMAL,    "100.0"},
        {"profile",         "position window",                 0x124,  0x6067,  0x00,   CFG_NORMAL,    "0.1"},
        {"profile",         "velocity window",                 0x125,  0x606D,  0x00,   CFG_NORMAL,    "0.4"},
        {"hardware",        "shunt resistance",                0x700,  0,       0x00,   CFG_NORMAL,    nullptr},
        {"GPIO",            "mode",                            0x160,  0x2024,  0x01,   CFG_NORMAL,    "OFF"},
        {"sensors",         "torque",                          0x200,  0x2003,  0x01,   CFG_NORMAL,    "OFF"},
    };
    // clang-format on

    inline constexpr size_t CFG_MAP_SIZE = sizeof(CFG_MAP) / sizeof(CFG_MAP[0]);

    /// @brief Values of a .cfg file as written in it, indexed like CFG_MAP, empty when absent
    struct CfgValues_S
    {
        std::string value[CFG_MAP_SIZE];
        bool        isDefault[CFG_MAP_SIZE] = {};  // value came from CFG_MAP, not the file
    };

    /// @brief Read a .cfg file, missing keys get their default
    /// @return false when the file is unreadable, a critical key is missing or both KV and the
    /// torque constant are 0
    bool cfgLoad(const std::filesystem::path& path, CfgValues_S& cfg);

    /// @brief Whether the value is to be written to the drive, see CfgFlags_E
    bool cfgIsSet(const CfgMap_S& entry, const std::string& value);

    /// @brief Print a value written to the drive next to the file value it came from, defaults
    /// are tagged and printed in yellow
    /// @param source "section.key = value" as in the file
    /// @param target where it was written, e.g. "0x011" or "0x6075:00 Motor Rated Current"
    void cfgPrintWrite(const Logger&      log,
                       const std::string& source,
                       bool               isDefault,
                       const std::string& target,
                       const std::string& value);

    /// @brief Write a .cfg file, empty values are left out
    bool cfgSave(const std::filesystem::path& path, const CfgValues_S& cfg);

    /// @brief Check values against the min/max and enum rules of the schema
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
