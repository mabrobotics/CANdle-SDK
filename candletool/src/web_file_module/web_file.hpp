#pragma once

#include <filesystem>

namespace mab
{

    struct WebFile_S
    {
        enum class Type_E
        {
            UNKNOWN,
            MD_FLASHER,
            CANDLE_FLASHER,
            MAB_FILE,
            MOTOR_CONFIG,
            PDS_CONFIG
        };
        Type_E                m_type = Type_E::UNKNOWN;
        std::filesystem::path m_path = "";

        static inline Type_E strToType(std::string_view str)
        {
            if (str == "MD_FLASHER")
                return Type_E::MD_FLASHER;
            if (str == "CANDLE_FLASHER")
                return Type_E::CANDLE_FLASHER;
            if (str == "MAB_FILE")
                return Type_E::MAB_FILE;
            if (str == "MOTOR_CONFIG")
                return Type_E::MOTOR_CONFIG;
            if (str == "PDS_CONFIG")
                return Type_E::PDS_CONFIG;
            return Type_E::UNKNOWN;
        }

        static inline std::string typeToStr(Type_E type)
        {
            switch (type)
            {
                case Type_E::MD_FLASHER:
                    return "MD_FLASHER";
                case Type_E::CANDLE_FLASHER:
                    return "CANDLE_FLASHER";
                case Type_E::MAB_FILE:
                    return "MAB_FILE";
                case Type_E::MOTOR_CONFIG:
                    return "MOTOR_CONFIG";
                case Type_E::PDS_CONFIG:
                    return "PDS_CONFIG";
                default:
                    return "UNKNOWN";
            }
        }
    };
}  // namespace mab