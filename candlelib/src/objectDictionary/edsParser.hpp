#ifndef EDS_PARSER_HPP
#define EDS_PARSER_HPP

#include <string>
#include <fstream>
#include <iostream>
#include <regex>
#include <set>
#include <filesystem>
#include <algorithm>
#include <cctype>
#include "configHelpers.hpp"
#include "mab_types.hpp"
#include "logger.hpp"
#include "edsEntry.hpp"
#include "mini/ini.h"

namespace mab
{

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
        mINI::INIStructure m_edsIni;

        /// @brief Load EDS file from the specified path
        /// @param edsFilePath Path to the EDS file
        /// @return Error_t indicating the result of the operation
        Error_t load(const std::filesystem::path& edsFilePath);

        /// @brief Check if the EDS file is valid
        /// @return Error_t indicating the result of the validation
        Error_t isValid();

        /// @brief Display the content of the EDS file
        /// @return Error_t indicating the result of the operation
        Error_t display();

        /// @brief Get the content of an object from its index and subindex
        /// @param index Index of the object to retrieve
        /// @param subindex Subindex of the object to retrieve
        /// @return Error_t indicating the result of the operation
        Error_t get(u32 index, u8 subindex);

        /// @brief Find and display the content of an object based on a search term
        /// @param searchTerm Term to search for in the EDS file
        /// @return Error_t indicating the result of the operation
        Error_t find(const std::string& searchTerm);

      private:
        Logger log = Logger(Logger::ProgramLayer_E::TOP, "EDS_PARSER");

        /// @brief verify if the EDS file has a FileInfo section
        /// @return true if the FileInfo section exists, false otherwise
        bool hasFileInfoSection();

        /// @brief Check if the EDS file has a MandatoryObjects section
        /// @return true if the MandatoryObjects section exists, false otherwise
        bool hasMandatoryObjectsSection();

        /// @brief Check if the EDS file has a SupportedObjects key in the MandatoryObjects
        /// section
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
}  // namespace mab
#endif  // EDS_PARSER_HPP
