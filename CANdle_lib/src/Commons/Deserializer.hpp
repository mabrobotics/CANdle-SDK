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
	T frame = bit_cast<T>(byteArray);
	return frame;
}

template <typename T, typename Iterator>
void serialize(T frame, Iterator it)
{
	std::array<uint8_t, sizeof(T)> byteArray{};
	byteArray = bit_cast<decltype(byteArray)>(frame);
	std::copy(byteArray.begin(), byteArray.end(), it);
}

#endif /* MAB_COMMONS_DESERIALIZER_HPP_ */
