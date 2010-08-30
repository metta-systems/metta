#include "default_console.h"
#include "bootinfo.h"
#include "mmu_module_interface.h"
#include "mmu_module_impl.h"
#include "algorithm"

class mmu_v1_closure;

static mmu_v1_closure* mmu_mod_create(mmu_module_v1_closure* self, int initial_reservation/*, ramtab& ramtab, address_t& free*/)
{
    UNUSED(self);
    UNUSED(initial_reservation);

    // read the memory map from bootinfo page
    bootinfo_t* bi = new(BOOTINFO_PAGE) bootinfo_t;

    struct print_mmap
    {
        void operator ()(multiboot_t::mmap_entry_t e) const
        {
            kconsole << "mmap entry @ " << e.address() << " is " << e.size() << " bytes of type " << e.type() << endl;
        }
    };
    std::for_each(bi->mmap_begin(), bi->mmap_end(), print_mmap());

    return 0;
}

mmu_module_v1_ops ops = {
    mmu_mod_create,
    0
};
