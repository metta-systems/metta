//
// Copyright 2007 - 2009, Stanislav Karchebnyy <berkus+metta@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
//
// Prepare kernel and init component for starting up.
//
// * set up paging
// * unpack kernel.assembly in-place and copy appropriate kernel to mapped
// KERNEL_BASE.
// * unpack initrd to 0x1000
// * jump to initrd entrypoint
//
// Simplified version for now: set up paging, copy kernel to 0xC000_0000,
// copy initrd to 0x1000, jump to 0x1000.
//
#include "memutils.h"
#include "multiboot.h"
#include "minmax.h"
#include "global_descriptor_table.h"
#include "default_console.h"

// Excerpted from "memory_manager-arch.h"
#define PAGE_SIZE 0x1000
#define PAGE_MASK 0xFFFFF000

template <typename T>
inline T page_align_up(T a)
{
    if (a % PAGE_SIZE)
    {
        a &= PAGE_MASK;
        a += PAGE_SIZE;
    }
    return a;
}

extern "C" {
    void write_page_directory(address_t page_dir_physical);
    void enable_paging(void);
    void setup_kernel(multiboot::header *mbh);

    address_t placement_address;
    address_t KERNEL_BASE;
}

static address_t alloced_start;
static address_t *kernelpagedir;
static address_t *lowpagetable;
static address_t *highpagetable;

address_t alloc_next_page()
{
    address_t ret = alloced_start;
    alloced_start += PAGE_SIZE;
    return ret;
}

// This part starts in protected mode, linear == physical, paging is off.
void setup_kernel(multiboot::header *mbh)
{
    multiboot mb(mbh);

    ASSERT(mb.mod_count() == 2);
    // Look at the beginning of passed in modules.

    // For modules that are not compressed, map them into their corresponding space directly.
    multiboot::modinfo *kernel = mb.mod(0); // kernel
    multiboot::modinfo *initcp = mb.mod(1); // initcomp
    ASSERT(kernel);
    ASSERT(initcp);

    // For modules that are compressed, set up decompression area, unpack and set up mappings.
    // (not used atm).

    // Setup small pmm allocator.
    alloced_start = (address_t)&placement_address;
    alloced_start = max(alloced_start, kernel->mod_end);
    alloced_start = max(alloced_start, initcp->mod_end);
    alloced_start = page_align_up<address_t>(alloced_start);
    // now we can allocate extra pages from alloced_start using alloc_next_page()

    kconsole << WHITE << "We are loaded at " << (unsigned)&KERNEL_BASE << endl
                      << "Kernel module at " << kernel->mod_start << ", end " << kernel->mod_end << endl
                      << "Initcp module at " << initcp->mod_start << ", end " << initcp->mod_end << endl
                      << "Alloctn start at " << (unsigned)alloced_start << endl;

    // Create and configure paging directory.
    kernelpagedir = (address_t*)alloc_next_page();
    lowpagetable  = (address_t*)alloc_next_page();
    highpagetable = (address_t*)alloc_next_page();

    // We take two pmm pages for occupied memory bitmap and initialize that from the
    // meminfo in mb and memory we took for components.
    // This bitmap will be passed into initcp.
//     bitmap = alloc_next_page(), alloc_next_page();
//     bitmap->set(kernelpagedir);
//     bitmap->set(lowpagetable);
//     bitmap->set(highpagetable);

    unsigned int k;

    // Map initcp to RAM start.
    lowpagetable[0] = 0x0; // invalid page 0
    kconsole << GREEN << "Mapping initcp: ";
    for (k = 1; k <= 1+(initcp->mod_end - initcp->mod_start)/PAGE_SIZE; k++)
    {
        lowpagetable[k] = (initcp->mod_start + ((k-1) * PAGE_SIZE)) | 0x3;
        kconsole << k*PAGE_SIZE << " to " << (unsigned)(lowpagetable[k]&(~0x3)) << ", ";
    }

    // Map kernel to KERNEL_BASE aka 0xC0000000
    kconsole << endl << "Mapping kernel: ";
    for (k = 1; k <= 1+(kernel->mod_end - kernel->mod_start)/PAGE_SIZE; k++)
    {
        highpagetable[k] = (kernel->mod_start + ((k-1) * PAGE_SIZE)) | 0x3;
        kconsole << 0xC0000000+k*PAGE_SIZE << " to " << (unsigned)(highpagetable[k]&(~0x3)) << ", ";
    }

    // Identity map currently executing code.
    address_t start = (address_t)&KERNEL_BASE;
    address_t end   = page_align_up<address_t>((address_t)&placement_address); // one after end
    kconsole << endl << "Mapping loader: ";
    for (k = start/PAGE_SIZE; k < end/PAGE_SIZE; k++)
    {
        lowpagetable[k] = (k * PAGE_SIZE) | 0x3;
        kconsole << k*PAGE_SIZE << " to " << (unsigned)(lowpagetable[k]&(~0x3)) << ", ";
    }

    // Identity map video memory/bios area.
    start = (address_t)0xa0000;
    end   = 0x100000; // one after end
    kconsole << endl << "Mapping VRAM: ";
    for (k = start/PAGE_SIZE; k < end/PAGE_SIZE; k++)
    {
        lowpagetable[k] = (k * PAGE_SIZE) | 0x3;
        kconsole << k*PAGE_SIZE << " to " << (unsigned)(lowpagetable[k]&(~0x3)) << ", ";
    }

    kconsole << endl;

    // Identity map allocated pages?

    kernelpagedir[0] = (address_t)lowpagetable | 0x3;
    kernelpagedir[768] = (address_t)highpagetable | 0x3;

    global_descriptor_table<> gdt;

    // enable paging
    write_page_directory((address_t)kernelpagedir);
    enable_paging();

    // jump to linear 0x1000
    typedef void (*initfunc)(multiboot::header *mbh);
//     initfunc init = (initfunc)0x1000;
//     init(mbh);

    /* Never reached */
    PANIC("init() returned!");
}

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
