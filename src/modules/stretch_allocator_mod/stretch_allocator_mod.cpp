#include "stretch_allocator_module_v1_interface.h"
#include "stretch_allocator_module_v1_impl.h"
#include "system_stretch_allocator_v1_interface.h"
#include "stretch_allocator_v1_interface.h"
#include "stretch_v1_interface.h"
#include "memory_v1_interface.h"
#include "default_console.h"
#include "bootinfo.h"
#include "macros.h"
#include "domain.h"

//======================================================================================================================
// state structures
//======================================================================================================================

struct virtual_address_space_region
{
    virtual_address_space_region* next;
    virtual_address_space_region* prev;
    memory_v1_virtmem_desc desc;
};

/* Shared state */
struct server_state_t
{
    virtual_address_space_region* regions;

    //Frames_cl 	  *f;                  /* Only in nailed sallocs */
    heap_v1_closure* heap;
    mmu_v1_closure*  mmu;

    uint32_t*            sids;               /* Pointer to table of SIDs in use */
    stretch_v1_closure** ss_tab;             /* SID -> Stretch_clp mapping */
    dl_link_t            clients;            /* list of all client states */
};

// Client state.
struct system_stretch_allocator_v1_state
{
    dl_link_t                link;   /* Link into the list of stretch allocators */
    server_state_t*          sa_st;  /* Server (shared) state                    */
    protection_domain_v1_id  pdid;   /* Protection domain of client              */
    protection_domain_v1_id  parent; /* Protection domain of client's parent     */
    vcpu_v1_closure*         vcpu;   /* VCPU of client                           */

    stretch_allocator_v1_closure closure; /* Storage for the returned closure    */

    dl_link_t  stretches; /* We hang all of the allocated stretches here */
}; 

//======================================================================================================================
// stretch_allocator_module_v1 methods
//======================================================================================================================

static system_stretch_allocator_v1_closure* create(stretch_allocator_module_v1_closure* self, heap_v1_closure* heap, mmu_v1_closure* mmu)
{
    kconsole << "BOOO!";
    bootinfo_t* bi = new(BOOTINFO_PAGE) bootinfo_t;
    //TODO: use bootinfo page instead of allvm and used...
    
    // allvm is just an entire virtual address space (0 to ADDRESS_T_MAX, except some unusable areas, if any)
    
    PANIC("end");
}

static void finish_init(stretch_allocator_module_v1_closure* self, system_stretch_allocator_v1_closure* stretch_allocator, vcpu_v1_closure* vcpu, protection_domain_v1_id pdid)
{
    stretch_allocator->state->vcpu = vcpu;
    stretch_allocator->state->pdid = pdid;
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
