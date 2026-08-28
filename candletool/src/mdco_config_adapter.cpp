#include "candletool/mdco_config_adapter.hpp"

namespace mab
{
    std::vector<std::reference_wrapper<EDSEntry>> MDCOConfigAdapter::configToOd(
        MDConfigMap& config, std::shared_ptr<EDSObjectDictionary> od)
    {
        std::vector<std::reference_wrapper<EDSEntry>> result;
        Logger                                        log(Logger::ProgramLayer_E::TOP, "MDCO CFG");
        // Manufacturer part parsing
        for (const auto& [regAddr, objName, subIdx] : manufacturerRegMaping)
        {
            const std::string cfgValue =
                cfgToOdUnitConversions.find(regAddr) != cfgToOdUnitConversions.end()
                    ? cfgToOdUnitConversions.at(regAddr)(config.getValueByAddress(regAddr))
                    : config.getValueByAddress(regAddr);
            if (cfgValue.empty())
            {
                log.warn("Manufacturer register %s not found in config", objName.data());
                continue;
            }

            auto objOpt = od->getEntryByName(objName);
            if (!objOpt.has_value())
            {
                log.warn("Manufacturer register %s not found in OD - skipping", objName.data());
                continue;
            }
            EDSEntry& obj =
                subIdx.has_value() ? objOpt.value().get()[subIdx.value()] : objOpt.value().get();
            if (obj.setFromString(cfgValue) != EDSEntry::Error_t::OK)
            {
                log.error("Failed to set value for manufacturer register %s", objName.data());
                continue;
            }
            log.debug("%s = %s", obj.getEntryMetaData().parameterName.c_str(), cfgValue.c_str());
            result.push_back(std::reference_wrapper<EDSEntry>(obj));
        }

        // Parse standard CANopen registers with unit conversion
        for (const auto& [cfgAddr, odAddr, subIdx] : standardRegMaping)
        {
            const std::string cfgValue =
                cfgToOdUnitConversions.find(cfgAddr) != cfgToOdUnitConversions.end()
                    ? cfgToOdUnitConversions.at(cfgAddr)(config.getValueByAddress(cfgAddr))
                    : config.getValueByAddress(cfgAddr);
            if (cfgValue.empty())
            {
                log.warn("402 standard object 0x%x of the MD not found in config", odAddr);
                continue;
            }
            EDSEntry& obj = subIdx.has_value() ? (*od)[odAddr][subIdx.value()] : (*od)[odAddr];
            if (obj.setFromString(cfgValue) != EDSEntry::Error_t::OK)
            {
                log.error("Failed to set value for manufacturer register %s",
                          obj.getEntryMetaData().parameterName.c_str());
                continue;
            }
            log.debug("%s = %s = %s",
                      obj.getEntryMetaData().parameterName.c_str(),
                      cfgValue.c_str(),
                      obj.getAsString().c_str());
            result.push_back(obj);
        }

        return result;
    }
    void MDCOConfigAdapter::configFromOd(std::shared_ptr<EDSObjectDictionary> od,
                                         MDConfigMap&                         config)
    {
        Logger log(Logger::ProgramLayer_E::TOP, "MDCO CFG");
        for (const auto& [regAddr, objName, subIdx] : manufacturerRegMaping)
        {
            auto objOpt = od->getEntryByName(objName);
            if (!objOpt.has_value())
            {
                log.warn("Manufacturer register %s not found in OD - skipping", objName.data());
                continue;
            }
            EDSEntry& obj =
                subIdx.has_value() ? objOpt.value().get()[subIdx.value()] : objOpt.value().get();
            config.setValueByAddress(
                regAddr,
                odToCfgUnitConversions.find(regAddr) != odToCfgUnitConversions.end()
                    ? odToCfgUnitConversions.at(regAddr)(obj.getAsString())
                    : obj.getAsString());

            log.debug("%s = %s = %s",
                      obj.getEntryMetaData().parameterName.c_str(),
                      (odToCfgUnitConversions.find(regAddr) != odToCfgUnitConversions.end()
                           ? odToCfgUnitConversions.at(regAddr)(obj.getAsString())
                           : obj.getAsString())
                          .c_str(),
                      obj.getAsString().c_str());
        }
        for (const auto& [regAddr, objAddress, subIdx] : standardRegMaping)
        {
            EDSEntry& obj =
                subIdx.has_value() ? (*od)[objAddress][subIdx.value()] : (*od)[objAddress];
            config.setValueByAddress(
                regAddr,
                odToCfgUnitConversions.find(regAddr) != odToCfgUnitConversions.end()
                    ? odToCfgUnitConversions.at(regAddr)(obj.getAsString())
                    : obj.getAsString());
            log.debug("%s = %s = %s",
                      obj.getEntryMetaData().parameterName.c_str(),
                      (odToCfgUnitConversions.find(regAddr) != odToCfgUnitConversions.end()
                           ? odToCfgUnitConversions.at(regAddr)(obj.getAsString())
                           : obj.getAsString())
                          .c_str(),
                      obj.getAsString().c_str());
        }
        return;
    }
}  // namespace mab
