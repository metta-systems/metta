//
// Copyright 2007 - 2009, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "types.h"

/*!
@brief Memory utilities similar to standard libc operations.
*/
class memutils
{
public:
    /*!
    * Fill a region of memory with the given value.
    * @param[out] dest  Pointer to the start of the area.
    * @param[in]  value The byte to fill the area with
    * @param[in]  count The size of the area.
    * @return           Pointer to the start of the area.
    *
    * @warning Do not use fill_memory() to access IO space,
    * use fill_io_memory() instead.
    **/
    static void* fill_memory(void* dest, int value, size_t count);

    /*!
    * Copy one area of memory to another.
    * @param[out] dest  Where to copy to
    * @param[in]  src   Where to copy from
    * @param[in]  count The size of the area.
    * @return           Pointer to the start of the destination area.
    *
    * @warning You should not use this function to access IO space,
    * use copy_memory_to_io() or copy_memory_from_io() instead.
    **/
    static void* copy_memory(void* dest, const void* src, size_t count);

    /*!
    * Copy one area of memory to another.
    * @param[out] dest  Where to copy to
    * @param[in]  src   Where to copy from
    * @param[in]  count The size of the area.
    * @return           Pointer to the start of the destination area.
    *
    * Unlike copy_memory(), this function copes with overlapping areas.
    **/
    static void* move_memory(void* dest, const void* src, size_t count);

    /*!
    * Compare two C strings.
    * @param[in] s1 One string
    * @param[in] s2 Another string
    * @return true if @c s1 equals @c s2, false if strings are different.
    *
    * @note Used only in multiboot::set_header.
    **/
    static bool strequal(const char *s1, const char *s2);

    static size_t strlen(const char *s);
};

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
