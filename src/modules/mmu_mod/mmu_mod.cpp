#include "default_console.h"
#include "bootinfo.h"
#include "mmu_module_v1_interface.h"
#include "mmu_module_v1_impl.h"
#include "mmu_v1_interface.h"
#include "algorithm"

// TODO: We need to abstract frames module from the format of bootinfo page,
// so we create a local copy of memory map and pass it to frames_mod as a parameter.
// For simplicity we do it here at the moment.

static mmu_v1_closure* mmu_mod_create(mmu_module_v1_closure* self, int initial_reservation/*, ramtab& ramtab, address_t& free*/)
{
    UNUSED(self);
    UNUSED(initial_reservation);

    kconsole << "MMU mod : create" << endl;

    // read the memory map from bootinfo page
    bootinfo_t* bi = new(BOOTINFO_PAGE) bootinfo_t;

//    std::vector<multiboot_t::mmap_entry_t> physical_mem, initial_mappings;

//[&physical_mem, &initial_mappings]
    std::for_each(bi->mmap_begin(), bi->mmap_end(), [](const multiboot_t::mmap_entry_t e)
    {
        kconsole << "mmap entry @ " << e.address() << " is " << e.size() << " bytes of type " << e.type() << endl;
/*        if (e.type() == Free)
            physical_mem.push_back(e);
        else
            initial_mappings.push_back(e);*/
    });

    // Calculate how much space is needed for the MMU structures.
//    mmu_state,
//    pagetables
//    and ramtab

    // Find proper location to start "allocating" from.
    // Skip memory below 1Mb on x86.
//     if (first_available_range & initial_mapping)
//         first_available_range -= initial_mapping;
//     alloc_addr = page_round_up(initial_mapping.start);

    return 0;
}

static const mmu_module_v1_ops ops = {
    mmu_mod_create
};

static const mmu_module_v1_closure clos = {
    &ops,
    NULL
};

EXPORT_CLOSURE_TO_ROOTDOM(mmu_module_v1, mmu_module, clos);
