//
// Copyright 2007 - 2009, Stanislav Karchebnyy <berkus+metta@madfire.net>
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

int    memutils::memcmp(const void *s1, const void *s2, size_t count)
{
    const unsigned char *su1, *su2;
    int res = 0;

    for (su1 = (const unsigned char *)s1, su2 = (const unsigned char *)s2; 0 < count; ++su1, ++su2, count--)
        if ((res = *su1 - *su2) != 0)
            break;
    return res;
}

void*  memutils::memchr(const void *s, int c, size_t n)
{
    const unsigned char *p = (const unsigned char *)s;
    while (n-- != 0) {
        if ((unsigned char)c == *p++) {
            return (void *)(p - 1);
        }
    }
    return NULL;
}

size_t memutils::strlen(const char *s)
{
    const char *sc;

    for (sc = s; *sc != '\0'; ++sc)
        /* nothing */;
    return sc - s;
}

int  memutils::strcmp(const char *s1, const char *s2)
{
    signed char __res;

    while (1) {
        if ((__res = *s1 - *s2++) != 0 || !*s1++)
            break;
    }
    return __res;
}

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
