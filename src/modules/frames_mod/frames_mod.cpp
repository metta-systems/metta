#include "frames_module_v1_interface.h"
#include "frames_module_v1_impl.h"
#include "types.h"
#include "macros.h"
#include "default_console.h"
#include "bootinfo.h"
#include "algorithm"

struct frames_client_state
{
    frame_allocator_v1_closure closure;
    
    uint32_t n_phys_frames;

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
/*
    res = sizeof(Frames_cl) + sizeof(FramesClient_st) + 
	(nreg * sizeof(struct _FramesMod_st)) +
	(nlf * sizeof(struct _Frame));
*/
    return res;
}

//Don't need where, bootinfo page has logic for finding an allocatable place. We use it currently because MMU mod has to enter some initial mappings - but this can be streamlined.

static system_frame_allocator_v1_closure* frames_module_v1_create(frames_module_v1_closure* self, ramtab_v1_closure* rtab, memory_v1_address where_to_start)
{
    UNUSED(self);
/*    bootinfo_t* bi = new(BOOTINFO_PAGE) bootinfo_t; // simplify memory map operations
    
	address_t first_range = bi->find_highmem_range_of_at_least(required_size());

    // Find proper location to start "allocating" from.
	first_range = page_align_up(first_range);

	if (!bi->use_memory(first_range, mmu_memory_needed_bytes))
	{
        PANIC("Unable to use memory for initial frames_mod setup!");
	}*/

//     std::for_each(bi->mmap_begin(), bi->mmap_end(), [](const multiboot_t::mmap_entry_t e)
//     {
//         kconsole << "mmap entry @ " << e.address() << " is " << e.size() << " bytes of type " << e.type() << endl;
//         allmem.push_back(PMemDesc(e.address(), e.size(), e.type()));
//     });
//     std::for_each(bi->vmap_begin(), bi->vmap_end(), [](const memory_v1_mapping* e)
//     {
//         kconsole << "vmap entry @ " << e.address() << " is " << e.size() << " bytes of type " << e.type() << endl;
//         memmap.push_back(PMemDesc(e.address(), e.size(), e.type()));
//     });
    
	kconsole << "frames_mod create @ " << where_to_start << endl;
	return 0;
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
