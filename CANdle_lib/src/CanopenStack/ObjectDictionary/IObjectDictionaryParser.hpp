#ifndef IOBJECTDICTIONARYPARSER_HPP
#define IOBJECTDICTIONARYPARSER_HPP

#include <map>
#include <string>

class IObjectDictionaryParser
{
   public:
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
		no,
		ro,
		wo,
		rw
	};

	struct Entry
	{
		std::string parameterName;
		ObjectType objectType;
		DataType datatype;
		AccessSDO accessType;
		size_t highestSubindex = 0;
		void* defaultValue = nullptr;
	};

	virtual ~IObjectDictionaryParser() = default;

	virtual bool parseFile(const std::string& filePath, std::map<uint32_t, std::shared_ptr<Entry>>& objectDictionary) = 0;
};

#endif