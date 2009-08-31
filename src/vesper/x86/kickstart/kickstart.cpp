//
// Copyright 2007 - 2009, Stanislav Karchebnyy <berkus+metta@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
/*!
@file kickstart.cpp
@brief Prepare kernel and init component for starting up.
@ingroup Bootup

* put kernel in place
  - set up minimal paging,
  - map higher-half kernel to K_SPACE_START,
* init memory manager
  - map memory pages for paged mode, mapping bios 1-1 and kernel to highmem,
  - enter paged mode,
* activate startup servers
  - component constructors run in kernel mode, have the ability to set up their system tables etcetc,
    * vm_server
      - give vm_server current mapping situation and active memory map
      - further allocations will go on from vm_server
    * scheduler
    * portal_manager
    * interrupt_dispatcher
    * trader
    * security_server
* enter root_server
  - root_server can then use virtual memory, threads etc
  - root_server expects cpu to be in paging mode with pagedir in highmem, kernel
  mapped high and exception handlers set up in low mem - these will be relocated
  and reinstated by interrupts component when it's loaded,
  - verify required components are present in initfs (pmm, cpu, interrupts,
  security manager, portal manager, object loader),
  - set up security contexts and permissions
  - boot other cpus if present,
  - switch to usermode and continue execution in the init process,
* activate extra servers
  - root filesystem mounter,
  - hardware detector,
*/
#include "memutils.h"
#include "multiboot.h"
#include "memory.h" // boot_pmm_allocator
#include "gdt.h"
#include "idt.h"
#include "default_console.h"
#include "elf_parser.h"
#include "registers.h"
#include "initfs.h"
#include "mmu.h"
#include "c++ctors.h"
#include "debugger.h"
//{ DEBUG STUFF
#include "page_fault_handler.h"
page_fault_handler_t page_fault_handler;
//}
boot_pmm_allocator init_memmgr;
interrupt_descriptor_table_t interrupts_table;

extern "C" void kickstart(multiboot_t::header_t* mbh);
extern "C" address_t placement_address;
extern "C" address_t KICKSTART_BASE;
extern "C" address_t initial_esp; // in loader.s


//! Remap stack for paging mode.
/*!
* Allocate enough pages to fit existing stack frame, copy data from old stack
* and switch over to a new stack.
* @todo Allocated stack pages are 1-1 mapped currently, but probably should be mapped
* to some reserved stack area?
* Changes the original stack given by the bootloader to one at
* a virtual memory location defined at compile time.
*/
static void remap_stack()
{
    address_t old_stack_pointer = read_stack_pointer();
    address_t old_base_pointer  = read_base_pointer();

    size_t    stack_size   = initial_esp - old_stack_pointer;
    size_t    stack_npages = page_align_up<address_t>(stack_size) / PAGE_SIZE;
    address_t stack_page   = init_memmgr.alloc_next_page();
    init_memmgr.mapping_enter(stack_page, stack_page);
    for (size_t i = 1; i < stack_npages; i++)
    {
        address_t p = init_memmgr.alloc_next_page();
        init_memmgr.mapping_enter(p, p);
    }

    ia32_mmu_t::flush_page_directory();

    int       offset            = stack_page + stack_npages * PAGE_SIZE - initial_esp;
    address_t new_stack_pointer = old_stack_pointer + offset;
    address_t new_base_pointer  = old_base_pointer + offset;

    kconsole.print("Copying stack from %p-%p to %p-%p (%d bytes)..", old_stack_pointer, initial_esp, new_stack_pointer, new_stack_pointer + stack_size, stack_size);

    memutils::copy_memory((void*)new_stack_pointer, (const void*)old_stack_pointer, stack_size);

    write_stack_pointer(new_stack_pointer);
    write_base_pointer(new_base_pointer);

    kconsole << "done. Activated new stack." << endl;
}

