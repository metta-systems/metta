#ifndef __INCLUDED_COMMON_H
#define __INCLUDED_COMMON_H

#include "Types.h"

#define UNUSED(x) ((void)(x))
#define INLINE inline
#define PANIC(msg) panic(msg, __FILE__, __LINE__);
#define ASSERT(b) ((b) ? (void)0 : panic_assert(#b, __FILE__, __LINE__))

extern "C" void panic(const char *message, const char *file, uint32_t line);
extern "C" void panic_assert(const char *desc, const char *file, uint32_t line);

extern "C" void *memset (void *__s, int __c, size_t __n);

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

#endif
