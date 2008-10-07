#pragma once

#include "Types.h"

class Kernel
{
public:
	/**
	 * Boot the kernel
	 */
	void run() /*NORETURN*/;

	/**
	 * Dump @c size bytes from a memory region starting at virtual address @c start.
	 */
	static void dumpMemory(Address start, size_t size);

	/**
	 * Given a stack base pointer, follow it, return the next stack base
	 * pointer and also return the instruction pointer it returned to.
	 */
	static Address backtrace(Address basePointer, Address& returnAddress);

	/**
	 * Given the current stack, follow 'n' backtraces and return the
	 * return address found there.
	 */
	static Address backtrace(int n);

	inline static bool strEquals(const char *in1, const char *in2)
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
	 * memcpy - Copy one area of memory to another
	 * @dest: Where to copy to
	 * @src: Where to copy from
	 * @count: The size of the area.
	 *
	 * You should not use this function to access IO space, use memcpy_toio()
	 * or memcpy_fromio() instead.
	 */
	inline static void* copyMemory(void* dest, const void* src, size_t count)
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
	inline static void* moveMemory(void* dest, const void* src, size_t count)
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
	void printBacktrace(Address basePointer = 0, int n = 0);

	/**
	 * Prints first n words from the stack
	 */
	void printStacktrace(unsigned int n = 64);

private:
	void relocatePlacementAddress();
};

