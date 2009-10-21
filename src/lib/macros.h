//
// Copyright 2007 - 2009, Stanislav Karchebnyy <berkus+metta@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#define UNUSED(x)           ((void)(x))
#define UNUSED_ARG          __attribute__((unused))
#define NOINLINE            __attribute__((noinline))
#define PACKED              __attribute__((__packed__))
#define ALIGNED(poweroftwo) __attribute__((aligned (poweroftwo)))
#define SECTION(sect)       __attribute__((section(sect)))
#define NORETURN            __attribute__((noreturn))
// Uncommon optimization: functions that can be optimized out:
// Note that a function that has pointer arguments and examines the data
// pointed to must not be declared const. Likewise, a function that calls a
// non-const function usually must not be const. It does not make sense for a
// const function to return void.
#define CONST_FN            __attribute__((const))
#define EXPORT_SYMBOL(sym)  extern typeof(sym) sym

#define KB (1000)
#define MB (1000*1000)
#define GB (1000*1000*1000)
#define KiB (1024)
#define MiB (1024*1024)
#define GiB (1024*1024*1024)

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
