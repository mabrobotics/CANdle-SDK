#ifndef CONFIGPARSER_HPP
#define CONFIGPARSER_HPP

#include <algorithm>
#include <array>
#include <cctype>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_set>
#include <variant>
#include <vector>

#include "IObjectDictionaryParser.hpp"
#include "ObjectDictionaryParserEDS.hpp"
#include "ini.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

class ConfigParser
{
	struct ConfigField
	{
		std::string category;
		std::string fieldName;
		bool mandatory = false;
	};

   public:
	ConfigParser(std::shared_ptr<spdlog::logger> logger, IODParser::ODType* OD) : logger(logger), OD(OD)
	{
	}

	bool openFile(const std::string& path)
	{
		mINI::INIFile file(path);
		if (!file.read(ini))
			return false;
		return true;
	}

	std::optional<std::vector<std::pair<uint16_t, uint8_t>>> parseFile()
	{
		std::vector<std::pair<uint16_t, uint8_t>> ODindexes{};

		for (auto& field : Fields)
		{
			bool mandatory = field.mandatory;

			if (mandatory && !ini.has(field.category))
			{
				logger->error("Missing mandatory category: [{}]", field.category);
				return std::nullopt;
			}

			if (mandatory && !ini[field.category].has(field.fieldName))
			{
				logger->error("Missing mandatory field: [{}] {}", field.category, field.fieldName);
				return std::nullopt;
			}

			/* this is not very efficient, but we do no need to optimize here */
			for (auto& [index, entry] : *OD)
			{
				if (iequals(entry->parameterName, field.category))
				{
					logger->debug("Category matched!");
					for (auto& [subindex, subentry] : entry->subEntries)
					{
						if (iequals(subentry->parameterName, field.fieldName))
						{
							logger->debug("Field matched!");
							auto valueStr = ini.get(field.category).get(field.fieldName);

							try
							{
								subentry->value = ObjectDictionaryParserEDS::fillDefaultValue(valueStr, subentry->dataType);
							}
							catch (...)
							{
								logger->warn("exception while parsing 0x{:x}:0x{:x} ({}:{})", index, subindex, entry->parameterName, subentry->parameterName);
							}

							ODindexes.push_back({index, subindex});
						}
					}
				}
			}
		}

		return ODindexes;
	}

   private:
	bool iequals(const std::string& a, const std::string& b)
	{
		return a.size() == b.size() && std::equal(a.begin(), a.end(), b.begin(),
												  [](char a, char b) -> bool
												  { return std::tolower(static_cast<unsigned char>(a)) ==
														   std::tolower(static_cast<unsigned char>(b)); });
	}

   private:
	mINI::INIStructure ini;
	std::shared_ptr<spdlog::logger> logger;
	IODParser::ODType* OD;

	std::array<ConfigField, 100> Fields{
		ConfigField{"motor settings", "motor name", true},
		ConfigField{"motor settings", "pole pairs", true},
		ConfigField{"motor settings", "torque constant", true},
		ConfigField{"motor settings", "KV", false},
		ConfigField{"motor settings", "gear ratio", true},
		ConfigField{"motor settings", "torque bandwidth", true},
		ConfigField{"motor settings", "motor shutdown temperature", true},

		ConfigField{"limits", "max torque", true},
		ConfigField{"limits", "max velocity", true},
		ConfigField{"limits", "max position", true},
		ConfigField{"limits", "min position", true},
		ConfigField{"limits", "max acceleration", true},
		ConfigField{"limits", "min deceleration", true},
		ConfigField{"limits", "max current", true},

		ConfigField{"motion", "profile velocity", true},
		ConfigField{"motion", "profile acceleration", true},
		ConfigField{"motion", "profile deceleration", true},
		ConfigField{"motion", "position window", false},
		ConfigField{"motion", "velocity window", false},
		ConfigField{"motion", "quick stop deceleration", false},

		ConfigField{"output encoder", "type", true},
		ConfigField{"output encoder", "mode", true},

		ConfigField{"position pid controller", "kp", true},
		ConfigField{"position pid controller", "ki", true},
		ConfigField{"position pid controller", "kd", true},
		ConfigField{"position pid controller", "integral limit", true},

		ConfigField{"velocity pid controller", "kp", true},
		ConfigField{"velocity pid controller", "ki", true},
		ConfigField{"velocity pid controller", "kd", true},
		ConfigField{"velocity pid controller", "integral limit", true},

		ConfigField{"impedance pd controller", "kp", true},
		ConfigField{"impedance pd controller", "kd", true},
	};
};

#endif