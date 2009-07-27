//
// Copyright 2007 - 2009, Stanislav Karchebnyy <berkus+metta@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "multiboot.h"
#include "default_console.h"
#include "memory.h"
#include "initfs.h"

extern "C" void entry(multiboot::header *mbh, boot_pmm_allocator *init_memmgr);

/*!
@brief Boot up the system - kernel and libOS.
@ingroup Booting

Stack mapping is set up for us in unpacker, but we might need a larger one.

* initcp
 - initcp expects cpu to be in paging mode with pagedir in highmem, kernel
   mapped high and exception handlers set up in low mem - these will be relocated
   and reinstated by interrupts component when it's loaded,
 - verify required components are present in initfs (pmm, cpu, interrupts,
   security manager, portal manager, object loader),
 - instantiate/initialize components taking dependencies into account,
 - enter preexisting pmm mappings into pmm component
 - set up security contexts and permissions
 - mount root filesystem
 - boot other cpus if present,
 - switch to usermode and continue execution in the init process,
 - initcp is the first nester, that provides base Common Protocols components
   and boots up other components such as:
   - scheduler
   - security server
   - object (security) manager

* component constructors
 - run in kernel mode, have the ability to set up their system tables etcetc,

Metta components to instantiate in initcomp:
- interrupt dispatcher,
- root memory manager,
- portal manager,
- root filesystem mounter,
- hardware detector,
- root object manager,
- root security server,
- and root trader.

Once trader is started, components can be requested for and connected.
*/
void entry(multiboot::header *mbh, boot_pmm_allocator *init_memmgr)
{
    kconsole << WHITE << "...in the living memory of V2_OS" << endl;

    multiboot mb(mbh);
    kconsole << GREEN << "mb.lower_mem = " << mb.lower_mem() << endl
                      << "mb.upper_mem = " << mb.upper_mem() << endl;

    multiboot::modinfo *initfsmod = mb.mod(2); // initfs
    ASSERT(initfsmod);
    kconsole << "need " << (address_t)initfsmod << " mapped" << endl;
    ASSERT(init_memmgr->mapping_entered((address_t)initfsmod));

    initfs fs(initfsmod->mod_start);
    uint32_t i = 0;

    while (i < fs.count())
    {
        kconsole << YELLOW << "initfs file " << fs.get_file_name(i) << " @ " << fs.get_file(i) << endl;
//         elf_image img(
        i += 1;
    }

    while(1) {}
}

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
