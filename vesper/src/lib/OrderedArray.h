#pragma once

#include "Types.h"
#include "common.h"
#include "DefaultConsole.h"

/**
 * Array of maximum size @c N of pointers to @c Type.
 * This array is insertion sorted - it always remains in a sorted state (between calls).
 * @c Type must implement operator <()
 * Array must be in-place allocatable for Heap to work correctly.
 */
template<class Type, uint32_t N>
class OrderedArray
{
public:
	/**
	 * Create an ordered array.
	 */
	inline OrderedArray()
	{
		memset(array, 0, N * sizeof(Type*));
		size = 0;
	}

	~OrderedArray() {}

	void insert(Type* item)
	{
		ASSERT(size+1 < N);
		uint32_t iterator = 0;
		while (iterator < size && *array[iterator] < *item)
			iterator++;

		if (iterator == size) // just add at the end of the array
			array[size++] = item;
		else
		{
			Type* tmp = array[iterator];
			array[iterator] = item;
			while (iterator < size)
			{
				iterator++;
				Type* tmp2 = array[iterator];
				array[iterator] = tmp;
				tmp = tmp2;
			}
			size++;
		}
	}

	inline Type* lookup(uint32_t i)
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
		kconsole.print("Dumping OrderedArray %p (%d items)\n", this, size);
		for(int i = 0; i < size; i++)
		{
			kconsole.print("    %p\n", array[i]);
		}
	}

private:
	Type *array[N];
	uint32_t size;
};

