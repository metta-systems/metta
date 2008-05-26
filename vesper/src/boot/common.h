#ifndef __INCLUDED_COMMON_H
#define __INCLUDED_COMMON_H

#include "Types.h"
#define INLINE inline

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
