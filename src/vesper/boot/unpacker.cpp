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
#include "global_descriptor_table.h"
#include "default_console.h"

using namespace metta::kernel;

extern "C" {
    void write_page_directory(address_t page_dir_physical);
    void enable_paging(void);
    void unpack_modules(multiboot::header *mbh);
}

// Declare the page directory and a page table, both 4kb-aligned
address_t kernelpagedir[1024] __attribute__ ((aligned (4096)));
address_t lowpagetable[1024] __attribute__ ((aligned (4096)));

// Set up paging as follows:
// We don't need initial mappings for kernel, as it will be placed by loader.

void init_paging()
{
    // Pointers to the page directory and the page table
    void *kernelpagedirPtr = 0;
    void *lowpagetablePtr = 0;
    int k = 0;

    kernelpagedirPtr = (char *)kernelpagedir;// + 0x40000000;  // Translate the page directory from
    // virtual address to physical address
    lowpagetablePtr = (char *)lowpagetable;// + 0x40000000;    // Same for the page table

    for (k = 0; k < 1024; k++)
    {
        lowpagetable[k] = (k * 4096) | 0x3; // ...map the first 4MB of memory into the page table...
        kernelpagedir[k] = 0;           // ...and clear the page directory entries
    }

    // Fills the addresses 0...4MB and 3072MB...3076MB of the page directory
    // with the same page table

    kernelpagedir[0] = (unsigned long)lowpagetablePtr | 0x3;
    kernelpagedir[768] = (unsigned long)lowpagetablePtr | 0x3;

    write_page_directory((address_t)kernelpagedirPtr);
    enable_paging();
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


    // set up paging for higher-half kernel
// identity map the first meg
// map pages at KERNEL_VIRTUAL_BASE to frames at KERNEL_BASE
// (e.g. 0xC000_0000 to 0x0010_0000)
    init_paging();
    // install gdt
    global_descriptor_table<> gdt;

    kconsole << RED << "inited!";

    // find passed in modules - kernel and initrd

// copy initrd to 0x1000
// jump there
}

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
