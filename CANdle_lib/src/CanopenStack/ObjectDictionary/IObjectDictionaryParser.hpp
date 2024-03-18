#ifndef IOBJECTDICTIONARYPARSER_HPP
#define IOBJECTDICTIONARYPARSER_HPP

#include <array>
#include <memory>
#include <string>
#include <unordered_map>
#include <variant>

/**
 * @brief Interface for parsers that fill in Object Dictionary based on a description file
 *
 */
class IODParser
{
   public:
	typedef std::variant<std::monostate, bool, uint8_t, int8_t, uint16_t, int16_t, uint32_t, int32_t, float, std::array<uint8_t, 24>> ValueType;
	template <typename T>
	using IsIntegralType = std::disjunction<std::is_same<T, uint8_t>, std::is_same<T, int8_t>, std::is_same<T, uint16_t>, std::is_same<T, int16_t>, std::is_same<T, uint32_t>, std::is_same<T, int32_t>>;

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
		REAL32 = 8,			/**< float  */
		VISIBLE_STRING = 9, /**< String up to 24 characters  */
	};

	enum class ObjectType
	{
		UNKNOWN = -1,  /**< Unknown object type  */
		DOM = 2,	   /**< Domain  */
		DEFTYPE = 5,   /**< Type definition  */
		DEFSTRUCT = 6, /**< Struct definition  */
		VAR = 7,	   /**< Variable  */
		ARRAY = 8,	   /**< Array  */
		REC = 9,	   /**< Record  */
	};

	enum class AccessSDO
	{
		no = 0, /**< No acess */
		ro = 1, /**< Read-only  */
		wo = 2, /**< Write-only  */
		rw = 3	/**< Read-write  */
	};

	/**
	 * @brief Single object dictionary entry definition (either index or subindex param)
	 *
	 */
	struct Entry
	{
		std::string parameterName;
		ObjectType objectType;
		DataType dataType;
		AccessSDO accessType;
		size_t highestSubindex = 0;
		std::unordered_map<uint32_t, std::unique_ptr<Entry>> subEntries;
		ValueType highLimit;
		ValueType lowLimit;
		ValueType value;
	};

	using ODType = std::unordered_map<uint32_t, std::unique_ptr<Entry>>;

	virtual ~IODParser() = default;
	virtual bool parseFile(const std::string& filePath, ODType& objectDictionary) = 0;
};

#endif