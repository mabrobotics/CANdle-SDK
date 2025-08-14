#include <string>
#include <fstream>
#include <filesystem>
#include <iostream>
#include <regex>
#include <set>
#include <algorithm>  // std::transform
#include <cctype>     // std::tolower

#include "mab_types.hpp"
#include "logger.hpp"

#ifndef EDS_PARSER_HPP
#define EDS_PARSER_HPP

/// @brief Possible errors present in this class
/// @details This enum defines the possible errors that can occur during EDS file operations.
/// @note The values correspond to specific error conditions such as invalid path, invalid file,
/// invalid index, and unknown errors.
enum Error_t
{
    OK,
    INVALID_PATH,
    INVALID_FILE,
    INVALID_INDEX,
    UNKNOWN_ERROR
};

/// @brief Enum representing the type of object in the EDS file
/// @details This enum is used to categorize objects in the EDS file into three types:
/// MandatoryObjects, OptionalObjects, and ManufacturerObjects.
/// @note The values correspond to the object types defined in the EDS file.
enum SectionType_t
{
    MandatoryObjects    = 0,
    OptionalObjects     = 1,
    ManufacturerObjects = 2,
};

/// @brief Structure representing an object in the EDS file
/// @details Contains fields such as index, subindex, parameter name, storage location, data type,
/// access type, PDOMapping, default value, and object type.
/// @note The object type is represented as an enum for better readability.
struct edsObject
{
    u32           index           = 0x0000;
    u8            subIndex        = 0x0;
    std::string   ParameterName   = "";
    u8            ObjectType      = 0;
    std::string   StorageLocation = "RAM";
    u32           DataType        = 0x0000;
    std::string   accessType      = "RO";
    bool          PDOMapping      = false;
    u32           defaultValue    = 0;
    SectionType_t sectionType     = ManufacturerObjects;
};

/// @brief Get the name of the object type based on its hexadecimal representation
/// @param hex The hexadecimal string representing the object type
/// @return std::string The name of the object type, or the hex string if not recognized
std::string getObjectTypeName(const std::string& hex);

/// @brief Get the name of the data type based on its hexadecimal representation
/// @param hex The hexadecimal string representing the data type
/// @return std::string The name of the data type, or the hex string if not recognized
std::string getDataTypeName(const std::string& hex);

/// @brief Convert a string to lowercase
/// @param s The string to convert
/// @return std::string The lowercase version of the input string
std::string toLower(const std::string& s);

class edsParser
{
  public:
    explicit edsParser();

    ~edsParser();

    /// @brief Load EDS file from the specified path
    /// @param edsFilePath Path to the EDS file
    /// @return Error_t indicating the result of the operation
    Error_t load(const std::string& edsFilePath);

    /// @brief Check if the EDS file is valid
    /// @return Error_t indicating the result of the validation
    Error_t isValid();

    /// @brief Unload the EDS file
    /// @return Error_t indicating the result of the operation
    Error_t unload();

    /// @brief Display the content of the EDS file
    /// @return Error_t indicating the result of the operation
    Error_t display();

    /// @brief Generate Markdown documentation from the EDS file
    /// @return Error_t indicating the result of the operation
    Error_t generateMarkdown();

    /// @brief Generate HTML documentation from the EDS file
    /// @return Error_t indicating the result of the operation
    Error_t generateHtml();

    /// @brief Generate C++ header and source files from the EDS file
    /// @return Error_t indicating the result of the operation
    Error_t generateCpp();

    /// @brief Get the content of an object from its index and subindex
    /// @param index Index of the object to retrieve
    /// @param subindex Subindex of the object to retrieve
    /// @return Error_t indicating the result of the operation
    Error_t get(u32 index, u8 subindex);

    /// @brief Find and display the content of an object based on a search term
    /// @param searchTerm Term to search for in the EDS file
    /// @return Error_t indicating the result of the operation
    Error_t find(const std::string& searchTerm);

    /// @brief Add a new object to the EDS file
    /// @param obj The edsObject to add
    /// @return Error_t indicating the result of the operation
    Error_t addObject(const edsObject& obj);

    /// @brief Delete an object from the EDS file based on its index and subindex
    /// @param index Index of the object to delete
    /// @param subindex Subindex of the object to delete
    /// @return Error_t indicating the result of the operation
    Error_t deleteObject(u32 index, u8 subindex);

    /// @brief Modify an existing object in the EDS file
    /// @param obj The edsObject with updated values
    /// @param index Index of the object to modify
    /// @param subindex Subindex of the object to modify
    /// @return Error_t indicating the result of the operation
    Error_t modifyObject(const edsObject& obj, u32 index, u8 subindex);

  private:
    Logger log;

    std::string m_edsFilePath;

    /// @brief Update the file path from the eds_path.txt file
    /// @return Error_t indicating the result of the operation
    Error_t updateFilePath();

    /// @brief Generate a section string for an edsObject
    /// @param obj The edsObject to generate the section for
    /// @return std::string representing the section of the object
    std::string generateObjectSection(const edsObject& obj);

    /// @brief verify if the EDS file has a FileInfo section
    /// @return true if the FileInfo section exists, false otherwise
    bool hasFileInfoSection();

    /// @brief Check if the EDS file has a MandatoryObjects section
    /// @return true if the MandatoryObjects section exists, false otherwise
    bool hasMandatoryObjectsSection();

    /// @brief Check if the EDS file has a SupportedObjects key in the MandatoryObjects section
    /// @return true if the SupportedObjects key exists, false otherwise
    bool hasSupportedObjects();

    /// @brief Check if the EDS file has mandatory indices (0x1000, 0x1001, 0x1018)
    /// @return true if all mandatory indices are present, false otherwise
    bool hasMandatoryIndices();

    /// @brief Check if the EDS file has only valid access types
    /// @return true if all access types are valid, false otherwise
    bool hasValidAccessTypes();

    /// @brief Check if the EDS file has only valid data types
    /// @return true if all data types are valid, false otherwise
    bool hasValidDataTypes();
};

#endif  // EDS_PARSER_HPP