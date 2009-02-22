//
// Copyright 2007 - 2009, Stanislav Karchebnyy <berkus+metta@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "task.h"
#include "types.h"
#include "registers.h"
#include "memory_manager.h"
#include "default_console.h"

namespace metta {
namespace kernel {

// The next available process ID.
static uint32_t next_pid = 1;
static task* kernel_task = NULL;

void task::init()
{
    // Rather important stuff happening, no interrupts please!
    critical_section();

    // Initialise the first task (kernel task)
    kernel_task = /*ready_queue =*/ new task;
    kernel_task->id = next_pid++;
    assert(kernel_task->id == 1);
    kernel_task->page_dir = kmemmgr.get_current_directory();
    kernel_task->next = 0;

    kconsole.debug_log("Constructed kernel task.");

    end_critical_section();
}

/* void task::yield()
{
    // Make sure the memory manager knows we've changed page directory.
    kmemmgr.set_current_directory(current_task->page_dir);
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
        : : "r"(eip), "r"(esp), "r"(ebp), "r"(current_task->page_dir->get_physical()));
}*/

task* task::clone()
{
    // We are modifying kernel structures, and so cannot be interrupted.
    critical_section();

    // Clone the address space.
    page_directory *directory = kmemmgr.get_current_directory()->clone();

    // Create a new process.
    task *new_task = new task;

    new_task->id = next_pid++;
    new_task->page_dir = directory;
    new_task->next = 0;

    end_critical_section();

    return new_task;
}

}
}

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
