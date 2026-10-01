#include "cfg_map.hpp"
#include "MDObjects.hpp"
#include "MD_strings.hpp"
#include "logger.hpp"

#include <cctype>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <string_view>

namespace mab
{
    namespace
    {
        // ---- Enums, keyed by MD register, the numbers are the same on MDCO -----------------

        struct CfgEnum_S
        {
            std::optional<u32> (*toNumeric)(std::string_view);
            std::optional<std::string> (*toReadable)(u32);
        };

        const CfgEnum_S* enumOf(u16 mdReg)
        {
            static const CfgEnum_S encoder            = {MDAuxEncoderValue_S::toNumeric,
                                                         MDAuxEncoderValue_S::toReadable};
            static const CfgEnum_S encoderMode        = {MDAuxEncoderModeValue_S::toNumeric,
                                                         MDAuxEncoderModeValue_S::toReadable};
            static const CfgEnum_S encoderCalibration = {
                MDAuxEncoderCalibrationModeValue_S::toNumeric,
                MDAuxEncoderCalibrationModeValue_S::toReadable};
            static const CfgEnum_S motorCalibration = {
                MDMainEncoderCalibrationModeValue_S::toNumeric,
                MDMainEncoderCalibrationModeValue_S::toReadable};
            static const CfgEnum_S gpio         = {MDUserGpioConfigurationValue_S::toNumeric,
                                                   MDUserGpioConfigurationValue_S::toReadable};
            static const CfgEnum_S torqueSensor = {MDTorqueSensorType_S::toNumeric,
                                                   MDTorqueSensorType_S::toReadable};
            switch (mdReg)
            {
                case 0x01E:
                    return &motorCalibration;
                case 0x020:
                case 0x02A:
                    return &encoder;
                case 0x025:
                    return &encoderMode;
                case 0x026:
                    return &encoderCalibration;
                case 0x160:
                    return &gpio;
                case 0x200:
                    return &torqueSensor;
                default:
                    return nullptr;
            }
        }

        // ---- CANopen units, keyed by CiA402 index ------------------------------------------

        enum CfgUnit_E
        {
            UNIT_NONE,          // same as in the file
            UNIT_MILLI,         // A -> mA, Nm -> mNm
            UNIT_POSITION,      // rad     -> 0x60A8 units
            UNIT_VELOCITY,      // rad/s   -> 0x60A9 units
            UNIT_ACCELERATION,  // rad/s^2 -> 0x60AA units
            UNIT_COUNT
        };

        CfgUnit_E unitOf(u16 coIndex)
        {
            switch (coIndex)
            {
                case 0x6075:  // motor rated current
                case 0x6076:  // motor rated torque
                    return UNIT_MILLI;
                case 0x607D:  // software position limit
                    return UNIT_POSITION;
                case 0x6080:  // max motor speed
                case 0x6081:  // profile velocity
                    return UNIT_VELOCITY;
                case 0x6083:  // profile acceleration
                case 0x6084:  // profile deceleration
                case 0x6085:  // quick stop deceleration
                case 0x60C5:  // max acceleration
                case 0x60C6:  // max deceleration
                    return UNIT_ACCELERATION;
                default:
                    return UNIT_NONE;
            }
        }

        /// @brief Max torque and max current are permille of the rated values, the file holds
        /// the limit directly, so the rated value carries it and the permille is kept at 1000
        u16 permilleOf(u16 ratedIndex)
        {
            if (ratedIndex == 0x6075)
                return 0x6073;
            if (ratedIndex == 0x6076)
                return 0x6072;
            return 0;
        }

        /// @brief Decode a CiA402 SI unit object (0x60A8..0x60AA) into the SI value of one count
        /// @param code [31:24] power of ten, [23:16] unit, [15:8] time denominator
        bool decodeSiUnit(u32 code, double& factor)
        {
            factor = std::pow(10.0, (i8)(code >> 24));
            switch ((code >> 16) & 0xFF)
            {
                case 0x10:  // radian
                    break;
                case 0x41:  // degree
                    factor *= M_PI / 180.0;
                    break;
                case 0xB4:  // revolution
                    factor *= 2.0 * M_PI;
                    break;
                default:
                    return false;
            }
            switch ((code >> 8) & 0xFF)
            {
                case 0x00:  // none
                case 0x03:  // second
                case 0x57:  // second squared
                    break;
                case 0x47:  // minute
                    factor /= 60.0;
                    break;
                default:
                    return false;
            }
            return true;
        }

