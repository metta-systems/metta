//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2011, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#define UNUSED(x)           ((void)(x))
#define UNUSED_ARG          __attribute__((unused))
#define STDCALL             __attribute__((stdcall))
#define NOINLINE            __attribute__((noinline))
#define ALWAYS_INLINE       __attribute__((always_inline))
#define PACKED              __attribute__((__packed__))
#define ALIGNED(poweroftwo) __attribute__((aligned (poweroftwo)))
#define SECTION(sect)       __attribute__((section(sect))) // add used attr
#define NEVER_RETURNS       __attribute__((noreturn))
// Uncommon optimization: functions that can be optimized out:
// Note that a function that has pointer arguments and examines the data
// pointed to must not be declared const. Likewise, a function that calls a
// non-const function usually must not be const. It does not make sense for a
// const function to return void.
#define CONST_FN            __attribute__((const))
#define EXPORT_SYMBOL(sym)  extern typeof(sym) sym

// Startup code and data not needed during runtime.
#define INIT_ONLY      SECTION(".text.init")
#define INIT_ONLY_DATA SECTION(".data.init")

#if __GNUC__ > 2
// Don't forget to -fprofile-arcs your code!
#define likely(expr)        __builtin_expect(!!(expr), 1)
#define unlikely(expr)      __builtin_expect(!!(expr), 0)
#else
#define likely(expr)        (expr)
#define unlikely(expr)      (expr)
#endif

#define KB (1000)
#define MB (1000*KB)
#define GB (1000*MB)
#define KiB (1024)
#define MiB (1024*KiB)
#define GiB (1024*MiB)

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
