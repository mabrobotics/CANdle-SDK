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

			for (auto& [index, entry] : *OD)
			{
				if (iequals(entry->parameterName, field.category))
				{
					logger->info("Category matched!");

					for (auto& [subindex, subentry] : entry->subEntries)
					{
						if (iequals(subentry->parameterName, field.fieldName))
						{
							logger->info("Field matched!");
							auto valueStr = ini.get(field.category).get(field.fieldName);
							subentry->value = ObjectDictionaryParserEDS::fillDefaultValue(valueStr, subentry->dataType);
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

	std::array<ConfigField, 2> Fields{
		ConfigField{"motor settings", "motor name", true},
		ConfigField{"motor settings", "pole pairs", true}};
};

#endif