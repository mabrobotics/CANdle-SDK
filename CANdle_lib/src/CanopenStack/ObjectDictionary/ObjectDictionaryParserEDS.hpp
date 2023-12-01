#ifndef OBJECTDICTIONARYPARSEREDS_HPP
#define OBJECTDICTIONARYPARSEREDS_HPP

#include <map>
#include <memory>
#include <string>

#include "IObjectDictionaryParser.hpp"
#include "third_party/mINI/inc/ini.h"

class ObjectDictionaryParserEDS : public IODParser
{
   public:
	const std::map<std::string, IODParser::AccessSDO> strToAccessType = {
		{"no", IODParser::AccessSDO::no}, {"ro", IODParser::AccessSDO::ro}, {"wo", IODParser::AccessSDO::wo}, {"rw", IODParser::AccessSDO::rw}};

	bool parseFile(const std::string& filePath, std::map<uint32_t, std::shared_ptr<Entry>>& objectDictionary) override
	{
		mINI::INIFile file(filePath);
		mINI::INIStructure ini;
		file.read(ini);

		auto insertEntires = [&](const char* category)
		{
			for (size_t i = 1; i < 200; i++)
			{
				std::string a = ini.get(category).get(std::to_string(i));

				if (a.size() < 4)
					break;

				objectDictionary.insert({strtol(a.data(), nullptr, 0), std::make_shared<Entry>()});
			}
		};

		insertEntires("mandatoryobjects");
		insertEntires("optionalobjects");

		for (auto& entry : objectDictionary)
		{
			std::stringstream stream;
			stream << std::hex << entry.first;
			std::string key = stream.str();

			auto access = ini[key]["accesstype"];
			IODParser::AccessSDO accessType = strToAccessType.count(key) ? strToAccessType.at(access) : IODParser::AccessSDO::no;

			entry.second->parameterName = ini[key]["parametername"];
			entry.second->objectType = static_cast<IODParser::ObjectType>(strtol(ini[key]["objecttype"].data(), nullptr, 0));
			entry.second->datatype = static_cast<IODParser::DataType>(strtol(ini[key]["datatype"].data(), nullptr, 0));
			entry.second->accessType = accessType;
		}
	}
};

#endif