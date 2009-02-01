//
// Copyright 2007 - 2009, Stanislav Karchebnyy <berkus+metta@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "types.h"
#include "macros.h"

namespace metta {
namespace common {

/**
* An unordered array of predefined size that holds its contents internally as bits.
* Therefore, each element can only be of boolean type.
**/
class bit_array
{
public:
    //! Array dword position to bit index.
    inline static uint32_t INDEX_TO_BIT(int a)    { return a * (8*4); }
    //! Bit to dword position in array.
    inline static uint32_t INDEX_FROM_BIT(int a)  { return a / (8*4); }
    //! Bit to offset within dword.
    inline static uint32_t OFFSET_FROM_BIT(int a) { return a % (8*4); }

    /**
    * Construct an array of @p nbits capacity.
    **/
    bit_array(uint32_t nbits) : N(nbits)
    {
        ASSERT(N >= 32);
        table = new uint32_t [INDEX_FROM_BIT(N)];
        clear_all();
    }

    ~bit_array()
    {
        delete [] table;
    }

    /**
    * Clears the value of all bits in the bitmap.
    **/
    void clear_all()
    {
        for (uint32_t i = 0; i < INDEX_FROM_BIT(N); i++)
        {
            table[i] = 0;
        }
    }

    /**
    * Sets the bit at index @p i
    **/
    void set(uint32_t i)
    {
        ASSERT(i < N);
        uint32_t idx = INDEX_FROM_BIT(i);
        uint32_t off = OFFSET_FROM_BIT(i);
        table[idx] |= (0x1 << off);
    }

    /**
    * Clears the bit at index @p i
    **/
    void clear(uint32_t i)
    {
        ASSERT(i < N);
        uint32_t idx = INDEX_FROM_BIT(i);
        uint32_t off = OFFSET_FROM_BIT(i);
        table[idx] &= ~(0x1 << off);
    }

    /**
    * Tests if the bit at index @p i is set.
    **/
    bool test(uint32_t i)
    {
        ASSERT(i < N);
        uint32_t idx = INDEX_FROM_BIT(i);
        uint32_t off = OFFSET_FROM_BIT(i);
        return (table[idx] & (0x1 << off));
    }

    /**
    * Finds the first bit that is clear. Uses optimisations, so faster than
    * just looping and calling test().
    **/
    uint32_t first_clear()
    {
        for (uint32_t i = 0; i < INDEX_FROM_BIT(N); i++)
        {
            if (table[i] == 0xFFFFFFFF) // nothing free, exit early.
                continue;

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
        return (uint32_t)-1;
    }

    /**
    * Finds the first bit that is set. Uses optimisations, so faster than
    * just looping and calling test().
    **/
    uint32_t first_set()
    {
        for (uint32_t i = 0; i < INDEX_FROM_BIT(N); i++)
        {
            if (table[i] == 0x00000000) // nothing set, exit early.
                continue;

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
        return (uint32_t)-1;
    }

private:
    /**
    * The bitmap itself.
    **/
    uint32_t *table;

    /**
    * The number of bit entries
    **/
    uint32_t N;
};

}
}

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
