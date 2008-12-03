//
// Copyright 2007 - 2008, Stanislav Karchebnyy <berkus+metta@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "string.h"

/**
* memset - Fill a region of memory with the given value
* @param s: Pointer to the start of the area.
* @param c: The byte to fill the area with
* @param count: The size of the area.
*
* Do not use memset() to access IO space, use memset_io() instead.
**/
void* memset(void* dest, int value, size_t count)
{
    char *xs = (char *)dest;
    while (count--)
        *xs++ = value;
    return dest;
}

/**
* memcpy - Copy one area of memory to another
* @param dest: Where to copy to
* @param src: Where to copy from
* @param count: The size of the area.
*
* You should not use this function to access IO space, use memcpy_toio()
* or memcpy_fromio() instead.
**/
void* memcpy(void* dest, const void* src, size_t count)
{
    char *tmp = (char *)dest;
    const char *s = (const char *)src;

    while (count--)
        *tmp++ = *s++;
    return dest;
}

/**
* memmove - Copy one area of memory to another
* @param dest: Where to copy to
* @param src: Where to copy from
* @param count: The size of the area.
*
* Unlike memcpy(), memmove() copes with overlapping areas.
**/
void* memmove(void* dest, const void* src, size_t count)
{
    char *tmp;
    const char *s;

    if (dest <= src) {
        tmp = (char *)dest;
        s = (const char *)src;
        while (count--)
            *tmp++ = *s++;
    } else {
        tmp = (char *)dest;
        tmp += count;
        s = (const char *)src;
        s += count;
        while (count--)
            *--tmp = *--s;
    }
    return dest;
}

// kate: indent-width 4; replace-tabs on;
// vi:set ts=4:set expandtab=on:
