#include "utilities.hpp"
#include <algorithm>
#include <cctype>
#include <variant>
#include <string>
#include <string_view>

namespace mab
{
    std::string trim(const std::string_view s)
    {
        auto start = std::find_if_not(s.begin(), s.end(), ::isspace);
        auto end   = std::find_if_not(s.rbegin(), s.rend(), ::isspace).base();
        return (start < end) ? std::string(start, end) : "";
    }

}  // namespace mab
