//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2012, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "types.h"

/**
 * @brief Memory utilities similar to standard libc operations.
 * @todo Quite unoptimized at the moment, memory ops should be significantly slow.
 * Easy optimization would be __builtin_memcpy() and friends.
 */
namespace memutils {

/**
 * Fill a region of memory with the given value.
 * @param[out] dest  Pointer to the start of the area.
 * @param[in]  value The byte to fill the area with
 * @param[in]  count The size of the area.
 * @return           Pointer to the start of the area.
 *
 * @warning Do not use fill_memory() to access IO space, use fill_io_memory() instead.
 */
//Clearing 0x00040000 bytes at 0x009e2000 - this errored out in unrolled version, looks like counter wrapped. Need UTest.
inline void* fill_memory(void* dest, int value, size_t count)
{
    asm volatile ("rep stosb" :: "c"(count), "a"(value), "D"(dest) : "memory");
    return dest;
}

/**
 * Clear a region of memory. Usually sets given area to zero, but a debug implementation might want to clear with
 * a known pattern or do something else.
 * @param[out] dest  Pointer to the start of the area.
 * @param[in]  count The size of the area.
 * @return           Pointer to the start of the area.
 *
 * @warning Do not use clear_memory() to access IO space, use fill_io_memory() with a value of 0 instead.
 */
inline void* clear_memory(void* dest, size_t count)
{
    return fill_memory(dest, 0, count);
}

/**
 * Copy one area of memory to another.
 * @param[out] dest  Where to copy to
 * @param[in]  src   Where to copy from
 * @param[in]  count The size of the area.
 * @return           Pointer to the start of the destination area.
 *
 * @warning You should not use this function to access IO space, use copy_memory_to_io()
 * or copy_memory_from_io() instead.
 */
inline void* copy_memory(void* dest, const void* src, size_t count)
{
    char *tmp = reinterpret_cast<char*>(dest);
    const char *s = reinterpret_cast<const char*>(src);

    while (count--)
        *tmp++ = *s++;
    return dest;
}

inline address_t copy_memory(address_t dest, address_t src, size_t count)
{
    return reinterpret_cast<address_t>(copy_memory(reinterpret_cast<void*>(dest), reinterpret_cast<const void*>(src), count));
}

/**
 * Copy one area of memory to another.
 * @param[out] dest  Where to copy to
 * @param[in]  src   Where to copy from
 * @param[in]  count The size of the area.
 * @return           Pointer to the start of the destination area.
 *
 * Unlike copy_memory(), this function copes with overlapping areas.
 */
inline void* move_memory(void* dest, const void* src, size_t count)
{
    char *tmp;
    const char *s;

    if (dest <= src) {
        copy_memory(dest, src, count);
    } else {
        tmp = reinterpret_cast<char*>(dest) + count;
        s = reinterpret_cast<const char*>(src) + count;
        while (count--)
            *--tmp = *--s;
    }
    return dest;
}

/**
 * Compare two regions of memory.
 * @param[in]  left  First memory region.
 * @param[in]  right Second memory region.
 * @param[in]  count Number of bytes to compare.
 * @return     @c true if memory regions are equal, @c false otherwise.
 */
inline bool is_memory_equal(const void* left, const void* right, size_t count)
{
    const char* ltmp = reinterpret_cast<const char*>(left);
    const char* rtmp = reinterpret_cast<const char*>(right);

    while (count--)
        if (*ltmp++ != *rtmp++)
            return false;
    return true;
}

/**
 * Compare two regions of memory.
 * @param[in]  left  First memory region.
 * @param[in]  right Second memory region.
 * @param[in]  count Number of bytes to compare.
 * @return     result of lexicographical memory compare, -1 if left is less than right, 0 if they are equal or 1 if left is greater than right.
 */
inline int memory_difference(const void* left, const void* right, size_t count)
{
    signed char __res;

    const char* ltmp = reinterpret_cast<const char*>(left);
    const char* rtmp = reinterpret_cast<const char*>(right);

    while (count--)
        if ((__res = *ltmp++ - *rtmp++) != 0)
            return __res;

    return 0;
}

/**
 * Compare two null-terminated strings.
 * @param[in] s1 One string
 * @param[in] s2 Another string
 * @return true if @c s1 equals @c s2, false if strings are different.
 */
inline bool is_string_equal(const char *s1, const char *s2)
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

/**
 * Return size of a null-terminated string.
 * @param[in] s  Null-terminated string.
 * @returns      String length in bytes.
 */
inline size_t string_length(const char *s)
{
    size_t len = 0;
    if (s)
        while (*s++)
            len++;
    return len;
    // return __builtin_strlen(s); // Causes undefined reference to `strlen'
}

/**
 * Copy one string to another location.
 * @param[out] dest  Where to copy to
 * @param[in]  src   Where to copy from
 * @param[in]  max_length Maximum number of bytes to copy, unlimited if 0.
 * @return           Pointer to the start of the destination string.
 */
inline char* copy_string(char* dest, const char* src, size_t max_length = 0)
{
    if (!src || !dest)
        return 0;
    size_t src_length = string_length(src) + 1;
    size_t length = max_length == 0 ? src_length
                  : max_length < src_length ?
                  max_length
                  : src_length;
    copy_memory(dest, src, length);
    return dest;
}

} // namespace memutils
