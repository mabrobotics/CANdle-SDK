#pragma once

#include "md_types.hpp"

#include <map>
#include <string>
#include <memory>

namespace mab
{
    struct MDCfgElement
    {
        const std::string m_regName;
        const std::string m_tomlSection;
        const std::string m_tomlKey;

        MDCfgElement(std::string regName, std::string section, std::string key)
            : m_regName(std::move(regName)),
              m_tomlSection(std::move(section)),
              m_tomlKey(std::move(key))
        {
        }
    }

    class MDConfigMap
    {
      public:
        MDConfigMap()  = default;
        ~MDConfigMap() = default;

        std::optional<std::string> getSectionByRegName(const std::string& regName) const
        {
            auto it = m_map.find(regName);
            if (it != m_map.end())
            {
                return it->second.m_tomlSection;
            }
            return std::nullopt;
        }

      private:
        MDRegisters_S m_registers;
    };
}  // namespace mab