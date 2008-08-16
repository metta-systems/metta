#pragma once
#ifndef __INCLUDED_TASK_H
#define __INCLUDED_TASK_H

#include "common.h"
#include "paging.h"

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


#endif
