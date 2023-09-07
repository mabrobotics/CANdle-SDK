#pragma once

#include <array>

template <typename T>
T getHeaderFromArray(typename std::array<uint8_t, sizeof(T)>::const_iterator it)
{
    std::array<uint8_t, sizeof(T)> headerArray;
    std::copy(it, it + headerArray.size(), headerArray.begin());
    return std::bit_cast<T>(headerArray);
}
