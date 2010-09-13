#pragma once

#include "module_interface.h"

struct mmu_v1_closure;

struct mmu_module_v1_ops; struct mmu_module_v1_state; struct mmu_module_v1_closure
// DECLARE_CLOSURE(mmu_module_v1)
{
    const mmu_module_v1_ops* methods;
    mmu_module_v1_state* state;

    mmu_v1_closure* create(int size);
    void finish_init();
};
