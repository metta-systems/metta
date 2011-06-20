#include "cpu.h"

static void write_pdbr_impl(address_t phys, address_t)
{
    ia32_mmu_t::set_active_pagetable(phys);
}
