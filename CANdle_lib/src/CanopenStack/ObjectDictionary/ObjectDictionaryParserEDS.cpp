#include "ObjectDictionaryParserEDS.hpp"

#include <filesystem>

bool ObjectDictionaryParserEDS::parseFile(const std::string& filePath, ODType& objectDictionary)
{
	if (!std::filesystem::exists(filePath))
		return false;

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
			objectDictionary.emplace(stringToInt(key), std::move(std::make_unique<Entry>()));
		}
	};

	auto fillInEntryFields = [&](std::string key, Entry& entry)
	{
		auto access = ini[key]["accesstype"];
		IODParser::AccessSDO accessType = strToAccessType.count(key) ? strToAccessType.at(access) : IODParser::AccessSDO::no;

		entry.parameterName = ini[key]["parametername"];
		entry.objectType = static_cast<IODParser::ObjectType>(strtol(ini[key]["objecttype"].data(), nullptr, 0));
		entry.dataType = static_cast<IODParser::DataType>(strtol(ini[key]["datatype"].data(), nullptr, 0));
		entry.accessType = accessType;
		auto defaultValueStr = ini[key]["defaultvalue"];
		if (!defaultValueStr.empty())
			entry.value = fillValue(defaultValueStr, entry.dataType);

		auto lowLimit = ini[key]["lowlimit"];
		if (!lowLimit.empty())
			entry.lowLimit = fillValue(lowLimit, entry.dataType);

		auto highLimit = ini[key]["highlimit"];
		if (!highLimit.empty())
			entry.highLimit = fillValue(highLimit, entry.dataType);
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

				objectDictionary[key]->subEntries.emplace(i, std::move(std::make_unique<Entry>()));
				fillInEntryFields(subkey, *objectDictionary[key]->subEntries[i]);
				i++;
			}
		}
	}

	return true;
}

IODParser::ValueType ObjectDictionaryParserEDS::fillValue(std::string& value, IODParser::DataType dataType)
{
	switch (dataType)
	{
		case IODParser::DataType::REAL32:
			return std::stof(value);
		case IODParser::DataType::BOOLEAN:
		{
			if (value == "true")
				return true;
			else
				return false;
		}
		case IODParser::DataType::UNSIGNED8:
			return static_cast<uint8_t>(std::stoi(value, nullptr, 0));
		case IODParser::DataType::INTEGER8:
			return static_cast<int8_t>(std::stoi(value, nullptr, 0));
		case IODParser::DataType::UNSIGNED16:
			return static_cast<uint16_t>(std::stoi(value, nullptr, 0));
		case IODParser::DataType::INTEGER16:
			return static_cast<int16_t>(std::stoi(value, nullptr, 0));
		case IODParser::DataType::UNSIGNED32:
			return static_cast<uint32_t>(std::stoul(value, nullptr, 0));
		case IODParser::DataType::INTEGER32:
			return static_cast<int32_t>(std::stoi(value, nullptr, 0));
		case IODParser::DataType::VISIBLE_STRING:
		{
			std::array<uint8_t, 24> arr{};
			std::copy(value.begin(), value.end(), arr.begin());
			return arr;
		}
		case IODParser::DataType::UNKNOWN:
			return uint8_t{};
	}
	return uint8_t{};
}