//! Prepare and boot system.
/*!
This part starts in protected mode, linear == physical, paging is off.

As a rule: whatever we intend to bring to paging mode, we copy to an allocated frame
ourselves.
*/
void kickstart(multiboot_t::header_t* mbh)
{
    run_global_ctors();

    multiboot_t mb(mbh);

    ASSERT(mb.module_count() == 2);

    multiboot_t::modinfo_t* kernel = mb.module(0); // nucleus
    multiboot_t::modinfo_t* bootcp = mb.module(1); // bootcomps
    ASSERT(kernel);
    ASSERT(bootcp);

    // Setup small pmm allocator.
    init_memmgr.adjust_alloced_start((address_t)&placement_address);
    init_memmgr.adjust_alloced_start(kernel->mod_end);
    init_memmgr.adjust_alloced_start(bootcp->mod_end);
    // now we can allocate extra pages from alloced_start using alloc_next_page()

    kconsole << GREEN << "lower mem = " << (int)mb.lower_mem() << "KB" << endl
                      << "upper mem = " << (int)mb.upper_mem() << "KB" << endl;

    kconsole << WHITE << "We are loaded at " << (address_t)&KICKSTART_BASE << endl
                      << "kernel module at " << kernel->mod_start << ", end " << kernel->mod_end << endl
                      << "bootcp module at " << bootcp->mod_start << ", end " << bootcp->mod_end << endl
                      << "Alloctn start at " << init_memmgr.get_alloced_start() << endl;

    init_memmgr.setup_pagetables();

    uint32_t k;

    elf_parser elf;
    if (!elf.load_image(kernel->mod_start, kernel->mod_end - kernel->mod_start, &init_memmgr))
        kconsole << RED << "kernel NOT loaded (bad)" << endl;
    else
        kconsole << GREEN << "kernel loaded (ok)" << endl;

    // identity map start of initcp so we can access header_ from paging mode.
    init_memmgr.mapping_enter(bootcp->mod_start, bootcp->mod_start);

    // Identity map currently executing code.
    address_t ph_start = (address_t)&KICKSTART_BASE;
    address_t ph_end   = page_align_up<address_t>((address_t)&placement_address); // one after end
    kconsole << endl << "Mapping loader: ";
    for (k = ph_start/PAGE_SIZE; k < ph_end/PAGE_SIZE; k++)
    {
        init_memmgr.mapping_enter(k * PAGE_SIZE, k * PAGE_SIZE);
    }

    // Identity map video memory/bios area.
    ph_start = (address_t)0xa0000;
    ph_end   = 0x100000; // one after end
    kconsole << endl << "Mapping VRAM: ";
    for (k = ph_start/PAGE_SIZE; k < ph_end/PAGE_SIZE; k++)
    {
        init_memmgr.mapping_enter(k * PAGE_SIZE, k * PAGE_SIZE);
    }

    kconsole << endl << "Mapping multiboot info: ";
    address_t a = page_align_down<address_t>(mbh);
    init_memmgr.mapping_enter(a, a);

    kconsole << endl << "Mapping modules list: ";
    a = page_align_down<address_t>(mb.module(0));
    init_memmgr.mapping_enter(a, a);

    kconsole << endl << "Mapped." << endl;

    global_descriptor_table_t gdt;
    kconsole << "Created gdt." << endl;

    interrupts_table.set_isr_handler(14, &page_fault_handler);
    interrupts_table.install();

    remap_stack();

    // Print mmap
//     mb.memory_map()->dump();
    multiboot_t::mmap_entry_t* mmi = mb.memory_map()->first_entry();
    kconsole.print("MMAP is provided\n");
    while (mmi)
    {
        kconsole << "[entry @ " << (uint32_t)mmi << "]  " << (uint32_t)mmi->address() << ", " << (int32_t)mmi->size() << " bytes, type " <<  (mmi->is_free() ? "Free" : "Occupied") << endl;
        // We will need to access mmap from paging mode
        init_memmgr.mapping_enter((address_t)mmi, (address_t)mmi);
        mmi = mb.memory_map()->next_entry(mmi);
    }

    // Get kernel entry before enabling paging, as this area won't be mapped.
    typedef void (*kernel_entry)(address_t mem_end, multiboot_t::mmap_t* mmap);
    kernel_entry init_nucleus = (kernel_entry)elf.get_entry_point();

    // TODO: We've allocated memory from a contiguous region, mark it and modify
    // bootloader memory map to exclude this region as occupied.

    init_memmgr.start_paging();

    /// here we have paging enabled and can call kernel functions
    /// start by creating PDs for boot_components and load them in their PDs
    /// fill in startup portals code

    /// call vm_server.init(mbh->mmap, current_pdir)
    kconsole << RED << "going to init nucleus" << endl;
    init_nucleus(0/*mem_end*/, mb.memory_map());
    kconsole << GREEN << "done, instantiating components" << endl;

    // Load components from bootcp.
    initfs_t initfs(bootcp->mod_start);
    k = 0;

    while (k < initfs.count())
    {
        kconsole << YELLOW << "boot component " << initfs.get_file_name(k) << " @ " << initfs.get_file(k) << endl;
        if (!elf.load_image(initfs.get_file(k), initfs.get_file_size(k), &init_memmgr))
            kconsole << RED << "not an ELF file, load failed" << endl;
        k += 1;
    }

    kconsole << WHITE << "...in the living memory of V2_OS" << endl;

    kconsole << "need " << (address_t)bootcp << " mapped" << endl;
    ASSERT(init_memmgr.mapping_entered((address_t)bootcp));

    /* Never reached */
    PANIC("root_server returned!");
}

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
