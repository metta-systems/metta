//
// Copyright 2007 - 2009, Stanislav Karchebnyy <berkus+metta@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "types.h"
#include "macros.h"
#include "common.h"
#include "default_console.h"

using namespace metta::kernel;

namespace metta {
namespace common {

/**
* Array of maximum size @c n of pointers to @c type.
* This array is insertion sorted - it always remains in a sorted state (between calls).
* @c type must implement operator <()
* Array must be in-place allocatable for Heap to work correctly.
* This implementation is not particularly optimized for large arrays - insertion is O(N).
**/
template<class type, uint32_t n>
class ordered_array
{
public:
	/**
	 * Create an ordered array.
	 */
	inline ordered_array()
	{
		memset(array, 0, n * sizeof(type*));
		size = 0;
	}

	void insert(type* item)
	{
		ASSERT(size+1 < n);
		uint32_t iterator = 0;
		while (iterator < size && *array[iterator] < *item)
			iterator++;

		if (iterator == size) // just add at the end of the array
			array[size++] = item;
		else
		{
			type* tmp = array[iterator];
			array[iterator] = item;
			while (iterator < size)
			{
				iterator++;
				type* tmp2 = array[iterator];
				array[iterator] = tmp;
				tmp = tmp2;
			}
			size++;
		}
	}

	inline type* lookup(uint32_t i)
	{
		ASSERT(i < size);
		return array[i];
	}

	void remove(uint32_t i)
	{
		size--;
		while (i < size)
		{
			array[i] = array[i+1];
			i++;
		}
	}

	inline uint32_t count()
	{
		return size;
	}

	/**
	 * Debug helper function.
	 */
	void dump()
	{
		kconsole.print("Dumping ordered_array %p (%d items)\n", this, size);
		for(int i = 0; i < size; i++)
		{
			kconsole.print("    %p\n", array[i]);
		}
	}

private:
	type*    array[n];
	uint32_t size;
};

}
}

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
