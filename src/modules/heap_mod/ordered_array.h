//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2010, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "types.h"
#include "default_console.h"
#include "memutils.h"

/*!
* Array of maximum size @c n of pointers to @c type.
* This array is insertion sorted - it always remains in a sorted state (between calls).
* @c type must implement operator <()
* Array must be in-place allocatable for @c heap to work correctly.
* This implementation is not particularly optimized for large arrays - insertion is O(N).
*/
template<typename type_t, size_t n>
class ordered_array_t
{
public:
    /*!
    * Create an ordered array.
    */
    inline ordered_array_t()
    {
        memutils::fill_memory(array, 0, n * sizeof(type_t*));
        size = 0;
    }

    void insert(type_t* item)
    {
        ASSERT(size+1 < n);
        uint32_t iterator = 0;
        while (iterator < size && *array[iterator] < *item)
            iterator++;

        if (iterator == size) // just add at the end of the array
            array[size++] = item;
        else
        {
            type_t* tmp = array[iterator];
            array[iterator] = item;
            while (iterator < size)
            {
                iterator++;
                type_t* tmp2 = array[iterator];
                array[iterator] = tmp;
                tmp = tmp2;
            }
            size++;
        }
    }

    inline type_t* lookup(uint32_t i)
    {
        ASSERT(i < size);
        return array[i];
    }

    template <typename T>
    inline T lookup(uint32_t i)
    {
        ASSERT(i < size);
        return reinterpret_cast<T>(array[i]);
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

    inline size_t count()
    {
        return size;
    }

    /*!
    * Debug helper function.
    */
    void dump()
    {
        kconsole.print("Dumping ordered_array %p (%d items)\n", this, size);
        for(int i = 0; i < size; i+=8)
        {
            for (int j = 0; j < 8, i+j < size; j++)
                kconsole.print(" %p", array[i+j]);
            kconsole.newline();
        }
    }

private:
    type_t*  array[n];
    size_t   size;
};

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
