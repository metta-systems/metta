#include "stretch_allocator_module_v1_interface.h"
#include "stretch_allocator_module_v1_impl.h"
#include "default_console.h"
#include "macros.h"

static system_stretch_allocator_v1_closure* create(stretch_allocator_module_v1_closure* self, heap_v1_closure* heap, mmu_v1_closure* mmu, memory_v1_virtmem_desc* allvm, memory_v1_virtmem_desc* used)
{
    kconsole << "BOOO!";
    PANIC("end");
}

static void finish_init(stretch_allocator_module_v1_closure* self, system_stretch_allocator_v1_closure* stretch_allocator, vcpu_v1_closure* vcpu, protection_domain_v1_id pdid)
{
    
}

static const stretch_allocator_module_v1_ops ops = {
    create,
    finish_init
};

static const stretch_allocator_module_v1_closure clos = {
    &ops,
    NULL
};

EXPORT_CLOSURE_TO_ROOTDOM(stretch_allocator_module, v1, clos);