        /// @brief SI value of one drive count for every unit, taken from the .eds
        bool loadScale(EDSObjectDictionary& od, double* scale, const Logger& log)
        {
            static constexpr u16 SI_UNIT_OBJECTS[] = {0x60A8, 0x60A9, 0x60AA};

            scale[UNIT_NONE]  = 1.0;
            scale[UNIT_MILLI] = 0.001;
            for (int i = 0; i < 3; i++)
            {
                const u16 index = SI_UNIT_OBJECTS[i];
                if (!od.hasEntry(index))
                {
                    log.error("SI unit object 0x%04X is missing from the .eds", index);
                    return false;
                }
                const u32 code = (u32)std::strtoul(od[index].getAsString().c_str(), nullptr, 0);
                if (!decodeSiUnit(code, scale[UNIT_POSITION + i]))
                {
                    log.error("Unsupported SI unit 0x%08X in object 0x%04X", code, index);
                    return false;
                }
            }
            return true;
        }

        // ---- Object access -----------------------------------------------------------------

        /// @brief CFG_MAP holds md_1.2 addresses, older .eds files place other objects at them
        bool isSupportedOd(EDSObjectDictionary& od, const Logger& log)
        {
            if (od.hasEntry(0x2000) &&
                md_objects::namesMatch(od[0x2000].getEntryMetaData().parameterName,
                                       "Actuator Config"))
                return true;
            log.error(
                "Motor configuration needs md_1.2 or a newer .eds, select it with "
                "'candletool mdco eds'");
            return false;
        }

        EDSEntry* findObject(EDSObjectDictionary& od, u16 index, u8 subIndex)
        {
            if (!od.hasEntry(index))
                return nullptr;
            EDSEntry& obj = od[index];
            if (subIndex == 0)
                return &obj;
            return obj.hasSubEntry(subIndex) ? &obj[subIndex] : nullptr;
        }

        bool fitsType(const EDSEntry& obj, long long value)
        {
            if (!obj.getValueMetaData().has_value())
                return false;
            const EDSEntry::DataType_E type = obj.getValueMetaData()->dataType;
            if (type == EDSEntry::DataType_E::UNSIGNED16)
                return value >= 0 && value <= UINT16_MAX;
            if (type == EDSEntry::DataType_E::UNSIGNED32)
                return value >= 0 && value <= UINT32_MAX;
            if (type == EDSEntry::DataType_E::INTEGER32)
                return value >= INT32_MIN && value <= INT32_MAX;
            return false;
        }

        bool writeObject(MDCO& mdco, EDSEntry& obj, const std::string& value, const Logger& log)
        {
            const char*       name = obj.getEntryMetaData().parameterName.c_str();
            EDSEntry::Error_t err  = EDSEntry::Error_t::PARSING_FAILED;
            try
            {
                err = obj.setFromString(value);
            }
            catch (const std::exception&)
            {
            }
            if (err != EDSEntry::Error_t::OK)
            {
                log.error("Invalid value %s for %s", value.c_str(), name);
                return false;
            }
            if (mdco.writeSDO(obj) != MDCO::Error_t::OK)
            {
                log.error("Could not write %s", name);
                return false;
            }
            log.debug("%s = %s", name, value.c_str());
            return true;
        }

        std::string formatSi(double value)
        {
            char buffer[32];
            std::snprintf(buffer, sizeof(buffer), "%.6g", value);
            return buffer;
        }

        /// @brief Closest fraction num/den to x, from the convergents of its continued fraction
        void toFraction(double x, u32& num, u32& den)
        {
            double h0 = 1.0, h1 = std::floor(x);
            double k0 = 0.0, k1 = 1.0;
            double r = x - std::floor(x);
            while (r > 1e-9 && std::fabs(h1 / k1 - x) > x * 1e-7)
            {
                r              = 1.0 / r;
                const double a = std::floor(r);
                r -= a;
                const double h2 = a * h1 + h0;
                const double k2 = a * k1 + k0;
                if (h2 > UINT32_MAX || k2 > UINT32_MAX)
                    break;
                h0 = h1;
                h1 = h2;
                k0 = k1;
                k1 = k2;
            }
            num = (u32)h1;
            den = (u32)k1;
        }

