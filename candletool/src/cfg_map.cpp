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
                case 0x6067:  // position window
                case 0x607D:  // software position limit
                    return UNIT_POSITION;
                case 0x606D:  // velocity window
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

        /// @brief Max current and max torque are permille of the rated values, the file holds
        /// both in SI
        /// @return rated object of a limit object, 0 for other objects
        u16 ratedOf(u16 limitIndex)
        {
            if (limitIndex == 0x6073)
                return 0x6075;
            if (limitIndex == 0x6072)
                return 0x6076;
            return 0;
        }

        size_t rowOf(u16 coIndex)
        {
            for (size_t i = 0; i < CFG_MAP_SIZE; i++)
            {
                if (CFG_MAP[i].coIndex == coIndex)
                    return i;
            }
            return CFG_MAP_SIZE;
        }

        /// @brief SI value of one drive count for every unit, taken from the .eds
        bool loadScale(EDSObjectDictionary& od, double* scale, const Logger& log)
        {
            scale[UNIT_NONE]  = 1.0;
            scale[UNIT_MILLI] = 0.001;
            return md_objects::siUnitScale(
                       od, md_objects::SI_UNIT_POSITION, scale[UNIT_POSITION], log) &&
                   md_objects::siUnitScale(
                       od, md_objects::SI_UNIT_VELOCITY, scale[UNIT_VELOCITY], log) &&
                   md_objects::siUnitScale(
                       od, md_objects::SI_UNIT_ACCELERATION, scale[UNIT_ACCELERATION], log);
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

        /// @brief Write one object and print it next to the file value it came from
        /// @param source "section.key = value" as in the file, empty for a value derived from
        /// the line above
        /// @param isDefault the file value is a default from CFG_MAP
        bool writeObject(MDCO&              mdco,
                         EDSEntry&          obj,
                         const std::string& value,
                         const std::string& source,
                         bool               isDefault,
                         const Logger&      log)
        {
            const EDSEntry::EDSEntryMetaData& meta = obj.getEntryMetaData();
            const char*                       name = meta.parameterName.c_str();
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
            char address[16];
            std::snprintf(address,
                          sizeof(address),
                          "0x%04X:%02X ",
                          meta.address.first,
                          meta.address.second.value_or(0));
            cfgPrintWrite(log, source, isDefault, address + meta.parameterName, value);
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
                            const std::string&   source,
                            bool                 isDefault,
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
            return writeObject(mdco, *motorRevs, std::to_string(num), source, isDefault, log) &&
                   writeObject(mdco, *shaftRevs, std::to_string(den), "", isDefault, log);
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

        /// @brief Write the rated value and the limit as permille of it. A rated value that is
        /// not set is replaced by the limit, the permille is then 1000
        /// @param limitRow CFG_MAP row of max current or max torque
        bool writeLimit(MDCO&                mdco,
                        EDSObjectDictionary& od,
                        const CfgValues_S&   cfg,
                        size_t               limitRow,
                        const Logger&        log)
        {
            const CfgMap_S&    limit      = CFG_MAP[limitRow];
            const size_t       ratedRow   = rowOf(ratedOf(limit.coIndex));
            const CfgMap_S&    rated      = CFG_MAP[ratedRow];
            const std::string& limitText  = cfg.value[limitRow];
            const std::string  limitSource =
                std::string(limit.section) + "." + limit.key + " = " + limitText;
            std::string ratedText   = cfg.value[ratedRow];
            std::string ratedSource =
                std::string(rated.section) + "." + rated.key + " = " + ratedText;
            bool ratedIsDefault = cfg.isDefault[ratedRow];
            if (!cfgIsSet(rated, ratedText))
            {
                log.warn("%s.%s is not set, %s.%s is used as the rated value",
                         rated.section,
                         rated.key,
                         limit.section,
                         limit.key);
                ratedText      = limitText;
                ratedSource    = limitSource;
                ratedIsDefault = true;
            }

            EDSEntry* ratedObj = findObject(od, rated.coIndex, 0);
            EDSEntry* limitObj = findObject(od, limit.coIndex, 0);
            if (ratedObj == nullptr || limitObj == nullptr)
            {
                log.error("Objects 0x%04X and 0x%04X are missing from the .eds",
                          rated.coIndex,
                          limit.coIndex);
                return false;
            }
            const double    ratedSi  = std::strtod(ratedText.c_str(), nullptr);
            const double    limitSi  = std::strtod(limitText.c_str(), nullptr);
            const long long counts   = std::llround(ratedSi * 1000.0);  // mA, mNm
            const long long permille = counts > 0 ? std::llround(limitSi * 1000.0 / ratedSi) : 0;
            if (counts <= 0 || !fitsType(*ratedObj, counts) || !fitsType(*limitObj, permille))
            {
                log.error("%s.%s = %s does not fit object 0x%04X as permille of %s",
                          limit.section,
                          limit.key,
                          limitText.c_str(),
                          limit.coIndex,
                          ratedText.c_str());
                return false;
            }
            // the drive accepts the permille limit only once the rated value is set
            return writeObject(
                       mdco, *ratedObj, std::to_string(counts), ratedSource, ratedIsDefault, log) &&
                   writeObject(mdco,
                               *limitObj,
                               std::to_string(permille),
                               ratedIsDefault ? "" : limitSource,
                               cfg.isDefault[limitRow],
                               log);
        }

        std::optional<double> readNumber(MDCO& mdco, EDSObjectDictionary& od, u16 index)
        {
            EDSEntry* obj = findObject(od, index, 0);
            if (obj == nullptr || mdco.readSDO(*obj) != MDCO::Error_t::OK)
                return std::nullopt;
            return std::strtod(obj->getAsString().c_str(), nullptr);
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
        const bool        noOutputEncoder = outputEncoder.empty() || outputEncoder == "NONE" ||
                                     outputEncoder == "0";
        bool ok = true;
        for (size_t i = 0; i < CFG_MAP_SIZE; i++)
        {
            const CfgMap_S& entry = CFG_MAP[i];
            if (noOutputEncoder && entry.mdReg != 0x020 &&
                std::string_view(entry.section) == "output encoder")
                continue;
            if (ini.has(entry.section))
                cfg.value[i] = ini.get(entry.section).get(entry.key);
            if (!cfg.value[i].empty())
                continue;
            if (entry.flags & CFG_CRITICAL)
            {
                log.error("Key %s.%s is required", entry.section, entry.key);
                ok = false;
            }
            else if (entry.defaultValue != nullptr)
            {
                // reported when it is written, see cfgPrintWrite
                cfg.value[i]     = entry.defaultValue;
                cfg.isDefault[i] = true;
            }
            else
                log.warn("Key %s.%s not found in configuration file. Skipping.",
                         entry.section,
                         entry.key);
        }

        // The drive needs KV or the torque constant, 0 leaves one of them unset
        bool hasMotorConstant = false;
        for (size_t i = 0; i < CFG_MAP_SIZE; i++)
        {
            if ((CFG_MAP[i].mdReg == 0x01D || CFG_MAP[i].mdReg == 0x012) &&
                cfgIsSet(CFG_MAP[i], cfg.value[i]))
                hasMotorConstant = true;
        }
        if (!hasMotorConstant)
        {
            log.error("Both motor.KV and motor.torque constant are 0, set at least one of them");
            ok = false;
        }
        return ok;
    }

    bool cfgIsSet(const CfgMap_S& entry, const std::string& value)
    {
        if (value.empty())
            return false;
        if (!(entry.flags & CFG_SKIP_ZERO))
            return true;
        // text that is not a number is passed on, so writing it reports the error
        char*        end    = nullptr;
        const double number = std::strtod(value.c_str(), &end);
        return *end != '\0' || number != 0.0;
    }

    void cfgPrintWrite(const Logger&      log,
                       const std::string& source,
                       bool               isDefault,
                       const std::string& target,
                       const std::string& value)
    {
        if (isDefault)
            log.info(YELLOW "%-50s -> %s = %s [WARN - default]" RESETCLR,
                     source.c_str(),
                     target.c_str(),
                     value.c_str());
        else
            log.info("%-50s -> %s = %s", source.c_str(), target.c_str(), value.c_str());
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

            // missing keys are handled by cfgLoad, from the flags in CFG_MAP
            if (value.empty())
                continue;

            // enums come from MD_strings.hpp, a name, an alias or a known number is accepted
            const CfgEnum_S*   codec  = enumOf(entry.mdReg);
            std::optional<u32> enumValue = std::nullopt;
            if (codec != nullptr)
                enumValue = codec->toNumeric(value);
            if (codec != nullptr &&
                (!enumValue.has_value() || !codec->toReadable(enumValue.value()).has_value()))
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

        bool ok = true;
        for (size_t i = 0; i < CFG_MAP_SIZE; i++)
        {
            const CfgMap_S& entry = CFG_MAP[i];
            // rated values are written together with their limit
            if (entry.coIndex == 0 || entry.coIndex == 0x6075 || entry.coIndex == 0x6076 ||
                !cfgIsSet(entry, cfg.value[i]))
                continue;
            if (ratedOf(entry.coIndex) != 0)
            {
                if (!writeLimit(mdco, od, cfg, i, log))
                    ok = false;
                continue;
            }
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
            const std::string source =
                std::string(entry.section) + "." + entry.key + " = " + cfg.value[i];
            if (entry.coIndex == 0x6091)
            {
                if (!writeGearRatio(mdco, od, raw.value(), source, cfg.isDefault[i], log))
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
            if (!writeObject(mdco, *obj, value, source, cfg.isDefault[i], log))
                ok = false;
        }
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
                raw = formatSi(std::strtod(raw.c_str(), nullptr) * scale[unit]);
            if (ratedOf(entry.coIndex) != 0)
            {
                const std::optional<double> rated = readNumber(mdco, od, ratedOf(entry.coIndex));
                if (!rated.has_value())
                {
                    log.error("Could not read %s.%s, object 0x%04X is not readable",
                              entry.section,
                              entry.key,
                              ratedOf(entry.coIndex));
                    continue;
                }
                raw = formatSi(std::strtod(raw.c_str(), nullptr) / 1000.0 * rated.value() *
                               scale[UNIT_MILLI]);
            }
            cfg.value[i] = cfgFromRaw(entry, raw);
        }
        return true;
    }
}  // namespace mab
