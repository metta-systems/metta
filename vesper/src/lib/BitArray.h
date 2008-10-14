//
// Copyright 2007 - 2008, Stanislav Karchebnyy <berkus+metta@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "Types.h"

/**
  An unordered array that holds its contents internally as bits. Therefore,
  each element can only be of boolean type.

  Unit tested: 24 Jul 07
**/

class BitArray
{
public:
	inline static uint32_t INDEX_TO_BIT(int a)    { return a * (8*4); }
	inline static uint32_t INDEX_FROM_BIT(int a)  { return a / (8*4); }
	inline static uint32_t OFFSET_FROM_BIT(int a) { return a % (8*4); }

	/**
     * Constructor
	 */
	BitArray(uint32_t nbits) : N(nbits)
	{
		table = new uint32_t [INDEX_FROM_BIT(N)];
		clearAll();
	}

	/**
	 * Clears the value of all bits in the bitmap.
	 */
	void clearAll()
	{
		for (uint32_t i = 0; i < INDEX_FROM_BIT(N); i++)
		{
			table[i] = 0;
		}
	}

	/**
	 * Sets the bit at index i
	 */
	void set(uint32_t i)
	{
		ASSERT(i < N);
		uint32_t idx = INDEX_FROM_BIT(i);
		uint32_t off = OFFSET_FROM_BIT(i);
		table[idx] |= (0x1 << off);
	}

	/**
	 * Clears the bit at index i
	 */
	void clear(uint32_t i)
	{
		ASSERT(i < N);
		uint32_t idx = INDEX_FROM_BIT(i);
		uint32_t off = OFFSET_FROM_BIT(i);
		table[idx] &= ~(0x1 << off);
	}

	/**
	 * Tests if the bit at index i is set.
	 */
	bool test(uint32_t i)
	{
		ASSERT(i < N);
		uint32_t idx = INDEX_FROM_BIT(i);
		uint32_t off = OFFSET_FROM_BIT(i);
		return (table[idx] & (0x1 << off));
	}

	/**
	 * Finds the first bit that is clear. Uses optimisations so faster than
	 * just looping and calling test().
	 */
	uint32_t firstClear()
	{
		for (uint32_t i = 0; i < INDEX_FROM_BIT(N); i++)
		{
			if (table[i] != 0xFFFFFFFF) // nothing free, exit early.
			{
				// at least one bit is clear here.
				for (uint32_t j = 0; j < 32; j++)
				{
					uint32_t toTest = 0x1 << j;
					if (!(table[i] & toTest))
					{
						return INDEX_TO_BIT(i)+j;
					}
				}
			}
		}
		return (uint32_t)-1;
	}

	/**
	 * Finds the first bit that is set. Uses optimisations so faster than
	 * just looping and calling test().
	 */
	uint32_t firstSet()
	{
		for (uint32_t i = 0; i < INDEX_FROM_BIT(N); i++)
		{
			if (table[i] != 0x00000000) // nothing set, exit early.
			{
				// at least one bit is set here.
				for (uint32_t j = 0; j < 32; j++)
				{
					uint32_t toTest = 0x1 << j;
					if (table[i] & toTest)
					{
						return INDEX_TO_BIT(i)+j;
					}
				}
			}
		}
		return (uint32_t)-1;
	}

private:
	/**
	 * The bitmap itself.
	 */
	uint32_t *table;

	/**
	 * The number of bit entries
	 */
	uint32_t N;
};

