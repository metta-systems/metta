//
// Copyright 2007 - 2008, Stanislav Karchebnyy <berkus+metta@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "Types.h"
#include "Macros.h"

class kernel
{
public:
	/**
	 * Boot the kernel
	 */
	void run() NORETURN;

	/**
	 * Dump @c size bytes from a memory region starting at virtual address @c start.
	 */
	static void dump_memory(address_t start, size_t size);

	/**
	 * Given a stack base pointer, follow it, return the next stack base
	 * pointer and also return the instruction pointer it returned to.
	 */
	static address_t backtrace(address_t base_pointer, address_t& return_address);

	/**
	 * Given the current stack, follow 'n' backtraces and return the
	 * return address found there.
	 */
	static address_t backtrace(int n);

	// TODO: move to string class
	inline static bool str_equals(const char *in1, const char *in2)
	{
		char *left = (char *)in1;
		char *right = (char *)in2;
		while(*left && *right && *left == *right)
			left++, right++;
		if (*left != *right)
			return false;
		return true;
	}

	/**
	 * memset - Fill a region of memory with the given value
	 * @s: Pointer to the start of the area.
	 * @c: The byte to fill the area with
	 * @count: The size of the area.
	 *
	 * Do not use memset() to access IO space, use memset_io() instead.
	 */
	INLINE static void* set_memory(void* dest, int value, size_t count)
	{
		char *xs = (char *)dest;
		while (count--)
			*xs++ = value;
		return dest;
	}

	/**
	 * memcpy - Copy one area of memory to another
	 * @dest: Where to copy to
	 * @src: Where to copy from
	 * @count: The size of the area.
	 *
	 * You should not use this function to access IO space, use memcpy_toio()
	 * or memcpy_fromio() instead.
	 */
	INLINE static void* copy_memory(void* dest, const void* src, size_t count)
	{
		char *tmp = (char *)dest;
		const char *s = (const char *)src;

		while (count--)
			*tmp++ = *s++;
		return dest;
	}

	/**
	 * memmove - Copy one area of memory to another
	 * @dest: Where to copy to
	 * @src: Where to copy from
	 * @count: The size of the area.
	 *
	 * Unlike memcpy(), memmove() copes with overlapping areas.
	 */
	INLINE static void* move_memory(void* dest, const void* src, size_t count)
	{
		char *tmp;
		const char *s;

		if (dest <= src) {
			tmp = (char *)dest;
			s = (const char *)src;
			while (count--)
				*tmp++ = *s++;
		} else {
			tmp = (char *)dest;
			tmp += count;
			s = (const char *)src;
			s += count;
			while (count--)
				*--tmp = *--s;
		}
		return dest;
	}

	/**
	 * Print a full backtrace from the current location. (Or, if @p n is specified,
	 * up to n stack frames.
	 */
	void print_backtrace(address_t base_pointer = 0, int n = 0);

	/**
	 * Prints first n words from the stack
	 */
	void print_stacktrace(unsigned int n = 64);

private:
	void relocate_placement_address();
};

// kate: indent-width 4; replace-tabs on;
// vi:set ts=4:set expandtab=on:
