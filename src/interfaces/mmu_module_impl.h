#pragma once

struct mmu_v1_closure;

// ops structure should be exposed to module implementors!
struct mmu_module_v1_ops
{
    mmu_v1_closure* (*create)(mmu_module_v1_closure* self, int32_t size/*, ramtab& rt, address_t& free*/);
    void (*finish_init)(mmu_module_v1_closure* self/*, mmu_v1& mmu, frames_allocator_v1& frames, heap_v1& heap, stretch_allocator_v1& sysalloc*/);
};
