//
// Copyright 2007 - 2008, Stanislav Karchebnyy <berkus+metta@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "Types.h"

class PageDirectory;

// This structure defines a 'task' - a process.
class Task
{
	public:
		static Task *self();

		static void init();// Initialises the tasking system.
		static void yield();// Called by the timer hook, this changes the running process.

		// Forks the current process, spawning a new one with a different
		// memory space.
		int fork();

		// Returns the pid of the current process.
		int getpid();

	private:
		int id;                        // Process ID.
		uint32_t esp, ebp;             // Stack and base pointers.
		uint32_t eip;                  // Instruction pointer.
		PageDirectory *page_directory; // Page directory.
		Task *next;                    // The next task in a linked list.
};

// kate: indent-width 4; replace-tabs on;
// vi:set ts=4:set expandtab=on:
