#include "mmu_module_interface.h"
#include "mmu_module_impl.h"

mmu_v1_closure* mmu_module_v1_closure::create(int size)
{
    return methods->create(this, size);
}

void mmu_module_v1_closure::finish_init()
{
    methods->finish_init(this);
}
