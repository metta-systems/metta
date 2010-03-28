/*!
 * Loader version that supports multiboot specification.
 */
#include "multiboot.h"
#include "bootinfo.h"
#include "new.h"

/*!
 * Check if a valid multiboot info structure is present.
 */
bool mbi_probe()
{
    multiboot_t* _mbi = multiboot_t::prepare();

    if (_mbi == NULL)
        return false;

    // Make a safe copy of the MBI structure itself.
    bootinfo_t* bi = new(BOOTINFO_PAGE) bootinfo_t(false);
    bi->append(_mbi);

    return true;
}


/**
 * Init function that understands multiboot info structure.
 *
 * The procedure goes as follows:
 * - We have mbi inside the bootinfo page already.
 * - ELF-load the kernel module.
 * TODO: make bootinfo page independent of mbi structures (and then install memory map and modules list plus cmdline).
 * - Flush caches.
 * - Launch the kernel.
 *
 * @returns entry point for kernel
 */
address_t mbi_init()
{
    multiboot_t* mbi = multiboot_t::prepare();

//     if (mbi->flags.cmdline)
//     {
//     char * p;
//     COPY_STRING (mbi->cmdline);
// 
/*#define PARSENUM(name, var, msg, massage...)            \
        if ((p = strstr(mbi->cmdline, name"=")) != NULL)    \
        {                           \
            var = strtoul(p+strlen(name)+1, &p, 10);        \
            if (*p == 'K') var*=1024;               \
            if (*p == 'M') var*=1024*1024;          \
            if (*p == 'G') var*=1024*1024*1024;         \
            massage;                        \
            printf(msg,                     \
                   var >= 1<<30 ? var>>30 :         \
                   var >= 1<<20 ? var>>20 :         \
                   var >= 1<<10 ? var>>10 : var,        \
                   var >= 1<<30 ? "G" :             \
                   var >= 1<<20 ? "M" :             \
                   var >= 1<<10 ? "K" : "");            \
        }*/
// 
/*#define PARSEBOOL(name, var, msg)               \
    if ((p = strstr (mbi->cmdline, name"=")) != NULL)   \
    {                           \
        p = strchr (p, '=') + 1;                \
        if (strncmp (p, "yes", 3) == 0 ||           \
        strncmp (p, "on", 2) == 0 ||            \
        strncmp (p, "enable", 6) == 0)          \
        {                           \
        if (! var) printf ("Enabling %s\n", msg);   \
        var = true;                 \
        }                           \
        else if (strncmp (p, "no", 2) == 0 ||       \
             strncmp (p, "off", 3) == 0 ||      \
             strncmp (p, "disable", 7) == 0)        \
        {                           \
        if (var) printf ("Disabling %s\n", msg);    \
        var = false;                    \
        }                           \
    }*/
// 
//         PARSENUM("maxmem",
//                  max_phys_mem,
//                  "Limiting physical memory to %d%sB\n");
//         PARSENUM("kmem",
//                  additional_kmem_size,
//                  "Reserving %d%sB for kernel memory\n",
//                  additional_kmem_size &= ~(kip.get_min_pagesize()-1));
// 
//     PARSEBOOL ("bootinfo", use_bootinfo, "generic bootinfo");
//     PARSEBOOL ("mbi", use_mbi, "multiboot info");
//     PARSEBOOL ("decode-all", decode_all_executables,
//            "decoding of all executables");
//     }
// 
//     // Load the first three modules as ELF images into memory
//     if (!load_modules())
//     {
//         // Bail out if loading failed
//         printf("Failed to load all necessary modules\n");
//         FAIL();
//     }
// 
//     // Protect all user-level modules.
//     for (L4_Word_t i = 3; i < mbi->modcount; i++)
//     kip.dedicate_memory (mbi->mods[i].start, mbi->mods[i].end - 1,
//                  L4_BootLoaderSpecificMemoryType,
//                  kip_manager_t::desc_boot_module);
// 

    return _mbi->module(0)->reserved; //todo: elf_load the module to figure out the entry address?
}
