#pragma once

#include "module_interface.h"

struct mmu_v1_closure;

DECLARE_CLOSURE(mmu_module_v1)
{
    mmu_v1_closure* (*create)(int32_t size/*, ramtab& rt, address_t& free*/);
    void (*finish_init)(/*mmu_v1& mmu, frames_allocator_v1& frames, heap_v1& heap, stretch_allocator_v1& sysalloc*/);
};
