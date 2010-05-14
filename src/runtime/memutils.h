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

/*!
 * @brief Memory utilities similar to standard libc operations.
 */
namespace memutils {

/*!
 * Fill a region of memory with the given value.
 * @param[out] dest  Pointer to the start of the area.
 * @param[in]  value The byte to fill the area with
 * @param[in]  count The size of the area.
 * @return           Pointer to the start of the area.
 *
 * @warning Do not use fill_memory() to access IO space, use fill_io_memory() instead.
 */
void* fill_memory(void* dest, int value, size_t count);

/*!
 * Copy one area of memory to another.
 * @param[out] dest  Where to copy to
 * @param[in]  src   Where to copy from
 * @param[in]  count The size of the area.
 * @return           Pointer to the start of the destination area.
 *
 * @warning You should not use this function to access IO space, use copy_memory_to_io()
 * or copy_memory_from_io() instead.
 */
void* copy_memory(void* dest, const void* src, size_t count);

/*!
 * Copy one area of memory to another.
 * @param[out] dest  Where to copy to
 * @param[in]  src   Where to copy from
 * @param[in]  count The size of the area.
 * @return           Pointer to the start of the destination area.
 *
 * Unlike copy_memory(), this function copes with overlapping areas.
 */
void* move_memory(void* dest, const void* src, size_t count);

/*!
 * Compare two regions of memory.
 * @param[in]  left  First memory region.
 * @param[in]  right Second memory region.
 * @param[in]  count Number of bytes to compare.
 * @return           @c true if memory regions are equal, @c false otherwise.
 */
bool is_memory_equal(const void* left, const void* right, size_t count);

/*!
 * Compare two regions of memory.
 * @param[in]  left  First memory region.
 * @param[in]  right Second memory region.
 * @param[in]  count Number of bytes to compare.
 * @return     result of lexicographical memory compare, -1 if left is less than right, 0 if they are equal or 1 if left is greater than right.
 */
int memory_difference(const void* left, const void* right, size_t count);

/*!
 * Compare two null-terminated strings.
 * @param[in] s1 One string
 * @param[in] s2 Another string
 * @return true if @c s1 equals @c s2, false if strings are different.
 */
bool is_string_equal(const char *s1, const char *s2);

/*!
 * Return size of a null-terminated string.
 * @param[in] s  Null-terminated string.
 * @returns      String length in bytes.
 */
size_t string_length(const char *s);

/*!
 * Copy one string to another location.
 * @param[out] dest  Where to copy to
 * @param[in]  src   Where to copy from
 * @return           Pointer to the start of the destination string.
 */
char* copy_string(char* dest, const char* src);

} // namespace memutils

// #if __Metta__ && defined(__GNUC__)
// extern "C" void* memmove(void* dest, const void* src, size_t count);
// #endif

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
