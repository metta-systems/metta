//
// Copyright 2007 - 2009, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "memutils.h"


void* memutils::fill_memory(void* dest, int value, size_t count)
{
    char *xs = (char *)dest;
    while (count--)
        *xs++ = value;
    return dest;
}


void* memutils::copy_memory(void* dest, const void* src, size_t count)
{
    char *tmp = (char *)dest;
    const char *s = (const char *)src;

    while (count--)
        *tmp++ = *s++;
    return dest;
}


void* memutils::move_memory(void* dest, const void* src, size_t count)
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


bool memutils::strequal(const char* s1, const char* s2)
{
    signed char __res;

    while (1) {
        if ((__res = *s1 - *s2++) != 0 || !*s1++)
            break;
    }
    return __res == 0;
}


size_t memutils::strlen(const char* s)
{
    size_t len = 0;
    char*  t   = (char*)s;
    while (*t++)
        len++;
    return len;
}


// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
