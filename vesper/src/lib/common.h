//
// Copyright 2007 - 2008, Stanislav Karchebnyy <berkus+metta@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

// [ ] Use djb's safe string operations?

#include "Types.h"
#include "Macros.h"
#include <stdarg.h> // for va_list macros

extern "C" void panic(const char *message, const char *file, uint32_t line);
extern "C" void panic_assert(const char *desc, const char *file, uint32_t line);

// Write a byte out to the specified port.
INLINE void outb(uint16_t port, uint8_t value)
{
	asm volatile ("outb %1, %0" : : "dN" (port), "a" (value));
}

INLINE uint8_t inb(uint16_t port)
{
	uint8_t ret;
	asm volatile("inb %1, %0" : "=a" (ret) : "dN" (port));
	return ret;
}

INLINE uint16_t inw(uint16_t port)
{
	uint16_t ret;
	asm volatile ("inw %1, %0" : "=a" (ret) : "dN" (port));
	return ret;
}

// ==================================================================
// from vsprintf.c
// FIXME: do we need to export this uglyness at all?

extern "C" unsigned long simple_strtoul(const char *,char **,unsigned int);
extern "C" long simple_strtol(const char *,char **,unsigned int);
extern "C" unsigned long long simple_strtoull(const char *,char **,unsigned int);
extern "C" long long simple_strtoll(const char *,char **,unsigned int);
extern "C" int strict_strtoul(const char *, unsigned int, unsigned long *);
extern "C" int strict_strtol(const char *, unsigned int, long *);
extern "C" int strict_strtoull(const char *, unsigned int, unsigned long long *);
extern "C" int strict_strtoll(const char *, unsigned int, long long *);
extern "C" int sprintf(char * buf, const char * fmt, ...)
        __attribute__ ((format (printf, 2, 3)));
extern "C" int vsprintf(char *buf, const char *, va_list)
        __attribute__ ((format (printf, 2, 0)));
extern "C" int snprintf(char * buf, size_t size, const char * fmt, ...)
        __attribute__ ((format (printf, 3, 4)));
extern "C" int vsnprintf(char *buf, size_t size, const char *fmt, va_list args)
        __attribute__ ((format (printf, 3, 0)));
extern "C" int scnprintf(char * buf, size_t size, const char * fmt, ...)
        __attribute__ ((format (printf, 3, 4)));
extern "C" int vscnprintf(char *buf, size_t size, const char *fmt, va_list args)
        __attribute__ ((format (printf, 3, 0)));
// extern "C" char *kasprintf(gfp_t gfp, const char *fmt, ...)
//         __attribute__ ((format (printf, 2, 3)));
// extern "C" char *kvasprintf(gfp_t gfp, const char *fmt, va_list args);

extern "C" int sscanf(const char *, const char *, ...)
        __attribute__ ((format (scanf, 2, 3)));
extern "C" int vsscanf(const char *, const char *, va_list)
        __attribute__ ((format (scanf, 2, 0)));

