#include <fcntl.h>
#include <any>
#include <cstring>
#include <edsEntry.hpp>
#include <exception>
#include <numeric>
#include <span>
#include <string>
#include <charconv>
#include "edsParser.hpp"

namespace mab
{
    //-------------------VALUE------------------------
    template <class T>
    std::vector<std::byte> EDSEntryValue<T>::getSerializedValue() const
    {
        auto byteRepresntation = std::as_bytes(std::span{&m_value, 1});
        return std::vector<std::byte>(byteRepresntation.begin(), byteRepresntation.end());
    }

    template <class T>
    I_EDSEntry::Error_t EDSEntryValue<T>::setSerializedValue(std::span<const std::byte> value)
    {
        if (value.size() != sizeof(T))
            return I_EDSEntry::Error_t::PARSING_FAILED;

        std::memcpy(&m_value, value.data(), sizeof(T));

        return I_EDSEntry::Error_t::OK;
    }

    template <class T>
    std::string EDSEntryValue<T>::toString() const
    {
        return std::to_string(m_value);
    }

    template <class T>
    I_EDSEntry::Error_t EDSEntryValue<T>::fromString(const std::string_view value)
    {
        T parsed{};
        if constexpr (std::is_integral_v<T>)
        {
            auto [ptr, ec] = std::from_chars(value.data(), value.data() + value.size(), parsed, 10);

            if (ec != std::errc{} || ptr != value.data() + value.size())
                return I_EDSEntry::Error_t::PARSING_FAILED;
        }
        else if constexpr (std::is_floating_point_v<T>)
        {
            auto [ptr, ec] = std::from_chars(
                value.data(), value.data() + value.size(), parsed, std::chars_format::general);

            if (ec != std::errc{} || ptr != value.data() + value.size())
                return I_EDSEntry::Error_t::PARSING_FAILED;
        }
        else
        {
            return I_EDSEntry::Error_t::PARSING_FAILED;
        }

        m_value = parsed;
        return I_EDSEntry::Error_t::OK;
    }
    template <class T>
    I_EDSEntry::EDSEntryMetaData EDSEntryValue<T>::getBasicData() const
    {
        return m_metaData;
    }
    template <class T>
    std::any EDSEntryValue<T>::get() const
    {
        return std::any(m_value);
    }
    template <class T>
    I_EDSEntry::Error_t EDSEntryValue<T>::set(std::any val)
    {
        if (!val.has_value())
            return Error_t::PARSING_FAILED;
        try
        {
            m_value = std::any_cast<T>(val);
        }
        catch (const std::bad_any_cast& ex)
        {
            return Error_t::PARSING_FAILED;
        }
        return Error_t::OK;
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

    //-------------------ARRAY------------------------

    template <class T>
    std::vector<std::byte> EDSEntryArray<T>::getSerializedValue() const
    {
        std::vector<std::byte> buffer;
        for (const auto& value : m_entries)
        {
            auto vec = value.getSerializedValue();
            buffer.insert(buffer.end(), vec.begin(), vec.end());
        }
        return buffer;
    }
    template <class T>
    I_EDSEntry::Error_t EDSEntryArray<T>::setSerializedValue(std::span<const std::byte> value)
    {
        const size_t fullArraySize = sizeof(T) * m_entries.size();
        if (fullArraySize != value.size())
            return I_EDSEntry::Error_t::PARSING_FAILED;

        auto valueIt = value.begin();
        for (auto& m_val : m_entries)
        {
            m_val.setSerializedValue(std::span(valueIt, valueIt + sizeof(T)));
            valueIt += sizeof(T);
        }

        return I_EDSEntry::Error_t::OK;
    }
    template <class T>
    std::string EDSEntryArray<T>::toString() const
    {
        std::string str = "[ ";
        for (auto& m_val : m_entries)
        {
            str += m_val.toString();
            str += " ";
        }
        str += "]";
        return str;
    }
    template <class T>
    I_EDSEntry::Error_t EDSEntryArray<T>::fromString(const std::string_view value)
    {
        return I_EDSEntry::Error_t::PARSING_FAILED;  // TODO: make generic from string array
                                                     // initializer
    }
    template <class T>
    I_EDSEntry::EDSEntryMetaData EDSEntryArray<T>::getBasicData() const
    {
        return m_metaData;
    }

    // Possible type instantiation
    template class EDSEntryArray<uint8_t>;
    template class EDSEntryArray<uint16_t>;
    template class EDSEntryArray<uint32_t>;
    template class EDSEntryArray<uint64_t>;
    template class EDSEntryArray<int8_t>;
    template class EDSEntryArray<int16_t>;
    template class EDSEntryArray<int32_t>;
    template class EDSEntryArray<int64_t>;
    template class EDSEntryArray<float>;
    template class EDSEntryArray<double>;

}  // namespace mab
