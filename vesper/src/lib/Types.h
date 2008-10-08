#pragma once

#include <stdint.h> // Use C99-mandated types.

#define NULL 0

typedef uint32_t size_t;
typedef uint32_t addr_t;
typedef int32_t  ptrdiff_t;
typedef uint32_t Address;

#define PACKED __attribute__((__packed__))
#define IN_SECTION(sect) __attribute__((section(sect)))
#define NORETURN __attribute__((noreturn))
// Uncommon optimization: functions that can be optimized out:
// Note that a function that has pointer arguments and examines the data pointed to must not be declared const. Likewise, a function that calls a non-const function usually must not be const. It does not make sense for a const function to return void.
#define CONST_FN __attribute__((const))
