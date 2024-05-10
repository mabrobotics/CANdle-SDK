/**
 * @file ObjectDictionaryParserEDS.hpp
 * @author Piotr Wasilewski (piotr.wasilewski@mabrobotics.pl)
 * @brief
 * @version 0.1
 * @date 2024-03-08
 *
 * @copyright Copyright (c) 2024
 *
 */
#ifndef OBJECTDICTIONARYPARSEREDS_HPP
#define OBJECTDICTIONARYPARSEREDS_HPP

#include <memory>
#include <string>
#include <unordered_map>

#include "IObjectDictionaryParser.hpp"
#include "ini.h"

/**
 * @brief Implements IODParser for *.eds files
 *
 * If needed other parsers for different files can be implemented based on shared interface
 */
class ObjectDictionaryParserEDS : public IODParser
{
   public:
	const std::unordered_map<std::string, IODParser::AccessSDO> strToAccessType = {
		{"no", IODParser::AccessSDO::no}, {"ro", IODParser::AccessSDO::ro}, {"wo", IODParser::AccessSDO::wo}, {"rw", IODParser::AccessSDO::rw}};

	/**
	 * @brief Parses EDS file and fills in the objectDictionary.
	 *
	 * The parsing process also intializes the std::variant with the correct type based on DataType fields from the EDS file.
	 * @param filePath file path to the EDS file.
	 * @param objectDictionary reference to OD to be filled in based on EDS
	 * @return true
	 * @return false
	 */
	bool parseFile(const std::string& filePath, ODType& objectDictionary) override;
	static IODParser::ValueType fillValue(std::string& value, IODParser::DataType dataType);
};

#endif