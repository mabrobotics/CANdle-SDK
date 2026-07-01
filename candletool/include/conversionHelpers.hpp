#include "mdco_config_adapter.hpp"

const std::unordered_map<std::string_view, std::string> encoderTicksMap = {
    {"NONE", "16384"},            // NONE - AMS
    {"ME_AS_CENTER", "16384"},    // ME Center
    {"ME_AS_OFFAXIS", "16384"},   // ME Offaxis
    {"RLS_17B_RS422", "131072"},  // RLS 17B
    {"CM_OFFAXIS", "22"},         // CM Offaxis
    {"M24B_CENTER", "33"},        // M24B Center
    {"M24B_OFFAXIS", "44"},       // M24B Offaxis
    {"DUAL_ENCODER", "16384"},    // Dual
    {"ONBOARD", "16384"},         // Onboard - ams
    {"RLS_17B_SPI", "131072"},    // RLS17B
    {"RLS_ORBIS_RS422", "0"},     // RLS Orbis
    {"CE300", "0"},               // CE300
};

std::string_view getCPRFromCfg(mab::MDConfigMap& _config)
{  // get CPR value

    mab::MDAuxEncoderValue_S               mdTypeCheck;
    const std::map<u32, std::string_view>& mdType         = mdTypeCheck.fromNumericMap;
    std::string                            _configType    = _config.getValueByAddress(0x020);
    u32                                    _configTypeU32 = 0;
    std::from_chars(_configType.data(), _configType.data() + _configType.size(), _configTypeU32);
    ///
    std::string_view configCPR = encoderTicksMap.at(
        mdType.at(_configTypeU32));  // encoderTicksMap takes the value of mdType at _configTypeU32

    volatile i64 test;
    std::from_chars(configCPR.begin(), configCPR.end(), test);

    return configCPR;
};
