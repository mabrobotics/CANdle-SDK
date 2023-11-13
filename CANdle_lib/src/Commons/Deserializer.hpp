/*
 * Deserializer.hpp
 *
 *  Created on: 18 wrz 2023
 *      Author: Piotr Wasilewski
 */

#ifndef MAB_COMMONS_DESERIALIZER_HPP_
#define MAB_COMMONS_DESERIALIZER_HPP_

#include <algorithm>

#include "BitCast.hpp"

template <typename T, typename Iterator>
T deserialize(Iterator it)
{
	std::array<uint8_t, sizeof(T)> byteArray{};
	std::copy(it, it + byteArray.size(), byteArray.begin());
	return bit_cast_<T, std::array<uint8_t, sizeof(T)>>(byteArray);
}

template <typename T, typename Iterator>
size_t serialize(T frame, Iterator it)
{
	std::array<uint8_t, sizeof(T)> byteArray{};
	byteArray = bit_cast_<decltype(byteArray)>(frame);
	std::copy(byteArray.begin(), byteArray.end(), it);
	return byteArray.size();
}

#endif /* MAB_COMMONS_DESERIALIZER_HPP_ */
