#include "utilities.hpp"
#include "MDStatus.hpp"
#include <algorithm>
#include <cctype>
#include <iostream>
#include <variant>
#include <string>
#include <string_view>
#include <unordered_map>

namespace mab
{
    std::string trim(const std::string_view s)
    {
        auto start = std::find_if_not(s.begin(), s.end(), ::isspace);
        auto end   = std::find_if_not(s.rbegin(), s.rend(), ::isspace).base();
        return (start < end) ? std::string(start, end) : "";
    }

    /// @brief Names of the set status flags, errors in red and warnings in yellow, OK when none
    template <typename T>
    static std::string statusToString(u32 raw, std::unordered_map<T, MDStatus::StatusItem_S>& map)
    {
#ifndef WIN32
        const std::string red = "\033[1;31m", yellow = "\033[1;33m", green = "\033[1;32m",
                          reset = "\033[0m";
#else
        const std::string red, yellow, green, reset;
#endif
        MDStatus::decode(raw, map);
        std::string result;
        for (const auto& [key, item] : map)
        {
            if (!item.isSet())
                continue;
            if (!result.empty())
                result += ", ";
            result += (item.isError ? red : yellow) + item.name + reset;
        }
        if (result.empty())
            return green + "OK" + reset;
        return "Set flags: " + result;
    }

    void printStatusSummary(Logger& log, const DriveStatus_S& status)
    {
        MDStatus s;
        log << std::hex;
        log << "***** ERRORS *****" << std::endl;
        log << "- main encoder error: \t0x" << status.mainEncoder << " ("
            << statusToString(status.mainEncoder, s.encoderStatus) << ")" << std::endl;
        if (status.hasAuxEncoder)
            log << "- aux encoder status: \t0x" << status.auxEncoder << " ("
                << statusToString(status.auxEncoder, s.encoderStatus) << ")" << std::endl;
        log << "- calibration status: \t0x" << status.calibration << " ("
            << statusToString(status.calibration, s.calibrationStatus) << ")" << std::endl;
        log << "- bridge status: \t\t0x" << status.bridge << " ("
            << statusToString(status.bridge, s.bridgeStatus) << ")" << std::endl;
        log << "- hardware status: \t\t0x" << status.hardware << " ("
            << statusToString(status.hardware, s.hardwareStatus) << ")" << std::endl;
        log << "- communication status: \t0x" << status.communication << " ("
            << statusToString(status.communication, s.communicationStatus) << ")" << std::endl;
        log << "- motion status: \t\t0x" << status.motion << " ("
            << statusToString(status.motion, s.motionStatus) << ")" << std::endl;
        log << "- misc status: \t\t0x" << status.misc << " ("
            << statusToString(status.misc, s.miscStatus) << ")" << std::endl;
        log << "- config status: \t\t0x" << status.config << " ("
            << statusToString(status.config, s.configStatus) << ")" << std::endl;
        log << std::dec;
    }

    version_ut getMdFirmwareVersion(MD& md)
    {
        md.readRegister(md.m_mdRegisters.firmwareVersion);
        return {.i = md.m_mdRegisters.firmwareVersion.value};
    }

    bool isVersionAtLeast(version_ut fwVersion, int major, int minor, int rev)
    {
        if (fwVersion.s.major < major || fwVersion.s.minor < minor || fwVersion.s.revision < rev)
            return false;
        return true;
    }

    bool userConfirm()
    {
        std::string answer;
        std::cout << "Continue? [y/N]: ";
        std::getline(std::cin, answer);
        return answer == "y" || answer == "Y";
    }

}  // namespace mab
