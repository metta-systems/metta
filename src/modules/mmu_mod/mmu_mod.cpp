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

    kconsole << "MMU mod : create" << endl;

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

//     frames_mod->initialise_before_paging(mb.memory_map());//, x86_frame_allocator_t::instance().reserved_range()

    return 0;
}

static const mmu_module_v1_ops ops = {
    mmu_mod_create,
    NULL
};

static const mmu_module_v1_closure clos = {
    &ops,
    NULL
};

#define EXPORT_CL_TO_ROOTDOM(_type, _name, _cl) \
    extern "C" const _type##_closure* const exported_##_name##_rootdom = &_cl

EXPORT_CL_TO_ROOTDOM(mmu_module_v1, mmu_module, clos);
