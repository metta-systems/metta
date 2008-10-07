#pragma once

#include <stdint.h> // Use C99-mandated types.

#define NULL 0

typedef uint32_t size_t;
typedef uint32_t addr_t;
typedef int32_t  ptrdiff_t;
typedef uint32_t Address;

#define PACKED __attribute__((__packed__))

