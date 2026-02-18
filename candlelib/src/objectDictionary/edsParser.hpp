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

    class EDSParser
    {
      public:
        /// @brief Load EDS file from the specified path
        /// @param edsFilePath Path to the EDS file
        /// @return Error_t indicating the result of the operation
        static std::pair<std::shared_ptr<EDSObjectDictionary>, Error_t> load(
            const std::filesystem::path& edsFilePath);

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
        static EDSEntry::EDSValueMetaData parseValueMetadata(mINI::INIMap<std::string>& entry);

        static inline bool isEntry(std::string_view s)
        {
            return !s.empty() &&
                   std::all_of(
                       s.begin(), s.end(), [](unsigned char c) { return std::isdigit(c); }) &&
                   s.size() == 4;
        }
        static bool isSubentry(std::string_view s)
        {
            constexpr std::string_view sub = "sub";

            if (s.size() <= 7)  // minimum: 4 + 3 + 1 = 8
                return false;

            return s.substr(4, 3) == sub;
        }

        static std::optional<std::pair<unsigned int, unsigned int>> extractIndexAndSubindex(
            std::string_view input)
        {
            // Must be at least "XXXXsubY"
            if (input.size() < 8)
                return std::nullopt;

            // Check fixed separator
            if (input.substr(4, 3) != "sub")
                return std::nullopt;

            unsigned int index{};
            unsigned int subindex{};

            // Parse XXXX
            auto [p1, ec1] = std::from_chars(input.data(), input.data() + 4, index, 16);

            if (ec1 != std::errc{})
                return std::nullopt;

            // Parse YYY (rest of string)
            auto [p2, ec2] =
                std::from_chars(input.data() + 7, input.data() + input.size(), subindex, 16);

            if (ec2 != std::errc{} || p2 != input.data() + input.size())
                return std::nullopt;

            return std::pair{index, subindex};
        }
    };
}  // namespace mab
#endif  // EDS_PARSER_HPP
