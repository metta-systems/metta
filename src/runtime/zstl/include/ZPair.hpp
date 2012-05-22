/*
	ZPair.hpp
	Author: James Russell <jcrussell@762studios.com>
	Created: 9/12/2011

	Purpose: 

	Templated dynamic tuple implementation.

	License:

	This program is free software. It comes without any warranty, to
	the extent permitted by applicable law. You can redistribute it
	and/or modify it under the terms of the Do What The Fuck You Want
	To Public License, Version 2, as published by Sam Hocevar. See
	http://sam.zoy.org/wtfpl/COPYING for more details.

*/

#pragma once

#ifndef _ZPAIR_HPP
#define _ZPAIR_HPP

#include <ZSTL/ZSTLCommon.hpp>

/*
Dynamic tuple implementation.

The template parameter T1 is the type of the first contained value.

The template parameter T2 is the type of the second contained value.
*/
template <typename T1, typename T2>
class ZPair
{
public:
	//The first value
	T1 First;

	//The second value
	T2 Second;

	/*
	Default constructor.
	*/
	ZPair();

	/*
	Copy constructor.

	@param _other - the other pair
	*/
	ZPair(const ZPair<T1, T2>& _other);

	/*
	Parameterized constructor.

	@param _first - the first value
	@param _second - the second value
	*/	
	ZPair(const T1& _first, const T2& _second);

	/*	
	operator < override, used to ensure ZPair can be compared using the default ZComparator.
	
	@param _other - the other pair
	@return (bool) - true if this ZPair is less than the other, false otherwise
	*/
	bool operator < (const ZPair<T1, T2>& _other);

	/*	
	operator == override, used to ensure ZPair can be compared using the default ZComparator.
	
	@param _other - the other pair
	@return (bool) - true if this ZPair is equal to the other, false otherwise
	*/
	bool operator == (const ZPair<T1, T2>& _other);

	/*
	public ZPair<T1, T2>::Swap

	Returns another pair that is has swapped the first and second values of this pair.

	@return (ZPair<T2, T1>) - a pair with swapped first/second values
	*/
	ZPair<T2, T1> Swap() const;
};

template <typename T1, typename T2>
ZPair<T1, T2>::ZPair()
{

}

template <typename T1, typename T2>
ZPair<T1, T2>::ZPair( const ZPair<T1, T2>& _other )
: First(_other.First), Second(_other.Second)
{

}

template <typename T1, typename T2>
ZPair<T1, T2>::ZPair(const T1& _first, const T2& _second)
: First(_first), Second(_second) 
{ 

}

template <typename T1, typename T2>
bool ZPair<T1, T2>::operator < ( const ZPair<T1, T2>& _other )
{
	if (First < _other.First)
		return true;

	return (Second < _other.Second);
}

template <typename T1, typename T2>
bool ZPair<T1, T2>::operator == ( const ZPair<T1, T2>& _other )
{
	if (First == _other.First && Second == _other.Second)
		return true;

	return false;
}

template <typename T1, typename T2>
ZPair<T2, T1> ZPair<T1, T2>::Swap() const
{
	return ZPair<T2, T1>(this->Second, this->First);
}

#endif