        /// @brief 0x6091 holds motor revolutions per shaft revolutions, the file holds the
        /// inverse as a float
        bool writeGearRatio(MDCO&                mdco,
                            EDSObjectDictionary& od,
                            const std::string&   text,
                            const Logger&        log)
        {
            EDSEntry*    motorRevs = findObject(od, 0x6091, 0x01);
            EDSEntry*    shaftRevs = findObject(od, 0x6091, 0x02);
            const double ratio     = std::strtod(text.c_str(), nullptr);
            u32          num = 0, den = 0;
            if (ratio > 0.0)
                toFraction(1.0 / ratio, num, den);
            if (num == 0 || den == 0 || motorRevs == nullptr || shaftRevs == nullptr)
            {
                log.error("Invalid gear ratio %s", text.c_str());
                return false;
            }
            return writeObject(mdco, *motorRevs, std::to_string(num), log) &&
                   writeObject(mdco, *shaftRevs, std::to_string(den), log);
        }

        std::optional<std::string> readGearRatio(MDCO&                mdco,
                                                 EDSObjectDictionary& od,
                                                 const Logger&        log)
        {
            EDSEntry* motorRevs = findObject(od, 0x6091, 0x01);
            EDSEntry* shaftRevs = findObject(od, 0x6091, 0x02);
            if (motorRevs == nullptr || shaftRevs == nullptr ||
                mdco.readSDO(*motorRevs) != MDCO::Error_t::OK ||
                mdco.readSDO(*shaftRevs) != MDCO::Error_t::OK)
                return std::nullopt;
            const double motor = std::strtod(motorRevs->getAsString().c_str(), nullptr);
            const double shaft = std::strtod(shaftRevs->getAsString().c_str(), nullptr);
            if (motor == 0.0)
            {
                log.error("Gear ratio motor revolutions are 0");
                return std::nullopt;
            }
            return formatSi(shaft / motor);
        }

        double readPermille(MDCO& mdco, EDSObjectDictionary& od, u16 index, const Logger& log)
        {
            EDSEntry* obj = findObject(od, index, 0);
            if (obj == nullptr || mdco.readSDO(*obj) != MDCO::Error_t::OK)
            {
                log.warn("Could not read object 0x%04X, assuming 1000 permille", index);
                return 1000.0;
            }
            return std::strtod(obj->getAsString().c_str(), nullptr);
        }

        std::string toLower(std::string text)
        {
            for (char& c : text)
                c = (char)std::tolower((unsigned char)c);
            return text;
        }
    }  // namespace

    // ---- File ------------------------------------------------------------------------------

    bool cfgLoad(const std::filesystem::path& path, CfgValues_S& cfg)
    {
        Logger             log(Logger::ProgramLayer_E::TOP, "CFG");
        mINI::INIFile      file(path.string());
        mINI::INIStructure ini;
        if (!file.read(ini))
        {
            log.error("Could not read configuration file: %s", path.string().c_str());
            return false;
        }
        // Without an output encoder the rest of its section means nothing, so it is ignored
        const std::string outputEncoder   = ini.get("output encoder").get("output encoder");
        const bool        noOutputEncoder = outputEncoder == "NONE" || outputEncoder == "0";
        for (size_t i = 0; i < CFG_MAP_SIZE; i++)
        {
            const CfgMap_S& entry = CFG_MAP[i];
            if (noOutputEncoder && entry.mdReg != 0x020 &&
                std::string_view(entry.section) == "output encoder")
                continue;
            if (ini.has(entry.section))
                cfg.value[i] = ini.get(entry.section).get(entry.key);
            if (cfg.value[i].empty())
                log.warn("Key %s.%s not found in configuration file. Skipping.",
                         entry.section,
                         entry.key);
        }
        return true;
    }

