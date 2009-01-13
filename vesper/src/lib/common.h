//
// Copyright 2007 - 2008, Stanislav Karchebnyy <berkus+metta@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

// [x] Use bstrlib for safe string operations.

#include "types.h"

extern "C" void panic(const char *message, const char *file, uint32_t line);
extern "C" void panic_assert(const char *desc, const char *file, uint32_t line);

/**
* Write a byte out to the specified port.
* a puts value in eax, dN puts port in edx or uses 1-byte constant.
**/
static inline void outb(uint16_t port, uint8_t value)
{
    asm volatile ("outb %0, %1" :: "a" (value), "dN" (port));
}

/// Write a word out to the specified port.
static inline void outw(uint16_t port, uint16_t value)
{
    asm volatile ("outw %0, %1" :: "a" (value), "dN" (port));
}

static inline uint8_t inb(uint16_t port)
{
	uint8_t ret;
	asm volatile("inb %1, %0" : "=a" (ret) : "dN" (port));
	return ret;
}

static inline uint16_t inw(uint16_t port)
{
	uint16_t ret;
	asm volatile ("inw %1, %0" : "=a" (ret) : "dN" (port));
	return ret;
}

static inline void rdtsc(uint32_t* upper, uint32_t* lower)
{
    asm volatile("rdtsc" : "=a"(*lower), "=d"(*upper));
}

// kate: indent-width 4; replace-tabs on;
// vi:set ts=4:set expandtab=on:
