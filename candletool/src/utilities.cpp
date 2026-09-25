#include "utilities.hpp"
#include <algorithm>
#include <cctype>
#include <iostream>
#include <variant>
#include <string>
#include <string_view>

namespace mab
{
    std::string trim(const std::string_view s)
    {
        auto start = std::find_if_not(s.begin(), s.end(), ::isspace);
        auto end   = std::find_if_not(s.rbegin(), s.rend(), ::isspace).base();
        return (start < end) ? std::string(start, end) : "";
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
