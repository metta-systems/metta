//
// Copyright 2007 - 2009, Stanislav Karchebnyy <berkus+metta@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
/*!
Prepare kernel and init component for starting up.

 * unpack kernel.assembly in-place and copy appropriate kernel to mapped KERNEL_BASE.
   - TODO: use device tree from bootloader to figure out what kernel to use
 * unpack initrd to 0x1000
 * set up paging
 * jump to initrd entrypoint

TODO:
debug_console = (debug_on ? kconsole : bochs_console/null_console)
pass debug_on via grub cmdline
*/
#include "memutils.h"
#include "multiboot.h"
#include "memory.h"
#include "global_descriptor_table.h"
#include "interrupt_descriptor_table.h"
#include "default_console.h"
#include "elf_parser.h"
#include "registers.h"

//{ DEBUG STUFF
#include "page_fault_handler.h"
page_fault_handler page_fault_handler_;
//}
boot_pmm_allocator init_memmgr;
interrupt_descriptor_table interrupts_table;

extern "C" {
    void setup_kernel(multiboot::header *mbh);

    address_t placement_address;
    address_t KERNEL_BASE;
}

extern "C" address_t initial_esp; // in loader.s

/*!
* Remap stack for paging mode.
*/
void remap_stack()
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

    // Flush the TLB
    flush_page_directory();

    int       offset            = stack_page + stack_npages * PAGE_SIZE - initial_esp;
    address_t new_stack_pointer = old_stack_pointer + offset;
    address_t new_base_pointer  = old_base_pointer + offset;

    kconsole.print("Copying stack from %p-%p to %p-%p (%d bytes)..", old_stack_pointer, initial_esp, new_stack_pointer, new_stack_pointer + stack_size, stack_size);

    memutils::copy_memory((void*)new_stack_pointer, (const void*)old_stack_pointer, stack_size);

    write_stack_pointer(new_stack_pointer);
    write_base_pointer(new_base_pointer);

    kconsole << "done. Activated new stack." << endl;
}

typedef void (*ctorfn)();
extern ctorfn start_ctors; // defined by linker
extern ctorfn end_ctors;

// This part starts in protected mode, linear == physical, paging is off.
void setup_kernel(multiboot::header *mbh)
{
    // run static ctors
    for (ctorfn* ctor = &start_ctors; ctor < &end_ctors; ctor++)
        (*ctor)();

    multiboot mb(mbh);

    ASSERT(mb.mod_count() == 3);
    // Look at the beginning of passed in modules.

    // For modules that are not compressed, map them into their corresponding space directly.
    multiboot::modinfo *kernel = mb.mod(0); // kernel
    multiboot::modinfo *initcp = mb.mod(1); // initcomp
    multiboot::modinfo *initfs = mb.mod(2); // initfs
    ASSERT(kernel);
    ASSERT(initcp);
    ASSERT(initfs);

    // For modules that are compressed, set up decompression area, unpack and set up mappings.
    // (not used atm).
    // FIXME: if we use entirely compressed image, we would waste memory to
    // first uncompress the elf image and then to copy it to destination address.
    // TODO: compress individual program sections (filesz will be compressed size and memsz - uncompressed, will need some crc to verify section correctness, so each compressed section will have a header { 4 bytes = magic, 4 bytes = crc }
    //

    // Setup small pmm allocator.
    init_memmgr.adjust_alloced_start((address_t)&placement_address);
    init_memmgr.adjust_alloced_start(kernel->mod_end);
    init_memmgr.adjust_alloced_start(initcp->mod_end);
    init_memmgr.adjust_alloced_start(initfs->mod_end);
    // now we can allocate extra pages from alloced_start using pmm_alloc_next_page()

    kconsole << WHITE << "We are loaded at " << (unsigned)&KERNEL_BASE << endl
                      << "Kernel module at " << kernel->mod_start << ", end " << kernel->mod_end << endl
                      << "Initcp module at " << initcp->mod_start << ", end " << initcp->mod_end << endl
                      << "Initfs module at " << initfs->mod_start << ", end " << initfs->mod_end << endl
                      << "Alloctn start at " << init_memmgr.get_alloced_start() << endl;

    init_memmgr.setup_pagetables();

    unsigned int k;

    elf_parser elf;
// TODO: map kernel to highmem
    elf.load_image(kernel->mod_start, kernel->mod_end - kernel->mod_start, &init_memmgr);
    // load initcp last, because of implicit elf state in header_
    // we will use it to jump to initcp entry point below.
    elf.load_image(initcp->mod_start, initcp->mod_end - initcp->mod_start, &init_memmgr);
    // identity map start of initcp so we can access header_ from paging mode.
    init_memmgr.mapping_enter(initcp->mod_start, initcp->mod_start);

    // Identity map initfs (we will load_image components from it later).
    kconsole << endl << "Mapping initfs: ";
    for (k = initfs->mod_start/PAGE_SIZE; k < page_align_up<address_t>(initfs->mod_end)/PAGE_SIZE; k++)
    {
        init_memmgr.mapping_enter(k * PAGE_SIZE, k * PAGE_SIZE);
    }

    // Identity map currently executing code.
    address_t ph_start = (address_t)&KERNEL_BASE;
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
    a = page_align_down<address_t>(mb.mod(0));
    init_memmgr.mapping_enter(a, a);

    kconsole << endl << "Mapped." << endl;

    global_descriptor_table<> gdt;
    kconsole << "Created gdt." << endl;

    interrupts_table.set_isr_handler(14, &page_fault_handler_);
    interrupts_table.install();

    remap_stack();

    init_memmgr.start_paging();

    // jump to initcomp entry point linear address
    typedef void (*initfunc)(multiboot::header *, boot_pmm_allocator *);
    initfunc init = (initfunc)elf.get_entry_point();
    kconsole << endl << "Jumping to " << elf.get_entry_point() << endl << endl;
    init(mbh, &init_memmgr);

    /* Never reached */
    PANIC("init() returned!");
}

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
