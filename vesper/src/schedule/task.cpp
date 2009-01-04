//
// Copyright 2007 - 2008, Stanislav Karchebnyy <berkus+metta@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "task.h"
#include "registers.h"
#include "memory_manager.h"
#include "default_console.h"

namespace metta {
namespace kernel {

// The currently running task.
static volatile task *current_task = 0;

// The start of the task linked list.
static volatile task *ready_queue;

// The next available process ID.
static uint32_t next_pid = 1;

void task::init()
{
	// Rather important stuff happening, no interrupts please!
	critical_section();

	// Initialise the first task (kernel task)
	current_task = ready_queue = new task;
	current_task->id = next_pid++;
	current_task->esp = current_task->ebp = 0;
	current_task->eip = 0;
	current_task->page_dir = memory_manager.get_current_directory();
	current_task->next = 0;

	kconsole.debug_log("Constructed kernel task.");

	// Reenable interrupts.
	end_critical_section();
}

void task::yield()
{
    // If we haven't initialised tasking yet, just return.
    if (!current_task)
        return;

    // Read esp, ebp now for saving later on.
    uint32_t esp, ebp, eip;
    asm volatile("mov %%esp, %0" : "=r"(esp));
    asm volatile("mov %%ebp, %0" : "=r"(ebp));

    // Read the instruction pointer. We do some cunning logic here:
    // One of two things could have happened when this function exits -
    //   (a) We called the function and it returned the EIP as requested.
    //   (b) We have just switched tasks, and because the saved EIP is essentially
    //       the instruction after read_eip(), it will seem as if read_eip has just
    //       returned.
    // In the second case we need to return immediately. To detect it we put a dummy
    // value in EAX further down at the end of this function. As C returns values in EAX,
    // it will look like the return value is this dummy value! (0x12345).
    eip = read_instruction_pointer();

    // Have we just switched tasks?
    if (eip == 0x12345)
        return;

    // No, we didn't switch tasks. Let's save some register values and switch.
    current_task->eip = eip;
    current_task->esp = esp;
    current_task->ebp = ebp;

    // Get the next task to run.
    current_task = current_task->next;
    // If we fell off the end of the linked list start again at the beginning.
    if (!current_task) current_task = ready_queue;

    eip = current_task->eip;
    esp = current_task->esp;
    ebp = current_task->ebp;

// 	kconsole.print("yield() to %d\n", current_task->id);

    // Make sure the memory manager knows we've changed page directory.
    memory_manager.set_current_directory(current_task->page_dir);
    // Here we:
    // * Stop interrupts so we don't get interrupted.
    // * Temporarily put the new EIP location in ECX.
    // * Load the stack and base pointers from the new task struct.
    // * Change page directory to the physical address (physicalAddr) of the new directory.
    // * Put a dummy value (0x12345) in EAX so that above we can recognise that we've just
    //   switched task.
    // * Restart interrupts. The STI instruction has a delay - it doesn't take effect until after
    //   the next instruction.
    // * Jump to the location in ECX (remember we put the new EIP in there).
    asm volatile("         \
      cli;                 \
      mov %0, %%ecx;       \
      mov %1, %%esp;       \
      mov %2, %%ebp;       \
      mov %3, %%cr3;       \
      mov $0x12345, %%eax; \
      sti;                 \
      jmp *%%ecx           "
	  : : "r"(eip), "r"(esp), "r"(ebp), "r"(current_task->page_dir->getPhysical()));
}

task *task::self()
{
	return (task *)current_task;
}

int task::fork()
{
    // We are modifying kernel structures, and so cannot be interrupted.
	disable_interrupts();

    // Take a pointer to this process' task struct for later reference.
    task *parent_task = (task *)current_task;

    // Clone the address space.
    page_directory *directory = memory_manager.get_current_directory()->clone();

    // Create a new process.
    task *new_task = new task;

    new_task->id = next_pid++;
    new_task->esp = new_task->ebp = 0;
    new_task->eip = 0;
    new_task->page_dir = directory;
    new_task->next = 0;

	kconsole.print("fork() to %d\n", new_task->id);

    // Add it to the end of the ready queue.
    task *tmp_task = (task *)ready_queue;
    while (tmp_task->next)
        tmp_task = tmp_task->next;
    tmp_task->next = new_task;

    // This will be the entry point for the new process.
    uint32_t eip = read_instruction_pointer();

    // We could be the parent or the child here - check.
    if (current_task == parent_task)
    {
        // We are the parent, so set up the esp/ebp/eip for our child.
        uint32_t esp; asm volatile("mov %%esp, %0" : "=r"(esp));
        uint32_t ebp; asm volatile("mov %%ebp, %0" : "=r"(ebp));
        new_task->esp = esp;
        new_task->ebp = ebp;
        new_task->eip = eip;

		kconsole.print("Saving new task's EIP %08x\n", eip);
		enable_interrupts();

        return new_task->id;
    }
    else
    {
        // We are the child.
        return 0;
    }
}

int task::getpid()
{
    return id;
}

}
}

// kate: indent-width 4; replace-tabs on;
// vi:set ts=4:set expandtab=on:
