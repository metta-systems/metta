//
// Implementation of memory manipulation utilities for case when there's no standard C library.
//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2010, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "memutils.h"

// Stupid GCC generates memmove() in some iterator assignments in advance_ex instead of inlining (doh)
#if __Metta__ && defined(__GNUC__)
extern "C" void* memmove(void* dest, const void* src, size_t count)
{
    return memutils::move_memory(dest, src, count);
}
#endif

namespace memutils
{

void* fill_memory(void* dest, int value, size_t count)
{
    char *xs = reinterpret_cast<char*>(dest);
    while (count--)
        *xs++ = value;
    return dest;
}

// stdlib compat for compiler
extern "C" void* memcpy(void* dest, const void* src, size_t count) { return copy_memory(dest, src, count); }

void* copy_memory(void* dest, const void* src, size_t count)
{
    char *tmp = reinterpret_cast<char*>(dest);
    const char *s = reinterpret_cast<const char*>(src);

    while (count--)
        *tmp++ = *s++;
    return dest;
}


// Wow, ugly!
address_t copy_memory(address_t dest, address_t src, size_t count)
{
    return (address_t)copy_memory((void*)dest, (const void*)src, count);
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

int memory_difference(const void* left, const void* right, size_t count)
{
    signed char __res;

    const char* ltmp = reinterpret_cast<const char*>(left);
    const char* rtmp = reinterpret_cast<const char*>(right);

    while (count--)
        if ((__res = *ltmp++ - *rtmp++) != 0)
            return __res;

    return 0;
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

char* copy_string(char* dest, const char* src, size_t max_length)
{
    if (!src || !dest)
        return 0;
    size_t length = max_length == 0 ? string_length(src) + 1
                  : max_length < string_length(src) + 1 ?
                  max_length
                  : string_length(src) + 1;
    copy_memory(dest, src, length);
    return dest;
}

} // namespace memutils

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
