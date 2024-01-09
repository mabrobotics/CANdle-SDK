#ifndef IOBJECTDICTIONARYPARSER_HPP
#define IOBJECTDICTIONARYPARSER_HPP

#include <array>
#include <map>
#include <memory>
#include <string>
#include <variant>

class IODParser
{
   public:
	typedef std::variant<uint8_t, int8_t, uint16_t, int16_t, uint32_t, int32_t, float, std::array<uint8_t, 24>> ValueType;

	enum class DataType
	{
		UNKNOWN = 0,
		BOOLEAN = 1,
		INTEGER8 = 2,
		INTEGER16 = 3,
		INTEGER32 = 4,
		UNSIGNED8 = 5,
		UNSIGNED16 = 6,
		UNSIGNED32 = 7,
		REAL32 = 8,
		VISIBLE_STRING = 9,
		INTEGER64 = 0x15,
		UNSIGNED64 = 0x1B,
	};

	enum class ObjectType
	{
		UNKNOWN = -1,
		DOM = 2,
		DEFTYPE = 5,
		DEFSTRUCT = 6,
		VAR = 7,
		ARRAY = 8,
		REC = 9,
	};

	enum class PDOMappingType
	{
		no = 0,
		optional = 1,
		RPDO = 2,
		TPDO = 3,
	};

	enum class AccessSDO
	{
		no = 0,
		ro = 1,
		wo = 2,
		rw = 3
	};

	struct Entry
	{
		std::string parameterName;
		ObjectType objectType;
		DataType dataType;
		AccessSDO accessType;
		size_t highestSubindex = 0;
		std::map<uint32_t, std::shared_ptr<Entry>> subEntries;
		ValueType value;
	};

	virtual ~IODParser() = default;
	virtual bool parseFile(const std::string& filePath, std::map<uint32_t, std::shared_ptr<Entry>>& objectDictionary) = 0;
};

#endif