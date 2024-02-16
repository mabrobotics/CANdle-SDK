#ifndef CONFIGPARSER_HPP
#define CONFIGPARSER_HPP

#include <algorithm>
#include <array>
#include <cctype>
#include <optional>
#include <set>
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

			if (!ini.has(field.category))
			{
				if (mandatory)
				{
					logger->error("Missing mandatory category: [{}]", field.category);
					return std::nullopt;
				}
				continue;
			}

			if (!ini[field.category].has(field.fieldName))
			{
				if (mandatory)
				{
					logger->error("Missing mandatory field: [{}][{}]", field.category, field.fieldName);
					return std::nullopt;
				}
				continue;
			}

			/* this is not very efficient, but we do no need to optimize here */
			for (auto& [index, entry] : *OD)
			{
				if (!iequals(entry->parameterName, field.category))
					continue;

				for (auto& [subindex, subentry] : entry->subEntries)
				{
					if (!iequals(subentry->parameterName, field.fieldName))
						continue;

					auto valueStr = ini.get(field.category).get(field.fieldName);

					/* parse special text values */
					auto specialKey = toLower(field.category) + toLower(field.fieldName);
					if (specialTreatmentMap.contains(specialKey))
					{
						if (!specialTreatmentMap[specialKey].contains(valueStr))
						{
							logger->error("Unrecognised value \"{}\" for [{}]:[{}]", valueStr, field.category, field.fieldName);

							std::string output = "Possible values:";
							for (auto& [str, val] : specialTreatmentMap[specialKey])
								output.append(std::string(" ") + str);

							logger->info(output);
							return std::nullopt;
						}
						else
							valueStr = specialTreatmentMap[specialKey][valueStr];
					}

					try
					{
						subentry->value = ObjectDictionaryParserEDS::fillValue(valueStr, subentry->dataType);
					}
					catch (...)
					{
						logger->error("Exception while parsing [{}]:[{}]. Please check the value and try one more time. ", entry->parameterName, subentry->parameterName);
						if (mandatory)
							return std::nullopt;
					}

					ODindexes.push_back({index, subindex});
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

	std::string toLower(std::string str)
	{
		std::transform(str.begin(), str.end(), str.begin(),
					   [](unsigned char c)
					   { return std::tolower(c); });

		return str;
	}

   private:
	mINI::INIStructure ini;
	std::shared_ptr<spdlog::logger> logger;
	IODParser::ODType* OD;

	std::unordered_map<std::string, std::string> encoderTypes = {
		{"NONE", "0"},
		{"AS5047_CENTER", "1"},
		{"AS5047_OFFAXIS", "2"},
		{"MB053SFA17BENT00", "3"},
		{"CM_OFFAXIS", "4"},
		{"M24B_CENTER", "5"},
		{"M24B_OFFAXIS", "6"}};

	std::unordered_map<std::string, std::string> encoderModes = {
		{"NONE", "0"},
		{"STARTUP", "1"},
		{"MOTION", "2"},
		{"REPORT", "3"},
		{"MAIN", "4"}};

	std::unordered_map<std::string, std::string> encoderCalibrationModes = {
		{"FULL", "0"},
		{"DIRONLY", "1"}};

	std::unordered_map<std::string, std::string> motorCalibrationModes = {
		{"FULL", "0"},
		{"NOPPDET", "1"}};

	std::unordered_map<std::string, std::string> homingModes = {
		{"OFF", "0"},
		{"SENSORLESS", "1"}};

	std::unordered_map<std::string, std::string> brakeModes = {
		{"OFF", "0"},
		{"AUTO", "1"},
		{"MANUAL", "2"}};

	std::unordered_map<std::string, std::unordered_map<std::string, std::string>> specialTreatmentMap{
		{"output encodertype", encoderTypes},
		{"output encodermode", encoderModes},
		{"output encodercalibration mode", encoderCalibrationModes},
		{"motor settingscalibration mode", motorCalibrationModes},
		{"homingmode", homingModes},
		{"brakemode", brakeModes}};

	std::array<ConfigField, 100> Fields{
		ConfigField{"motor settings", "motor name", true},
		ConfigField{"motor settings", "pole pairs", true},
		ConfigField{"motor settings", "torque constant", true},
		ConfigField{"motor settings", "KV", false},
		ConfigField{"motor settings", "gear ratio", true},
		ConfigField{"motor settings", "torque bandwidth", true},
		ConfigField{"motor settings", "motor shutdown temperature", true},
		ConfigField{"motor settings", "calibration mode", false},

		ConfigField{"limits", "max torque", true},
		ConfigField{"limits", "max velocity", true},
		ConfigField{"limits", "max position", true},
		ConfigField{"limits", "min position", true},
		ConfigField{"limits", "max acceleration", true},
		ConfigField{"limits", "max deceleration", true},
		ConfigField{"limits", "max current", true},

		ConfigField{"motion", "profile velocity", true},
		ConfigField{"motion", "profile acceleration", true},
		ConfigField{"motion", "profile deceleration", true},
		ConfigField{"motion", "position window", false},
		ConfigField{"motion", "velocity window", false},
		ConfigField{"motion", "quick stop deceleration", false},

		ConfigField{"output encoder", "type", false},
		ConfigField{"output encoder", "mode", false},
		ConfigField{"output encoder", "calibration mode", false},

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

		ConfigField{"homing", "mode", false},
		ConfigField{"homing", "max travel", false},
		ConfigField{"homing", "max velocity", false},
		ConfigField{"homing", "max torque", false},

		ConfigField{"brake", "mode", false}};
};

#endif