    bool cfgSave(const std::filesystem::path& path, const CfgValues_S& cfg)
    {
        Logger             log(Logger::ProgramLayer_E::TOP, "CFG");
        mINI::INIFile      file(path.string());
        mINI::INIStructure ini;
        for (size_t i = 0; i < CFG_MAP_SIZE; i++)
        {
            if (!cfg.value[i].empty())
                ini[CFG_MAP[i].section][CFG_MAP[i].key] = cfg.value[i];
        }
        if (!file.generate(ini, true))
        {
            log.error("Could not write configuration to file: %s", path.string().c_str());
            return false;
        }
        return true;
    }

    bool cfgVerify(const CfgValues_S& cfg, const mINI::INIStructure& schema)
    {
        Logger log(Logger::ProgramLayer_E::TOP, "CFG");
        bool   valid = true;
        for (size_t i = 0; i < CFG_MAP_SIZE; i++)
        {
            const CfgMap_S&    entry = CFG_MAP[i];
            const std::string& value = cfg.value[i];
            if (!schema.has(entry.section))
                continue;
            const mINI::INIMap<std::string> rules = schema.get(entry.section);
            const std::string               key   = entry.key;

            if (value.empty())
            {
                if (rules.get(key + "_required") == "true")
                {
                    log.error(
                        "%s.%s is required for proper MD operation!", entry.section, entry.key);
                    valid = false;
                }
                continue;
            }

            // enums are listed as "<key>_<number> = <name>", either of the two is accepted
            const std::string prefix    = toLower(key) + "_";
            bool              isEnum    = false;
            bool              enumMatch = false;
            for (const auto& [ruleKey, ruleValue] : rules)
            {
                if (ruleKey.rfind(prefix, 0) != 0)
                    continue;
                const std::string number = ruleKey.substr(prefix.size());
                if (number.empty() || number.find_first_not_of("0123456789") != std::string::npos)
                    continue;
                isEnum = true;
                if (value == ruleValue || value == number)
                    enumMatch = true;
            }
            if (isEnum && !enumMatch)
            {
                log.error("Invalid enum value for [%s] [%s] = %s",
                          entry.section,
                          entry.key,
                          value.c_str());
                valid = false;
            }

            const double number = std::strtod(value.c_str(), nullptr);
            if (rules.has(key + "_min") &&
                number < std::strtod(rules.get(key + "_min").c_str(), nullptr))
            {
                log.error("Value is below minimum: [%s] [%s] = %s < %s",
                          entry.section,
                          entry.key,
                          value.c_str(),
                          rules.get(key + "_min").c_str());
                valid = false;
            }
            if (rules.has(key + "_max") &&
                number > std::strtod(rules.get(key + "_max").c_str(), nullptr))
            {
                log.error("Value is above maximum: [%s] [%s] = %s > %s",
                          entry.section,
                          entry.key,
                          value.c_str(),
                          rules.get(key + "_max").c_str());
                valid = false;
            }
        }
        return valid;
    }

    // ---- Data types ------------------------------------------------------------------------

    std::optional<std::string> cfgToRaw(const CfgMap_S& entry, const std::string& text)
    {
        const CfgEnum_S* codec = enumOf(entry.mdReg);
        if (codec == nullptr || text.empty() || std::isdigit((unsigned char)text[0]))
            return text;
        const std::optional<u32> number = codec->toNumeric(text);
        if (!number.has_value())
            return std::nullopt;
        return std::to_string(number.value());
    }

    std::string cfgFromRaw(const CfgMap_S& entry, const std::string& raw)
    {
        const CfgEnum_S* codec = enumOf(entry.mdReg);
        if (codec == nullptr || raw.empty() || !std::isdigit((unsigned char)raw[0]))
            return raw;
        return codec->toReadable((u32)std::strtoul(raw.c_str(), nullptr, 10)).value_or(raw);
    }

    // ---- MDCO ------------------------------------------------------------------------------

