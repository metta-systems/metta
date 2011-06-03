#include "frames_module_v1_interface.h"
#include "frames_module_v1_impl.h"
#include "types.h"
#include "macros.h"
#include "default_console.h"
#include "bootinfo.h"
#include "algorithm"
#include "system_frame_allocator_v1_impl.h"

struct frames_client_state
{
    frame_allocator_v1_closure closure;
    
    uint32_t n_allocated_phys_frames;

    uint32_t owner;
    size_t guaranteed_frames;
    size_t extra_frames;
    
    heap_v1_closure* heap;
    frames_module_v1_state* module_state;
};

struct frame_st
{
    uint32_t free;
};

struct frames_module_v1_state
{
    address_t start;
    size_t n_logical_frames;
    uint32_t frame_width;
    memory_v1_attrs attrs;
    ramtab_v1_closure* ramtab;
    frames_module_v1_state* next;
    frame_st* frames;
};

//======================================================================================================================
// system_frame_allocator_v1 implementation
//======================================================================================================================

static const system_frame_allocator_v1_ops system_frame_allocator_v1_methods = {
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
};

//======================================================================================================================
// frames_module_v1 implementation
//======================================================================================================================

static memory_v1_size frames_module_v1_required_size(frames_module_v1_closure* self)
{
    UNUSED(self);
    size_t n_regions = 0, n_frames = 0, res = 0;
    bootinfo_t* bi = new(BOOTINFO_PAGE) bootinfo_t; // simplify memory map operations

    // Scan through the set of mem desc and count the number of logical frames they contain in total.
    std::for_each(bi->mmap_begin(), bi->mmap_end(), [&n_regions, &n_frames](const multiboot_t::mmap_entry_t* e)
    {
        ++n_regions;
        n_frames += e->size() >> FRAME_WIDTH;
    });

    res = sizeof(frame_allocator_v1_closure) + sizeof(frames_client_state) + n_regions * sizeof(frames_module_v1_state) + n_frames * sizeof(frame_st);
    res = page_align_up(res);
    
    kconsole << " +-frames_mod: counted " << n_regions << " memory regions" << endl;
/*
    res = sizeof(Frames_cl) + sizeof(FramesClient_st) + 
	(nreg * sizeof(struct _FramesMod_st)) +
	(nlf * sizeof(struct _Frame));
*/
    return res;
}

//Don't need where, bootinfo page has logic for finding an allocatable place. We use it currently because MMU mod has to enter some initial mappings - but this could be streamlined.

//temp dup from mmu_mod
#define OWNER_SYSTEM  0x1     /* pfn is owned by us (mmgmt etc) */

static system_frame_allocator_v1_closure* frames_module_v1_create(frames_module_v1_closure* self, ramtab_v1_closure* rtab, memory_v1_address where_to_start)
{
    UNUSED(self);
    
	kconsole << "frames_mod create @ " << where_to_start << endl;
    frames_client_state* client_state = reinterpret_cast<frames_client_state*>(where_to_start);

    system_frame_allocator_v1_closure* ret = reinterpret_cast<system_frame_allocator_v1_closure*>(&client_state->closure);
    ret->state = reinterpret_cast<system_frame_allocator_v1_state*>(client_state);
    ret->methods = &system_frame_allocator_v1_methods;

    frames_module_v1_state* frames_state = reinterpret_cast<frames_module_v1_state*>(where_to_start + sizeof(frames_client_state));
    
    client_state->owner = OWNER_SYSTEM;
    client_state->n_allocated_phys_frames = 0;
    client_state->guaranteed_frames = -1;
    client_state->extra_frames = -1;
    client_state->heap = 0;
    client_state->module_state = frames_state;
    
    frames_module_v1_state* running_state = frames_state;
    frames_module_v1_state* last_state = running_state;
    size_t n_regions = 0;

    bootinfo_t* bi = new(BOOTINFO_PAGE) bootinfo_t;
    std::for_each(bi->mmap_begin(), bi->mmap_end(), [&running_state, &last_state, &n_regions, rtab](const multiboot_t::mmap_entry_t* e)
    {
        running_state->start = e->address();
        running_state->n_logical_frames = e->size() >> FRAME_WIDTH;
        running_state->frame_width = FRAME_WIDTH;
// temp defines
#define RAM_FREE 1
#define ACPI_RECLAIMABLE 3
        if ((e->type() == RAM_FREE) || (e->type() == ACPI_RECLAIMABLE))
        {
            running_state->attrs = memory_v1_attrs_regular;
            running_state->ramtab = rtab;
        }
        else
        {
            running_state->attrs = memory_v1_attrs_non_memory;
            running_state->ramtab = 0;            
        }
        running_state->frames = reinterpret_cast<frame_st*>(running_state + 1);
        
        for(size_t j = 0; j < running_state->n_logical_frames; ++j) 
    	    running_state->frames[j].free = running_state->n_logical_frames - j;

        running_state->next = reinterpret_cast<frames_module_v1_state*>(&running_state->frames[running_state->n_logical_frames]);
        last_state = running_state;
        running_state = running_state->next;
        ++n_regions;
    });
    last_state->next = 0;

    kconsole << " +-frames_mod: counted " << n_regions << " memory regions again" << endl;
    
	return ret;
}

static void frames_module_v1_finish_init(frames_module_v1_closure* self, system_frame_allocator_v1_closure* frames, heap_v1_closure* heap)
{
    
}

static const frames_module_v1_ops ops = {
    frames_module_v1_required_size,
    frames_module_v1_create,
    frames_module_v1_finish_init
};

static const frames_module_v1_closure clos = {
    &ops,
    NULL
};

EXPORT_CLOSURE_TO_ROOTDOM(frames_module_v1, frames_module, clos);
