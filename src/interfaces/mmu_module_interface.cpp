#include "mmu_module_interface.h"
#include "mmu_module_impl.h"

mmu_v1_closure* mmu_module_v1_closure::create(int32 size/*, ramtab& rt, address_t& free*/)
{
    return methods->create(this, size);
}

void mmu_module_v1_closure::finish_init(/*mmu_v1& mmu, frames_allocator_v1& frames, heap_v1& heap, stretch_allocator_v1& sysalloc*/)
{
    methods->finish_init();
}
