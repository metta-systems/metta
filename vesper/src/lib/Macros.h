#pragma once

#define UNUSED(x) ((void)(x))
#define INLINE inline
#define NOINLINE __attribute__((noinline))
#define EXPORT_SYMBOL(sym) extern typeof(sym) sym
#define PACKED __attribute__((__packed__))
#define IN_SECTION(sect) __attribute__((section(sect)))
#define NORETURN __attribute__((noreturn))
// Uncommon optimization: functions that can be optimized out:
// Note that a function that has pointer arguments and examines the data pointed to must not be declared const. Likewise, a function that calls a non-const function usually must not be const. It does not make sense for a const function to return void.
#define CONST_FN __attribute__((const))

#define PANIC(msg) panic(msg, __FILE__, __LINE__);
#define ASSERT(b) ((b) ? (void)0 : panic_assert(#b, __FILE__, __LINE__))
#define BUG_ON(b) ASSERT(!(b))
