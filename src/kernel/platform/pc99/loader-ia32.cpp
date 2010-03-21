/*!
 * Arch-specific part of the loader.
 */

#include "loader.h"

/*
* Functions needed for loader format structure.
*/
bool mbi_probe();
address_t mbi_init();

/*
 * Loader formats supported for IA32.
 */
loader_format_t loader_formats[] = {
    { "multiboot compliant loader", mbi_probe, mbi_init },
    NULL_LOADER
};


void flush_cache()
{
    __asm__ __volatile__ ("wbinvd");
}

/*
 * Start kernel at its entry point. No preconditions
 */
void launch_kernel(address_t entry)
{
    __asm__ __volatile__ ("jmp *%0" : : "r"(entry));
}
