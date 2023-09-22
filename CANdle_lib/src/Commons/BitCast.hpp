/*
 * BitCast.hpp
 *
 *  Created on: Sep 4, 2023
 *      Author: Piotr Wasilewski
 */

#ifndef MAB_BITCAST_HPP_
#define MAB_BITCAST_HPP_

#include <cstring>

template <class T2, class T1>
T2 bit_cast(T1 t1)
{
	static_assert(sizeof(T1) == sizeof(T2), "Types must match sizes");
	static_assert(std::is_trivial<T1>::value, "Requires POD input");
	static_assert(std::is_trivial<T2>::value, "Requires POD output");

	T2 t2;
	std::memcpy(std::addressof(t2), std::addressof(t1), sizeof(T1));
	return t2;
}

#endif
