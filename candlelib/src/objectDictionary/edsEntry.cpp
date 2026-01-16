#include <edsEntry.hpp>
#include <ios>
#include <string>
#include <charconv>

namespace mab
{
    template <class T>
    std::vector<std::byte> EDSEntryValue<T>::getSerializedValue() const
    {
    }

    template <class T>
    bool EDSEntryValue<T>::setSerializedValue(std::span<const std::byte> value)
    {
    }

    template <class T>
    std::string EDSEntryValue<T>::toString() const
    {
    }

    template <class T>
    bool EDSEntryValue<T>::fromString(const std::string_view value)
    {
        static_assert(std::is_arithmetic_v<T>,
                      "EDSEntryValue<T>::fromString requires an arithmetic type");
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
    EDSEntryMetaData EDSEntryValue<T>::getBasicData() const
    {
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
