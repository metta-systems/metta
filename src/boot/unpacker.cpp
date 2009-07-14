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
#include "minmax.h"
#include "memory.h"
#include "global_descriptor_table.h"
#include "default_console.h"
#include "elf_parser.h"

extern "C" {
    void write_page_directory(address_t page_dir_physical);
    void enable_paging(void);
    void setup_kernel(multiboot::header *mbh);

    address_t placement_address;
    address_t KERNEL_BASE;
}

//! Start and end of allocated pages for passing into initcp.
static address_t alloced_start;
// static address_t alloced_end;

static address_t *kernelpagedir;
static address_t *lowpagetable;
static address_t *highpagetable;

void mapping_enter(address_t vaddr, address_t paddr)
{
    address_t *pagetable = 0;
    if (vaddr < 256*1024*1024)
        pagetable = lowpagetable;
    else if (vaddr > 768*1024*1024)
        pagetable = highpagetable;
    else
        PANIC("invalid vaddr in mapping_enter");

    kconsole << "Entering mapping " << vaddr << " => " << paddr << endl;

    pagetable[vaddr / PAGE_SIZE] = paddr | 0x3;
}

// TODO: pmm interface + pmm_state transfer

address_t pmm_alloc_next_page()
{
    address_t ret = alloced_start;
    alloced_start += PAGE_SIZE;
    return ret;
}

address_t pmm_alloc_page(address_t vaddr)
{
    address_t ret = pmm_alloc_next_page();
    mapping_enter(vaddr, ret);
    return ret;
}

// This part starts in protected mode, linear == physical, paging is off.
void setup_kernel(multiboot::header *mbh)
{
    multiboot mb(mbh);

    ASSERT(mb.mod_count() == 3);
    // Look at the beginning of passed in modules.

    // For modules that are not compressed, map them into their corresponding space directly.
    multiboot::modinfo *kernel = mb.mod(0); // kernel
    multiboot::modinfo *initcp = mb.mod(1); // initcomp
    ASSERT(kernel);
    ASSERT(initcp);

    // For modules that are compressed, set up decompression area, unpack and set up mappings.
    // (not used atm).
    // FIXME: if we use entirely compressed image, we would waste memory to
    // first uncompress the elf image and then to copy it to destination address.
    // TODO: compress individual program sections (filesz will be compressed size and memsz - uncompressed, will need some crc to verify section correctness, so each compressed section will have a header { 4 bytes = magic, 4 bytes = crc }
    //

    // Setup small pmm allocator.
    alloced_start = (address_t)&placement_address;
    alloced_start = max(alloced_start, kernel->mod_end);
    alloced_start = max(alloced_start, initcp->mod_end);
    alloced_start = page_align_up<address_t>(alloced_start);
    // now we can allocate extra pages from alloced_start using pmm_alloc_next_page()

    kconsole << WHITE << "We are loaded at " << (unsigned)&KERNEL_BASE << endl
                      << "Kernel module at " << kernel->mod_start << ", end " << kernel->mod_end << endl
                      << "Initcp module at " << initcp->mod_start << ", end " << initcp->mod_end << endl
                      << "Alloctn start at " << (unsigned)alloced_start << endl;

    // Create and configure paging directory.
    kernelpagedir = reinterpret_cast<address_t*>(pmm_alloc_next_page());
    lowpagetable  = reinterpret_cast<address_t*>(pmm_alloc_next_page());
    highpagetable = reinterpret_cast<address_t*>(pmm_alloc_next_page());

    lowpagetable[0] = 0x0; // invalid page 0

    // We take two pmm pages for occupied memory bitmap and initialize that from the
    // meminfo in mb and memory we took for components.
    // This bitmap will be passed into initcp.
//     bitmap = pmm_alloc_next_page(), pmm_alloc_next_page();
//     bitmap->set(kernelpagedir);
//     bitmap->set(lowpagetable);
//     bitmap->set(highpagetable);

    unsigned int k;

    // NB!
    // mod_start and mod_end don't account for BSS size.
    // account for it when we have ELF parser in sprint 3.
    // for now just map two extra pages for BSS.

    elf_parser elf;
// TODO: map kernel to highmem
    elf.load_image(kernel->mod_start, kernel->mod_end - kernel->mod_start);
    // load initcp last, because of implicit elf state in header_
    // we will use it to jump to initcp entry point below.
    elf.load_image(initcp->mod_start, initcp->mod_end - initcp->mod_start);
    // identity map start of initcp so we can access header_ from paging mode.
    mapping_enter(initcp->mod_start, initcp->mod_start);

    // Identity map currently executing code.
    address_t ph_start = (address_t)&KERNEL_BASE;
    address_t ph_end   = page_align_up<address_t>((address_t)&placement_address); // one after end
    kconsole << endl << "Mapping loader: ";
    for (k = ph_start/PAGE_SIZE; k < ph_end/PAGE_SIZE; k++)
    {
        mapping_enter(k * PAGE_SIZE, k * PAGE_SIZE);
    }

    // Identity map video memory/bios area.
    ph_start = (address_t)0xa0000;
    ph_end   = 0x100000; // one after end
    kconsole << endl << "Mapping VRAM: ";
    for (k = ph_start/PAGE_SIZE; k < ph_end/PAGE_SIZE; k++)
    {
        mapping_enter(k * PAGE_SIZE, k * PAGE_SIZE);
    }

    kconsole << endl << "Mapping multiboot info: ";
    address_t a = page_align_down<address_t>(mbh);
    mapping_enter(a, a);

    kconsole << endl << "Mapped." << endl;

    kernelpagedir[0] = (address_t)lowpagetable | 0x3;
    kernelpagedir[768] = (address_t)highpagetable | 0x3;

    global_descriptor_table<> gdt;
    kconsole << "Created gdt." << endl;

    // enable paging
    write_page_directory((address_t)kernelpagedir);
    kconsole << "Set CR3." << endl;
    enable_paging();
    kconsole << "Enabled paging." << endl;

    // jump to initcomp entry point linear address
    typedef void (*initfunc)(multiboot::header *mbh);
    initfunc init = (initfunc)elf.get_entry_point();
    kconsole << endl << "Jumping to " << elf.get_entry_point();
    init(mbh);

    /* Never reached */
    PANIC("init() returned!");
}

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
