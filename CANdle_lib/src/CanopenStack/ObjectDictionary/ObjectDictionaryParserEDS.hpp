#ifndef OBJECTDICTIONARYPARSEREDS_HPP
#define OBJECTDICTIONARYPARSEREDS_HPP

#include <map>
#include <memory>
#include <string>

#include "IObjectDictionaryParser.hpp"
#include "third_party/mINI/inc/ini.h"

class ObjectDictionaryParserEDS : public IObjectDictionaryParser
{
   public:
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

			auto objectType = ini[key]["objecttype"];
			auto dataType = ini[key]["datatype"];

			entry.second->parameterName = ini[key]["parametername"];
			entry.second->objectType = static_cast<IObjectDictionaryParser::ObjectType>(strtol(objectType.data(), nullptr, 0));
			entry.second->datatype = static_cast<IObjectDictionaryParser::DataType>(strtol(dataType.data(), nullptr, 0));
		}
	}
};

#endif