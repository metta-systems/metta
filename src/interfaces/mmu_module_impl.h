#pragma once

struct mmu_v1_closure;

// ops structure should be exposed to module implementors!
struct mmu_module_v1_ops
{
    mmu_v1_closure* (*create)(mmu_module_v1_closure* self, int size);
    void (*finish_init)(mmu_module_v1_closure* self);
};
