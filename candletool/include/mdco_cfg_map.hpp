#include <map>
#include <string>
#include <stdexcept>
#include <iostream>
#include <cstring>

#include "mab_types.hpp"

enum class MDCOValueType
{
    INT,
    FLOAT,
    STRING
};

struct MDCOCfgElement
{
    std::string   Section;
    std::string   Key;
    MDCOValueType Type;

    MDCOCfgElement(std::string section, std::string key, MDCOValueType type)
        : Section(std::move(section)), Key(std::move(key)), Type(type)
    {
    }
};

class MDCOConfigMap
{
  public:
    MDCOConfigMap() = default;

    std::map<std::pair<u16, u8>, MDCOCfgElement> m_map{
        {{0x2000, 0x06}, MDCOCfgElement("motor", "name", MDCOValueType::STRING)},
        {{0x2000, 0x01}, MDCOCfgElement("motor", "pole pairs", MDCOValueType::INT)},
        {{0x0000, 0x00}, MDCOCfgElement("motor", "KV", MDCOValueType::INT)},
        {{0x2000, 0x02}, MDCOCfgElement("motor", "torque constant", MDCOValueType::FLOAT)},
        {{0x2000, 0x08}, MDCOCfgElement("motor", "gear ratio", MDCOValueType::FLOAT)},
        {{0x2000, 0x05}, MDCOCfgElement("motor", "torque bandwidth", MDCOValueType::INT)},

        {{0x6076, 0x00}, MDCOCfgElement("limits", "rated torque", MDCOValueType::INT)},
        {{0x6072, 0x00}, MDCOCfgElement("limits", "max torque", MDCOValueType::INT)},
        {{0x6075, 0x00}, MDCOCfgElement("limits", "rated current", MDCOValueType::INT)},
        {{0x6073, 0x00}, MDCOCfgElement("limits", "max current", MDCOValueType::INT)},
        {{0x6080, 0x00}, MDCOCfgElement("limits", "max velocity", MDCOValueType::INT)},
        {{0x607D, 0x02}, MDCOCfgElement("limits", "max position", MDCOValueType::INT)},
        {{0x607D, 0x01}, MDCOCfgElement("limits", "min position", MDCOValueType::INT)},
        {{0x60C5, 0x00}, MDCOCfgElement("limits", "max acceleration", MDCOValueType::INT)},
        {{0x60C6, 0x00}, MDCOCfgElement("limits", "max deceleration", MDCOValueType::INT)},

        {{0x6083, 0x00}, MDCOCfgElement("profile", "acceleration", MDCOValueType::INT)},
        {{0x6084, 0x00}, MDCOCfgElement("profile", "deceleration", MDCOValueType::INT)},
        {{0x6081, 0x00}, MDCOCfgElement("profile", "velocity", MDCOValueType::INT)},

        {{0x2005, 0x01}, MDCOCfgElement("output encoder", "output encoder", MDCOValueType::INT)},
        {{0x2005, 0x02},
         MDCOCfgElement("output encoder", "output encoder mode", MDCOValueType::INT)},

        {{0x2002, 0x01}, MDCOCfgElement("position pid", "kp", MDCOValueType::FLOAT)},
        {{0x2002, 0x02}, MDCOCfgElement("position pid", "ki", MDCOValueType::FLOAT)},
        {{0x2002, 0x03}, MDCOCfgElement("position pid", "kd", MDCOValueType::FLOAT)},
        {{0x2002, 0x04}, MDCOCfgElement("position pid", "windup", MDCOValueType::FLOAT)},

        {{0x2001, 0x01}, MDCOCfgElement("velocity pid", "kp", MDCOValueType::FLOAT)},
        {{0x2001, 0x02}, MDCOCfgElement("velocity pid", "ki", MDCOValueType::FLOAT)},
        {{0x2001, 0x03}, MDCOCfgElement("velocity pid", "kd", MDCOValueType::FLOAT)},
        {{0x2001, 0x04}, MDCOCfgElement("velocity pid", "windup", MDCOValueType::FLOAT)},

        {{0x200C, 0x01}, MDCOCfgElement("impedance pd", "kp", MDCOValueType::FLOAT)},
        {{0x200C, 0x02}, MDCOCfgElement("impedance pd", "kd", MDCOValueType::FLOAT)},
    };
};
