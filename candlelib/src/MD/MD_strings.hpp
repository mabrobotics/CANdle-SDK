#pragma once

#include <cctype>
#include <cstdlib>
#include <string>
#include <mab_types.hpp>
#include <map>
#include <optional>

namespace mab
{
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
        static inline const std::map<u32, std::string_view> fromNumericMap{{0, "NONE"},
                                                                           {1, "ME_AS_CENTER"},
                                                                           {2, "ME_AS_OFFAXIS"},
                                                                           {3, "MB053SFA17BENT00"},
                                                                           {4, "CM_OFFAXIS"},
                                                                           {5, "M24B_CENTER"},
                                                                           {6, "M24B_OFFAXIS"}};
        static inline const std::map<std::string_view, u32> toNumericMap{{"NONE", 0},
                                                                         {"ME_AS_CENTER", 1},
                                                                         {"ME_AS_OFFAXIS", 2},
                                                                         {"MB053SFA17BENT00", 3},
                                                                         {"CM_OFFAXIS", 4},
                                                                         {"M24B_CENTER", 5},
                                                                         {"M24B_OFFAXIS", 6}};

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
        static inline const std::map<u32, std::string_view> fromNumericMap{
            {0, "NONE"},
            {1, "STARTUP"},
            {2, "MOTION"},
            {3, "REPORT"},
            {4, "MAIN"},
            {5, "CALIBRATED_REPORT"}};
        static inline const std::map<std::string_view, u32> toNumericMap{{"NONE", 0},
                                                                         {"STARTUP", 1},
                                                                         {"MOTION", 2},
                                                                         {"REPORT", 3},
                                                                         {"MAIN", 4},
                                                                         {"CALIBRATED_REPORT", 5}};

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
            {0, "OFF"}, {1, "AUTO_BRAKE"}, {2, "GPIO_INPUT"}};
        static inline const std::map<std::string_view, u32> toNumericMap{
            {"OFF", 0}, {"AUTO_BRAKE", 1}, {"GPIO_INPUT", 2}};

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
                case 0xB0:
                    return "MD0004";
                default:
                    return "UNKNOWN";
            }
        }
    };
}  // namespace mab
