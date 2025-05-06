#include <string>
#include <filesystem>
#include "mab_types.hpp"

#include "mini/ini.h"

namespace mab
{

    inline std::string getDefaultConfigDir()
    {
#ifdef WIN32
        char path[256];
        GetModuleFileName(NULL, path, 256);
        return std::filesystem::path(path).remove_filename().string() + std::string("config\\");
#else
        return std::string("/etc/candletool/config/");
#endif
    }

    inline std::string getMotorsConfigPath()
    {
#ifdef WIN32
        return getDefaultConfigDir() + "motors\\";
#else
        return getDefaultConfigDir() + "motors/";
#endif
    }

    inline std::string getDefaultConfigPath()
    {
        return getMotorsConfigPath() + "default.cfg";
    }

    inline std::string getCandletoolConfigPath()
    {
        return getDefaultConfigDir() + "candletool.ini";
    }

    inline bool fileExists(const std::string& filepath)
    {
        std::ifstream fileStream(filepath);
        return fileStream.good();
    }

    inline bool isConfigValid(const std::string& pathToConfig)
    {
        std::string fileExtension = std::filesystem::path(pathToConfig).extension().string();
        if (!(fileExtension == ".cfg"))
            return false;
        std::error_code   ec;
        u32               filesize = (u32)std::filesystem::file_size(pathToConfig, ec);
        const std::size_t oneMB    = 1048576;  // 1 MB in bytes
        if (filesize > oneMB || ec)
            return false;
        return true;
    }

    inline bool isConfigComplete(const std::string& pathToConfig)
    {
        mINI::INIFile      defaultFile(getDefaultConfigPath());
        mINI::INIStructure defaultIni;
        defaultFile.read(defaultIni);

        mINI::INIFile      userFile(pathToConfig);
        mINI::INIStructure userIni;
        userFile.read(userIni);

        // Loop fills all lacking fields in the user's config file.
        for (auto const& it : defaultIni)
        {
            auto const& section    = it.first;
            auto const& collection = it.second;
            for (auto const& it2 : collection)
            {
                auto const& key = it2.first;
                if (!userIni[section].has(key))
                    return false;
            }
        }
        return true;
    }

    inline std::string generateUpdatedConfigFile(const std::string& pathToConfig)
    {
        mINI::INIFile      defaultFile(getDefaultConfigPath());
        mINI::INIStructure defaultIni;
        defaultFile.read(defaultIni);
        mINI::INIFile      userFile(pathToConfig);
        mINI::INIStructure userIni;
        userFile.read(userIni);

        std::string updatedUserConfigPath =
            pathToConfig.substr(0, pathToConfig.find_last_of(".")) + "_updated.cfg";
        mINI::INIFile      updatedFile(updatedUserConfigPath);
        mINI::INIStructure updatedIni;
        updatedFile.read(updatedIni);

        // Loop fills all lacking fields in the user's config file.
        for (auto const& it : defaultIni)
        {
            auto const& section    = it.first;
            auto const& collection = it.second;
            for (auto const& it2 : collection)
            {
                auto const& key   = it2.first;
                auto const& value = it2.second;
                if (!userIni[section].has(key))
                    updatedIni[section][key] = value;
                else
                    updatedIni[section][key] = userIni.get(section).get(key);
            }
        }
        // Write an updated config file
        updatedFile.write(updatedIni, true);
        return updatedUserConfigPath;
    }

    inline bool getConfirmation()
    {
        char x;
        std::cin >> x;
        if (x == 'Y' || x == 'y')
            return true;
        return false;
    }

    inline std::string prettyFloatToString(f32 value, bool noDecimals = false)
    {
        std::stringstream ss;
        ss << std::fixed;

        if (noDecimals)
        {
            ss << std::setprecision(0);
            ss << value;
            return ss.str();
        }
        else
        {
            if (static_cast<int>(value) == value)
            {
                ss << std::setprecision(1);
                ss << value;
                return ss.str();
            }
            else
            {
                ss << std::setprecision(7);
                ss << value;
                std::string str = ss.str();
                return str.substr(0, str.find_last_not_of('0') + 1);
            }
        }
    }

    inline std::optional<CANdleBaudrate_E> stringToBaudrate(const std::string& baudrateStr)
    {
        if (baudrateStr == "1M")
            return CANdleBaudrate_E::CAN_BAUD_1M;
        else if (baudrateStr == "2M")
            return CANdleBaudrate_E::CAN_BAUD_2M;
        else if (baudrateStr == "5M")
            return CANdleBaudrate_E::CAN_BAUD_5M;
        else if (baudrateStr == "8M")
            return CANdleBaudrate_E::CAN_BAUD_8M;
        else
            throw std::invalid_argument("Invalid baudrate string: " + baudrateStr);
    }

    inline std::string baudrateToString(CANdleBaudrate_E baudrate)
    {
        switch (baudrate)
        {
            case CANdleBaudrate_E::CAN_BAUD_1M:
                return "1M";
            case CANdleBaudrate_E::CAN_BAUD_2M:
                return "2M";
            case CANdleBaudrate_E::CAN_BAUD_5M:
                return "5M";
            case CANdleBaudrate_E::CAN_BAUD_8M:
                return "8M";
            default:
                throw std::invalid_argument("Invalid CANdleBaudrate_E value");
        }
    }

}  // namespace mab
