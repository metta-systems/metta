#ifndef __INCLUDED_TYPES_H
#define __INCLUDED_TYPES_H

#include <stdint.h> // Use C99-mandated types.
#define INLINE inline

typedef uint32_t size_t;

extern void *memset (void *__s, int __c, size_t __n);

#endif
