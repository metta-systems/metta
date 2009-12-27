//
// Copyright 2007 - 2009, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "memutils.h"

namespace memutils
{

void* fill_memory(void* dest, int value, size_t count)
{
    char *xs = reinterpret_cast<char*>(dest);
    while (count--)
        *xs++ = value;
    return dest;
}


void* copy_memory(void* dest, const void* src, size_t count)
{
    char *tmp = reinterpret_cast<char*>(dest);
    const char *s = reinterpret_cast<const char*>(src);

    while (count--)
        *tmp++ = *s++;
    return dest;
}


void* move_memory(void* dest, const void* src, size_t count)
{
    char *tmp;
    const char *s;

    if (dest <= src) {
        tmp = reinterpret_cast<char*>(dest);
        s = reinterpret_cast<const char*>(src);
        while (count--)
            *tmp++ = *s++;
    } else {
        tmp = reinterpret_cast<char*>(dest);
        tmp += count;
        s = reinterpret_cast<const char*>(src);
        s += count;
        while (count--)
            *--tmp = *--s;
    }
    return dest;
}


bool is_memory_equal(const void* left, const void* right, size_t count)
{
    const char* ltmp = reinterpret_cast<const char*>(left);
    const char* rtmp = reinterpret_cast<const char*>(right);

    while (count--)
        if (*ltmp++ != *rtmp++)
            return false;
    return true;
}


bool is_string_equal(const char* s1, const char* s2)
{
    signed char __res;

    if (!s1 && !s2)
        return true;
    if (!s1 || !s2)
        return false;

    while (1) {
        if ((__res = *s1 - *s2++) != 0 || !*s1++)
            break;
    }
    return __res == 0;
}


size_t string_length(const char* s)
{
    size_t len = 0;
    if (s)
        while (*s++)
            len++;
    return len;
}

} // namespace memutils

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
