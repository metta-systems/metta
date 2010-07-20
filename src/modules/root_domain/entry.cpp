#include "default_console.h"
#include "frames_module_interface.h"
#include "macros.h"
#include "c++ctors.h"

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


// setup gdt and page tables
static void init_mem()
{
    // create physical frames allocator
    frames_module_closure* frames_mod;
    frames_mod = 0;
    // initialize physical memory
///    x86_frame_allocator_t::instance().initialise_before_paging(mb.memory_map(), x86_frame_allocator_t::instance().reserved_range());
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

    init_mem(); // TODO: init mmu

    // Load the modules.
    // Module "boot" depends on all modules that must be probed at startup.
    // Dependency resolution will bring up modules in an appropriate order.
//    load_modules(bootimage, "boot");
}
