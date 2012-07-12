/**
 * Some assorted string "stuff" that is largely temporary until interfaces settle a bit.
 */
#pragma once

#include "memutils.h"
#include "heap_v1_interface.h"

/**
 * Allocate string of given size on a given heap.
 */
inline char*
stralloc(size_t size, heap_v1::closure_t* heap)
{
    return reinterpret_cast<char*>(heap->allocate(size));
}

/**
 * Allocate a new copy of the string on the given heap. Similar to libc strdup.
 */
inline char*
string_copy(const char* src, heap_v1::closure_t* heap)
{
    size_t len = memutils::string_length(src) + 1;
    char* dst = stralloc(len, heap);
    memutils::copy_memory(dst, src, len);
    return dst;
}

/**
 * Allocate memory for the string on the given heap and copy exactly specified number of characters there.
 * Terminate with a null character.
 */
inline char*
string_n_copy(const char* src, size_t len, heap_v1::closure_t* heap)
{
    char* dst = stralloc(len+1, heap);
    memutils::copy_memory(dst, src, len);
    dst[len] = 0;
    return dst;
}

