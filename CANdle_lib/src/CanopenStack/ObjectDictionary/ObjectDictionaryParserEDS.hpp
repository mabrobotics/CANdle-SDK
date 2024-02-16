#ifndef OBJECTDICTIONARYPARSEREDS_HPP
#define OBJECTDICTIONARYPARSEREDS_HPP

#include <memory>
#include <string>
#include <unordered_map>

#include "IObjectDictionaryParser.hpp"
#include "ini.h"

class ObjectDictionaryParserEDS : public IODParser
{
   public:
	const std::unordered_map<std::string, IODParser::AccessSDO> strToAccessType = {
		{"no", IODParser::AccessSDO::no}, {"ro", IODParser::AccessSDO::ro}, {"wo", IODParser::AccessSDO::wo}, {"rw", IODParser::AccessSDO::rw}};

	bool parseFile(const std::string& filePath, ODType& objectDictionary) override;
	static IODParser::ValueType fillValue(std::string& value, IODParser::DataType dataType);
};

#endif