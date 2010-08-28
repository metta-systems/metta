#include "default_console.h"
#include "frames_module_interface.h"
#include "macros.h"
#include "c++ctors.h"
#include "root_domain.h"
#include "bootinfo.h"
#include "new.h"
#include "algorithm"

// bootimage contains modules and namespaces
// each module has an associated namespace which defines some module attributes/parameters.
// startup module from which root_domain starts also has a namespace called "default_namespace"
// it defines general system attributes and startup configuration.

/*
 * Root domain starts executing without paging and with full ring0 rights.
 */

/*#include "memutils.h"
#include "memory.h"
#include "multiboot.h"
#include "elf_parser.h"
#include "registers.h"
#include "c++ctors.h"
#include "linksyms.h"
#include "frame.h"
#include "page_directory.h"
#include "x86_frame_allocator.h"
#include "x86_protection_domain.h"
#include "new.h"*/
// #include "stretch_driver.h"

#if 0
static void map_identity(const char* caption, address_t start, address_t end)
{
#if MEMORY_DEBUG
    kconsole << "Mapping " << caption << endl;
#endif
    end = page_align_up<address_t>(end); // one past end
    for (uint32_t k = start/PAGE_SIZE; k < end/PAGE_SIZE; k++)
        protection_domain_t::privileged().map(k * PAGE_SIZE, reinterpret_cast<void*>(k * PAGE_SIZE), page_t::kernel_mode | page_t::writable);
}
#endif

// x86_frame_allocator_t would wrap around frames_mod instance (??)
// inline 

// build dependency graph for "name" module and ensure all dependencies are loaded.
void* load_module(const char* name, module_namespace_t* namesp)
{
//    namesp->lookup(name);
    UNUSED(name);
    UNUSED(namesp);
/*    addr = bootimg.find_module(size, name);
    elf_parser mod(addr, size);
    elf32::section_header_t* modinfo = mod.section(".modinfo");
    // load all dependencies
    deplist_t* deps = module_info_t(modinfo).parse_deps();
    foreach (dep, deps)
    {
        load_module(dep->module_name); // ugh, don't recurse!
    }*/
    // return module "name"
    return 0;//addr; //? perhaps return modinfo location instead? sort of module control block
}

template <class closure_type>
closure_type* load_module(const char* name, module_namespace_t* namesp)
{
    return (closure_type*)load_module(name, namesp);
}

// setup gdt and page tables
static void init_mem(bootimage_t& /*bootimage*/)
{
    kconsole << "init_mem" << endl;
    // read the memory map from bootinfo page
    bootinfo_t* bi = new(BOOTINFO_PAGE) bootinfo_t;

    struct print_mmap
    {
        void operator ()(multiboot_t::mmap_entry_t e)
        {
            kconsole << "mmap entry @ " << e.address() << " is " << e.size() << " bytes of type " << e.type() << endl;
        }
    };
    std::for_each(bi->mmap_begin(), bi->mmap_end(), print_mmap());

/**    root_domain_t root_dom(bootimage);
    module_namespace_t namesp = root_dom.get_namespace();

    // create physical frames allocator
    frames_module_closure* frames_mod;
    // find mod from given namesp and load it
    frames_mod = load_module<frames_module_closure>("frames_mod", &namesp);
//     mmu_module_closure* mmu_mod;
//     mmu_mod = load_module("mmu_mod", namesp);

//     mmu_mod->create(...);

//     frames_mod->create(mmu_mod...);
*/
    // initialize physical memory
    //FIXME: this is inside frames_mod or mmu_mod even!
//     frames_mod->initialise_before_paging(mb.memory_map());//, x86_frame_allocator_t::instance().reserved_range()
    // create virtual memory allocator
    // initialize virtual memory map
    // create stretch allocator
    // assign stretches to address ranges
    // install page fault handler
    // enable paging
//     static_cast<x86_protection_domain_t&>(protection_domain_t::privileged()).enable_paging();
//     kconsole << "Enabled paging." << endl;

#if 0
    // Identity map currently executing code.
    // page 0 is not mapped to catch null pointers
    map_identity("bottom 4Mb", PAGE_SIZE, 4*MiB - PAGE_SIZE);
//     stretch_driver_t::default_driver().initialise();
//    x86_frame_allocator_t::set_allocation_start(page_align_up<address_t>(std::max(LINKSYM(placement_address), bootimage->mod_end)));
    // now we can allocate memory frames
#endif
}

// depgraph: boot -> frame_allocator
// frame_allocator -> frames_module
// client_frame_allocator -> frame_allocator

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

    // Load components from bootcp.
//     kconsole << "opening initfs @ " << bootimage->mod_start << endl;
//     initfs_t initfs(bootcp->mod_start);
//     typedef void (*comp_entry)(bootinfo_t bi_page);

    // For now, boot components are all linked at different virtual addresses so they don't overlap.
//     kconsole << "iterating components" << endl;
//     for (uint32_t k = 0; k < initfs.count(); k++)
//     {
//         kconsole << YELLOW << "boot component " << initfs.get_file_name(k) << " @ " << initfs.get_file(k) << endl;
//         // here loading an image should create a separate PD with its own pagedir
// //         domain_t* d = nucleus->create_domain();
//
//         if (!elf.load_image(initfs.get_file(k), initfs.get_file_size(k)))
//             kconsole << RED << "not an ELF file, load failed" << endl;
//         else
//             kconsole << GREEN << "ELF file loaded, entering component init" << endl;
//         comp_entry init_component = (comp_entry)elf.get_entry_point();
//         init_component(bootinfo);
//     }
//}

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

    init_mem(bootimage); // TODO: init mmu

    // Load the modules.
    // Module "boot" depends on all modules that must be probed at startup.
    // Dependency resolution will bring up modules in an appropriate order.
//    load_modules(bootimage, "boot");
}
