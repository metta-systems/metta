#include "default_console.h"
#include "frames_module_interface.h"
#include "mmu_module_interface.h"
#include "macros.h"
#include "c++ctors.h"
#include "root_domain.h"
#include "bootinfo.h"
#include "new.h"
#include "elf_loader.h"

// bootimage contains modules and namespaces
// each module has an associated namespace which defines some module attributes/parameters.
// startup module from which root_domain starts also has a namespace called "default_namespace"
// it defines general system attributes and startup configuration.

#include "mmu_module_impl.h" // for debug

//======================================================================================================================
// Look up in root_domain's namespace and load a module by given name, satisfying its dependencies, if possible.
//======================================================================================================================

/// tagged_t namesp.find(string key)

static void* load_module(bootimage_t& bootimg, const char* module_name, const char* clos)
{
    bootimage_t::modinfo_t addr = bootimg.find_module(module_name);
    if (!addr.start)
        return 0;

    kconsole << "Found module " << module_name << " at address " << addr.start << " of size " << addr.size << endl;

    elf_loader_t loader(addr.start);
    loader.relocate_to(addr.start);

    kconsole << "Module relocated." << endl;

    /* FIXME: Skip dependencies for now */

    return (void*)(loader.start() + loader.find_symbol(clos));
}

template <class closure_type>
static inline closure_type* load_module(bootimage_t& bootimg, const char* module_name, const char* clos)
{
    return (closure_type*)load_module(bootimg, module_name, clos);
}

//======================================================================================================================
// setup page mapping - TODO: move to MMU
//======================================================================================================================

#if 0
static void map_identity(const char* caption, address_t start, address_t end)
{
#if MEMORY_DEBUG
    kconsole << "Mapping " << caption << endl;
#endif
    end = page_align_up<address_t>(end); // one past end
    for (uint32_t k = start/PAGE_SIZE; k < end/PAGE_SIZE; k++)
        protection_domain_t::privileged().map(k * PAGE_SIZE, reinterpret_cast<void*>(k * PAGE_SIZE),
        page_t::kernel_mode | page_t::writable);
}
#endif

//======================================================================================================================
// setup MMU and frame allocator
//======================================================================================================================

static void init_mem(bootimage_t& bootimg)
{
    kconsole << "init_mem" << endl;

    // request necessary space for frames allocator
//     frames_module_v1_closure* frames_mod;
//     // find mod from given namesp and load it
//     frames_mod = load_module<frames_module_v1_closure>("frames_mod", 0);

    int required = 0;//frames_mod->required_size();

    int initial_heap_size = 64*1024;

    mmu_module_v1_closure* mmu_mod;
    mmu_mod = load_module<mmu_module_v1_closure>(bootimg, "mmu_mod", "exported_mmu_module_rootdom");

    kconsole << "Found mmu_mod closure @ " << mmu_mod << endl;

    //ASSERT(mmu_mod);
    if (mmu_mod)
    {
        kconsole << "Closure ops " << mmu_mod->methods << endl
                 << "Closure st  " << mmu_mod->state << endl;

        kconsole << "Create call @ " << (address_t)mmu_mod->methods->create << endl;

        void *mmu = mmu_mod->methods->create(mmu_mod, required + initial_heap_size);
        mmu = mmu_mod->create(required + initial_heap_size);
        UNUSED(mmu);
    }
#if 0
    // create virtual memory allocator
    // initialize virtual memory map
    // create stretch allocator
    // assign stretches to address ranges
//     stretch_driver_t::default_driver().initialise();
    // install page fault handler
    // Identity map currently executing code.
    // page 0 is not mapped to catch null pointers
    map_identity("bottom 4Mb", PAGE_SIZE, 4*MiB - PAGE_SIZE);
    // enable paging
//     static_cast<x86_protection_domain_t&>(protection_domain_t::privileged()).enable_paging();
//     kconsole << "Enabled paging." << endl;
#endif
}

//======================================================================================================================
// load all required modules (mostly drivers)
//======================================================================================================================

//static void load_modules(UNUSED_ARG bootimage_t& bm, UNUSED_ARG const char* root_module)
//{
    // if bootpage contains devtree, we use it in building modules deps
    // find modules corresponding to devtree entries and add them to deps list
    // if no devtree present (on x86) we add "probe devices later" entry to bootpage to force
    // module probing after initial startup.

//     module_loader_t ml;
//     ml.load_modules("boot");
    // each module has .modinfo section with entry point and other meta info
    // plus .modinfo.deps section with module dependencies graph data

    // Load components from bootimage.
//     kconsole << "opening initfs @ " << bootimage->mod_start << endl;
//     initfs_t initfs(bootcp->mod_start);
//     typedef void (*comp_entry)(bootinfo_t bi_page);
//}

//======================================================================================================================
// root_domain entry point
//======================================================================================================================

/*!
* Root domain starts executing without paging and with full ring0 rights.
*/

extern "C" void entry()
{
    run_global_ctors(); // don't forget, we don't have proper crt0 yet.

    kconsole << "root_domain entry!" << endl;

    bootinfo_t* bi = new(BOOTINFO_PAGE) bootinfo_t(false);
    address_t start, end;
    const char* name;
    if (!bi->get_module(1, start, end, name))
    {
        PANIC("Bootimage not found! in root_domain");
    }

    bootimage_t bootimage(name, start, end);

    init_mem(bootimage);

    // Load the modules.
    // Module "boot" depends on all modules that must be probed at startup.
    // Dependency resolution will bring up modules in an appropriate order.
//    load_modules(bootimage, "boot");

    PANIC("root_domain entry returned!");
}
