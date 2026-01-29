#include <fcntl.h>
#include <cstring>
#include <edsEntry.hpp>
#include <string>
#include <charconv>

namespace mab
{
    template <class T>
    std::vector<std::byte> EDSEntryValue<T>::getSerializedValue() const
    {
        return std::vector<std::byte>(reinterpret_cast<const std::byte*>(&m_value),
                                      reinterpret_cast<const std::byte*>(&m_value) + sizeof(T));
    }

    template <class T>
    bool EDSEntryValue<T>::setSerializedValue(std::span<const std::byte> value)
    {
        if (value.size() != sizeof(T))
            return false;

        std::memcpy(&m_value, value.data(), sizeof(T));

        return true;
    }

    template <class T>
    std::string EDSEntryValue<T>::toString() const
    {
        return std::to_string(m_value);
    }

    template <class T>
    bool EDSEntryValue<T>::fromString(const std::string_view value)
    {
        T parsed{};
        if constexpr (std::is_integral_v<T>)
        {
            auto [ptr, ec] = std::from_chars(value.data(), value.data() + value.size(), parsed, 10);

            if (ec != std::errc{} || ptr != value.data() + value.size())
                return false;
        }
        else if constexpr (std::is_floating_point_v<T>)
        {
            auto [ptr, ec] = std::from_chars(
                value.data(), value.data() + value.size(), parsed, std::chars_format::general);

            if (ec != std::errc{} || ptr != value.data() + value.size())
                return false;
        }
        else
        {
            return false;
        }

        m_value = parsed;
        return true;
    }
    template <class T>
    I_EDSEntry::EDSEntryMetaData EDSEntryValue<T>::getBasicData() const
    {
        return m_metaData;
    }
    // Possible type instantiation
    template class EDSEntryValue<uint8_t>;
    template class EDSEntryValue<uint16_t>;
    template class EDSEntryValue<uint32_t>;
    template class EDSEntryValue<uint64_t>;
    template class EDSEntryValue<int8_t>;
    template class EDSEntryValue<int16_t>;
    template class EDSEntryValue<int32_t>;
    template class EDSEntryValue<int64_t>;
    template class EDSEntryValue<float>;
    template class EDSEntryValue<double>;

}  // namespace mab
