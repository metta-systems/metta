/*!
* Minimal paged memory allocator.
*
* Used for startup and paged mode initialization.
*/
#include "memory.h"
#include "default_console.h"
#include "registers.h"
#include "minmax.h"

void boot_pmm_allocator::setup_pagetables()
{
    // Create and configure paging directory.
    kernelpagedir = reinterpret_cast<address_t*>(alloc_next_page());
    lowpagetable  = reinterpret_cast<address_t*>(alloc_next_page());
    highpagetable = reinterpret_cast<address_t*>(alloc_next_page());

    // TODO: map to top of memory
    mapping_enter((address_t)kernelpagedir, (address_t)kernelpagedir);
    mapping_enter((address_t)lowpagetable, (address_t)lowpagetable);
    mapping_enter((address_t)highpagetable, (address_t)highpagetable);

    lowpagetable[0] = 0x0; // invalid page 0
}

void boot_pmm_allocator::start_paging()
{
    kernelpagedir[0] = (address_t)lowpagetable | 0x3;
    kernelpagedir[768] = (address_t)highpagetable | 0x3;

    // enable paging
    write_page_directory((address_t)kernelpagedir);
    kconsole << "Set CR3." << endl;
    enable_paging(); // from registers.s
    kconsole << "Enabled paging." << endl;
}

void boot_pmm_allocator::adjust_alloced_start(address_t new_start)
{
    alloced_start = max(alloced_start, new_start);
    alloced_start = page_align_up<address_t>(alloced_start);
}

address_t boot_pmm_allocator::get_alloced_start()
{
    return alloced_start;
}

address_t *boot_pmm_allocator::select_pagetable(address_t vaddr)
{
    address_t *pagetable = 0;
    if (vaddr < 256*1024*1024)
        pagetable = lowpagetable;
    else if (vaddr > 768*1024*1024)
        pagetable = highpagetable;
    else
        PANIC("invalid vaddr in select_pagetable");
    return pagetable;
}

void boot_pmm_allocator::mapping_enter(address_t vaddr, address_t paddr)
{
    address_t *pagetable = select_pagetable(vaddr);
    kconsole << "Entering mapping " << vaddr << " => " << paddr << endl;
    pagetable[vaddr / PAGE_SIZE] = paddr | 0x3;
}

bool boot_pmm_allocator::mapping_entered(address_t vaddr)
{
    address_t *pagetable = select_pagetable(vaddr);
    return pagetable[vaddr / PAGE_SIZE] != 0;
}

// TODO: pmm interface + pmm_state transfer

address_t boot_pmm_allocator::alloc_next_page()
{
    address_t ret = alloced_start;
    alloced_start += PAGE_SIZE;
    return ret;
}

address_t boot_pmm_allocator::alloc_page(address_t vaddr)
{
    address_t ret = alloc_next_page();
    mapping_enter(vaddr, ret);
    return ret;
}
