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
#include "candle/types/mab_types.hpp"
#include "candle/logger/logger.hpp"
#include "candle/objectDictionary/edsEntry.hpp"

namespace mab
{

    class EDSParser
    {
      public:
        enum Error_t
        {
            OK,
            INVALID_PATH,
            INVALID_FILE,
            INVALID_INDEX,
            UNKNOWN_ERROR
        };
        /// @brief Load EDS file from the specified path
        /// @param edsFilePath Path to the EDS file
        /// @return Error_t indicating the result of the operation
        static std::pair<std::shared_ptr<EDSObjectDictionary>, Error_t> load(
            const std::filesystem::path& edsFilePath);

      private:
        static inline bool isEntry(std::string_view s)
        {
            return !s.empty() &&
                   std::all_of(
                       s.begin(), s.end(), [](unsigned char c) { return std::isxdigit(c); }) &&
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
