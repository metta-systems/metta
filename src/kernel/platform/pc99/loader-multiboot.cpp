/*!
 * Loader version that supports multiboot specification.
 */
#include "multiboot.h"

/*!
 * Check if a valid multiboot info structure is present.
 */
bool mbi_probe()
{
    multiboot_t* _mbi = multiboot_t::prepare();

    if (_mbi == NULL)
        return false;

    // Make a safe copy of the MBI structure itself.
//     memcopy (&mbi_copy, _mbi, sizeof (mbi_t));
//     mbi = &mbi_copy;

    return true;
}


/**
 * Init function that understands multiboot info structure.
 *
 * The procedure goes as follows:
 * - Find/prepare an MBI structure
 * - ELF-load the first three modules (kernel,sigma0,roottask)
 * - Find the KIP in the kernel
 * - Install memory descriptors from the MBI in the KIP
 * - Install initial servers (sigma0,roottask) in the KIP
 * - Store the bootinfo value in the KIP
 * - Flush caches
 * - Launch the kernel
 *
 * @returns entry point for kernel
 */
address_t mbi_init()
{
//     kip_manager_t kip;
// 
//     void * bi = NULL;
//     bool use_bootinfo = true;
//     bool use_mbi = true;
// 
//     // The KIP is somewhere in the kernel (module 0)
//     if (!kip.find_kip(mbi->mods[0].start, mbi->mods[0].end))
//     {
//         // Bail out if we couldn't find a KIP
//         FAIL();
//     }
// 
//     // Command line strings tend to occupy the same space that we want
//     // to use.  Make a copy of all the strings.
//     char * sptr = strings_copy;
//     L4_Word_t nfree = STRING_BUFFER_SIZE;
//     L4_Word_t len;
// 
/*#define COPY_STRING(str)            \
    do {                    \
    len = strlen (str) + 1;         \
    if (len > nfree)            \
    {                   \
        printf ("String buffer overrun\n"); \
        FAIL ();                \
    }                   \
    strcpy (sptr, str);         \
    str = sptr;             \
    nfree -= len;               \
    sptr += len;                \
    } while (0)*/
// 
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
//     if (mbi->flags.mods)
//     {
//     if (mbi->modcount > MAX_MBI_MODULES)
//     {
//         printf("WARNING: Restricting number of modules to %d (was %d)\n",
//            MAX_MBI_MODULES, mbi->modcount);
//         mbi->modcount = MAX_MBI_MODULES;
//     }
// 
//         // Copy all mods array members into new mods array
//         for (L4_Word_t i = 0; i < mbi->modcount; i++)
//     {
//         COPY_STRING (mbi->mods[i].cmdline);
//         orig_mbi_modules[i] = mbi_modules[i] = mbi->mods[i];
//     }
//         mbi->mods = mbi_modules;
// 
//         /* Install the roottask's command line as the kernel command
//            line in the MBI. By convention, the roottask is the third
//            module. */
//         if (mbi->modcount > 2)
//             mbi->cmdline = mbi->mods[2].cmdline;
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
//     // Update with location of KIP in loader kernel
//     if (!kip.find_kip(mbi->mods[0].start, mbi->mods[0].end))
//         FAIL();
// 
//     // Set up the memory descriptors in the KIP
//     install_memory(mbi, &kip);
// 
//     // Install sigma0's memory region and entry point in the KIP
//     kip.install_sigma0(mbi->mods[1].start, mbi->mods[1].end,
//                        mbi->mods[1].entry, sigma0_type);
//     // Install the root_task's memory region and entry point in the KIP
//     kip.install_root_task(mbi->mods[2].start, mbi->mods[2].end,
//               mbi->mods[2].entry, root_task_type);
// 
//     // Protect all user-level modules.
//     for (L4_Word_t i = 3; i < mbi->modcount; i++)
//     kip.dedicate_memory (mbi->mods[i].start, mbi->mods[i].end - 1,
//                  L4_BootLoaderSpecificMemoryType,
//                  kip_manager_t::desc_boot_module);
// 
// #if defined(L4_32BIT) || defined(ALSO_BOOTINFO32)
//     if (root_task_type == 1)
//     {
//     BI32::L4_BootRec_t * rec = NULL;
// 
//     if (use_bootinfo)
//     {
//         // Allocate a bootinfo structure
//         bi = create_bootinfo (&kip);
// 
//         if (bi)
//         {
//         // Initialize it
//         rec = BI32::init_bootinfo ((BI32::L4_BootInfo_t *) bi);
// 
//         // Record MBI modules
//         rec = BI32::record_bootinfo_modules
//                 ((BI32::L4_BootInfo_t *) bi,
//                  rec, mbi, orig_mbi_modules,
//                  decode_all_executables ? mbi->modcount : 3);
//         }
//     }
// 
//     // Move the MBI into a dedicated memory region
//     if (use_mbi)
//         mbi = install_mbi (&kip);
// 
//     if (bi && use_mbi)
//     {
//         // Make sure that we record MBI location after we have
//         // installed it
//         rec = BI32::record_bootinfo_mbi ((BI32::L4_BootInfo_t *) bi,
//                          rec, mbi);
//     }
//     }
// #endif
// 
// #if defined(L4_64BIT) || defined(ALSO_BOOTINFO64)
//     if (root_task_type == 2)
//     {
//     BI64::L4_BootRec_t * rec = NULL;
// 
//     if (use_bootinfo)
//     {
//         // Allocate a bootinfo structure
//         bi = create_bootinfo (&kip);
// 
//         if (bi)
//         {
//         // Initialize it
//         rec = BI64::init_bootinfo ((BI64::L4_BootInfo_t *) bi);
// 
//         // Record MBI modules
//         rec = BI64::record_bootinfo_modules
//                 ((BI64::L4_BootInfo_t *) bi,
//                  rec, mbi, orig_mbi_modules,
//                  decode_all_executables ? mbi->modcount : 3);
//         }
//     }
// 
//     // Move the MBI into a dedicated memory region
//     if (use_mbi)
//         mbi = install_mbi (&kip);
// 
//     if (bi && use_mbi)
//     {
//         // Make sure that we record MBI location after we have
//         // installed it
//         rec = BI64::record_bootinfo_mbi ((BI64::L4_BootInfo_t *) bi,
//                          rec, mbi);
//     }
//     }
// #endif
// 
//     // Install the bootinfo or MBI into the KIP
//     kip.update_kip (bi ? (L4_Word_t) bi :
//             use_mbi ? (L4_Word_t) mbi : 0);
    multiboot_t* _mbi = multiboot_t::prepare();

    return _mbi->module(0)->reserved; //todo: elf_load the module to figure out the entry address?
}
