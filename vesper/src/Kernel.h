#pragma once
#ifndef __INCLUDED_KERNEL_H
#define __INCLUDED_KERNEL_H

#include "Types.h"
#include "multiboot.h"

class Kernel
{
	public:
		/**
		 * Boot the kernel
		 */
		void run(multiboot_header* mh);

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

		/**
		 * Print a full backtrace from the current location. (Or, if specified,
		 * up to n stack frames.
		 */
		void printBacktrace(Address basePointer = 0, int n = 15);

		/**
		 * Prints first n words from the stack
		 */
		void printStacktrace(unsigned int n = 64);
};

#endif
