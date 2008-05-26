#ifndef __INCLUDED_COMMON_H
#define __INCLUDED_COMMON_H

#include "Types.h"
#define INLINE inline

extern "C" void *memset (void *__s, int __c, size_t __n);

// Write a byte out to the specified port.
INLINE void outb(unsigned short port, unsigned char value)
{
	asm volatile ("outb %1, %0" : : "dN" (port), "a" (value));
}

INLINE unsigned char inb(unsigned short port)
{
	unsigned char ret;
	asm volatile("inb %1, %0" : "=a" (ret) : "dN" (port));
	return ret;
}

INLINE unsigned short inw(unsigned short port)
{
	unsigned short ret;
	asm volatile ("inw %1, %0" : "=a" (ret) : "dN" (port));
	return ret;
}

#endif
