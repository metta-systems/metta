#pragma once

#include "types.h"

namespace metta {
namespace common {

class memutils
{
public:
    /**
    * Fill a region of memory with the given value
    * @p dest Pointer to the start of the area.
    * @p value The byte to fill the area with
    * @p count The size of the area.
    *
    * Do not use fill_memory() to access IO space, use fill_io_memory() instead.
    **/
    static void* fill_memory(void* dest, int value, size_t count);

    /**
    * Copy one area of memory to another
    * @p dest Where to copy to
    * @p src Where to copy from
    * @p count The size of the area.
    *
    * You should not use this function to access IO space, use copy_memory_to_io()
    * or copy_memory_from_io() instead.
    **/
    static void* copy_memory(void* dest, const void* src, size_t count);

    /**
    * Copy one area of memory to another
    * @p dest Where to copy to
    * @p src Where to copy from
    * @p count The size of the area.
    *
    * Unlike copy_memory(), move_memory() copes with overlapping areas.
    **/
    static void* move_memory(void* dest, const void* src, size_t count);
    // These functions are needed for bstrlib
    /**
    * Compare two areas of memory
    * @p s1 One area of memory
    * @p s2 Another area of memory
    * @p n  The size of the area.
    **/
    static int    memcmp(const void *s1, const void *s2, size_t n);
    /**
    * Find a character in an area of memory.
    * @p s The memory area
    * @p c The byte to search for
    * @p n The size of the area.
    *
    * @returns the address of the first occurrence of @p c, or NULL
    * if @p c is not found
    **/
    static void*  memchr(const void *s, int c, size_t n);
    /**
    * Find the length of a string not including the terminating \0
    * @p s The string to be sized
    **/
    static size_t strlen(const char *s);
    /**
    * Compare two strings
    * @p s1 One string
    * @p s2 Another string
    **/
    static int strcmp(const char *s1, const char *s2);
};

}
}
