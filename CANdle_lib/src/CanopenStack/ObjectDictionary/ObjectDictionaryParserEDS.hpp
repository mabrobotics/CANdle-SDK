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

		auto stringToInt = [](std::string str) -> int32_t
		{
			return strtol(str.data(), nullptr, 0);
		};

		auto insertEntires = [&](const char* category)
		{
			size_t i = 0;
			while (1)
			{
				if (!ini.get(category).has(std::to_string(++i)))
					break;

				std::string key = ini.get(category).get(std::to_string(i));
				objectDictionary.insert({stringToInt(key), std::make_shared<Entry>()});
			}
		};

		auto fillInEntryFields = [&](std::string key, Entry& entry)
		{
			auto access = ini[key]["accesstype"];
			IODParser::AccessSDO accessType = strToAccessType.count(key) ? strToAccessType.at(access) : IODParser::AccessSDO::no;

			entry.parameterName = ini[key]["parametername"];
			entry.objectType = static_cast<IODParser::ObjectType>(strtol(ini[key]["objecttype"].data(), nullptr, 0));
			entry.datatype = static_cast<IODParser::DataType>(strtol(ini[key]["datatype"].data(), nullptr, 0));
			entry.accessType = accessType;
		};

		insertEntires("mandatoryobjects");
		insertEntires("optionalobjects");
		insertEntires("manufacturerobjects");

		for (auto& entry : objectDictionary)
		{
			auto key = entry.first;

			std::stringstream stream;
			stream << std::hex << key;
			std::string keyString = stream.str();

			fillInEntryFields(keyString, *entry.second);

			if (entry.second->objectType == IODParser::ObjectType::ARRAY || entry.second->objectType == IODParser::ObjectType::REC)
			{
				size_t i = 0;
				while (1)
				{
					std::stringstream subIdx;
					subIdx << std::hex << i;

					auto subkey = keyString + "sub" + subIdx.str();
					if (!ini.has(subkey))
						break;

					objectDictionary[key]->subEntries.insert({i, std::make_shared<Entry>()});
					fillInEntryFields(subkey, *objectDictionary[key]->subEntries[i]);
					i++;
				}
			}
		}

		return false;
	}
};

#endif