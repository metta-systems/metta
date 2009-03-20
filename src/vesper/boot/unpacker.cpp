// * set up paging
// * unpack kernel.assembly in-place and copy appropriate kernel to mapped
// KERNEL_BASE.
// * unpack initrd to 0x1000
// * jump to initrd entrypoint
//
// Simplified version now: set up paging, copy kernel to 0xC000_0000, copy initrd to 0x1000,
// jump to 0x1000.
#include "memutils.h"
#include "multiboot.h"
#include "globals.h"
#include "global_descriptor_table.h"
#include "default_console.h"

//included from "memory_manager-arch.h"
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

// remove the namespace!
using namespace metta::kernel;

extern "C" {
    void write_page_directory(address_t page_dir_physical);
    void enable_paging(void);
    void unpack_modules(multiboot::header *mbh);

    address_t placement_address;
    address_t KERNEL_BASE;
}

static address_t alloced_start;
static address_t *kernelpagedir;
static address_t *lowpagetable;

// Set up paging as follows:
// We don't need initial mappings for kernel, as it will be placed by loader.

/*void init_paging()
{
    int k = 0;

    for (k = 0; k < 1024; k++)
    {
        lowpagetable[k] = (k * PAGE_SIZE) | 0x3; // ...map the first 4MB of memory into the page table...
        kernelpagedir[k] = 0;           // ...and clear the page directory entries
    }

    // Fills the addresses 0...4MB and 3072MB...3076MB of the page directory
    // with the same page table

    kernelpagedir[0] = (unsigned long)lowpagetablePtr | 0x3;
    kernelpagedir[768] = (unsigned long)lowpagetablePtr | 0x3;

    write_page_directory((address_t)kernelpagedirPtr);
    enable_paging();
}*/

address_t alloc_next_page()
{
    address_t ret = placement_address;
    placement_address += PAGE_SIZE;
    return ret;
}

// This part starts in protected mode, linear == physical, paging is off.
void unpack_modules(multiboot::header *mbh)
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
                      << "Alloctn start at " << (unsigned)&alloced_start << endl;

    // Create and configure paging directory.
    kernelpagedir = (address_t*)alloc_next_page();
    lowpagetable  = (address_t*)alloc_next_page();

    // We take two pmm pages for occupied memory bitmap and initialize that from the
    // meminfo in mb and memory we took for components.
    // This bitmap will be passed into initcp.

    // Map initcp to RAM start.
    unsigned int k = 0;

    lowpagetable[0] = 0x3;
    for (k = 1; k <= 1+(initcp->mod_end - initcp->mod_start)/PAGE_SIZE; k++)
    {
        lowpagetable[k] = (initcp->mod_start + ((k-1) * PAGE_SIZE)) | 0x3;
    	kconsole << GREEN << "Mapping " << k*PAGE_SIZE << " to " << (unsigned)(lowpagetable[k]&(~0x3)) << endl;
    }

    kernelpagedir[0] = (address_t)lowpagetable;

    global_descriptor_table<> gdt;

    // enable paging

    // jump to linear 0x1000
}

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