    bool cfgUploadMdco(MDCO& mdco, EDSObjectDictionary& od, const CfgValues_S& cfg)
    {
        Logger log(Logger::ProgramLayer_E::TOP, "MDCO CFG");
        double scale[UNIT_COUNT];
        if (!isSupportedOd(od, log) || !loadScale(od, scale, log))
            return false;

        bool ok           = true;
        bool ratedCurrent = false;
        bool ratedTorque  = false;
        for (size_t i = 0; i < CFG_MAP_SIZE; i++)
        {
            const CfgMap_S& entry = CFG_MAP[i];
            if (entry.coIndex == 0 || cfg.value[i].empty())
                continue;
            EDSEntry* obj = findObject(od, entry.coIndex, entry.coSubindex);
            if (obj == nullptr)
            {
                log.warn("%s.%s has no object 0x%04X in this .eds, skipping",
                         entry.section,
                         entry.key,
                         entry.coIndex);
                continue;
            }
            const std::optional<std::string> raw = cfgToRaw(entry, cfg.value[i]);
            if (!raw.has_value())
            {
                log.error(
                    "Invalid value %s.%s = %s", entry.section, entry.key, cfg.value[i].c_str());
                ok = false;
                continue;
            }
            if (entry.coIndex == 0x6091)
            {
                if (!writeGearRatio(mdco, od, raw.value(), log))
                    ok = false;
                continue;
            }

            std::string     value = raw.value();
            const CfgUnit_E unit  = unitOf(entry.coIndex);
            if (unit != UNIT_NONE)
            {
                const long long counts =
                    std::llround(std::strtod(value.c_str(), nullptr) / scale[unit]);
                if (!fitsType(*obj, counts))
                {
                    log.error("%s.%s = %s does not fit object 0x%04X",
                              entry.section,
                              entry.key,
                              value.c_str(),
                              entry.coIndex);
                    ok = false;
                    continue;
                }
                value = std::to_string(counts);
            }
            const bool written = writeObject(mdco, *obj, value, log);
            if (!written)
                ok = false;
            if (written && entry.coIndex == 0x6075)
                ratedCurrent = true;
            if (written && entry.coIndex == 0x6076)
                ratedTorque = true;
        }

        // The drive accepts the permille limits only once both rated values are set
        EDSEntry* maxCurrent = findObject(od, permilleOf(0x6075), 0);
        EDSEntry* maxTorque  = findObject(od, permilleOf(0x6076), 0);
        if (ratedCurrent && (maxCurrent == nullptr || !writeObject(mdco, *maxCurrent, "1000", log)))
            ok = false;
        if (ratedTorque && (maxTorque == nullptr || !writeObject(mdco, *maxTorque, "1000", log)))
            ok = false;
        return ok;
    }

    bool cfgDownloadMdco(MDCO& mdco, EDSObjectDictionary& od, CfgValues_S& cfg)
    {
        Logger log(Logger::ProgramLayer_E::TOP, "MDCO CFG");
        double scale[UNIT_COUNT];
        if (!isSupportedOd(od, log) || !loadScale(od, scale, log))
            return false;

        for (size_t i = 0; i < CFG_MAP_SIZE; i++)
        {
            const CfgMap_S& entry = CFG_MAP[i];
            if (entry.coIndex == 0)
                continue;
            EDSEntry* obj = findObject(od, entry.coIndex, entry.coSubindex);
            if (obj == nullptr)
            {
                log.warn("%s.%s has no object 0x%04X in this .eds, skipping",
                         entry.section,
                         entry.key,
                         entry.coIndex);
                continue;
            }
            if (entry.coIndex == 0x6091)
            {
                cfg.value[i] = readGearRatio(mdco, od, log).value_or("");
                continue;
            }
            if (mdco.readSDO(*obj) != MDCO::Error_t::OK)
            {
                log.error("Could not read %s.%s from object 0x%04X",
                          entry.section,
                          entry.key,
                          entry.coIndex);
                continue;
            }

            std::string     raw  = obj->getAsString();
            const CfgUnit_E unit = unitOf(entry.coIndex);
            if (unit != UNIT_NONE)
            {
                double si = std::strtod(raw.c_str(), nullptr) * scale[unit];
                if (permilleOf(entry.coIndex) != 0)
                    si *= readPermille(mdco, od, permilleOf(entry.coIndex), log) / 1000.0;
                raw = formatSi(si);
            }
            cfg.value[i] = cfgFromRaw(entry, raw);
        }
        return true;
    }
}  // namespace mab
