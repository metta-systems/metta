#include "frames_module_v1_interface.h"
#include "frames_module_v1_impl.h"
#include "types.h"
#include "macros.h"
#include "default_console.h"
#include "bootinfo.h"
#include "algorithm"
#include "system_frame_allocator_v1_impl.h"

struct frames_client_state // rename this to frame_allocator_v1_state and remove some ugly casts below
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
// implementation helper functions
//======================================================================================================================

/*
** get_region(): a utility function which returns a pointer to
** the 'state' of whatever region the address addr is in, or NULL
** if addr is not in any of the regions which we manage.
*/
static frames_module_v1_state* get_region(frames_module_v1_state* state, address_t addr)
{
    frames_module_v1_state* ret = state;

    while(ret)
    {
	    if ((addr >= ret->start) && addr < (ret->start + (ret->n_logical_frames << ret->frame_width)))
	        break;
	    ret = ret->next;
    }

    return ret;
}

/* Roundup a value "size" to an intergral number of frames of width "frame_width" */
inline size_t round_up(size_t size, size_t frame_width)
{
    return (size + ((1 << frame_width) - 1)) & ~((1UL << frame_width) - 1);
}

/* Convert "bytes" into a number of frames of logical width "frame_width" */
inline size_t bytes_to_log_frames(size_t bytes, size_t frame_width)
{
    return round_up(bytes, frame_width) >> frame_width;
}

static frames_module_v1_state* alloc_range(frame_allocator_v1_closure* self, size_t n_physical_frames, address_t start, address_t* first_log_frame, size_t* n_log_frames)
{
    frames_client_state* client_state = reinterpret_cast<frames_client_state*>(self->state);
    frames_module_v1_state* state = client_state->module_state;
    frames_module_v1_state* cur_state = state;
    uint32_t fshift;
    
    if ((cur_state = get_region(state, start)) == NULL) 
    {
        kconsole << "alloc_range: we don't own requested address " << start << endl;
        PANIC("frames_mod");
    }

    /*
     * Translate from n_physical_frames (i.e. frames of size FRAME_SIZE) required
     * into *n_log_frames = number of logical frames required (i.e. frames of size 1 << cur_state->frame_width).
     */
    // FIXME: on ARM, the allocation frame width may be smaller than FRAME_WIDTH (e.g. with 1K pages)
    fshift = cur_state->frame_width - FRAME_WIDTH; /* >= 0 */
    *n_log_frames = round_up(n_physical_frames, fshift) >> fshift; //bytes_to_log_frames, actually, too?
    *first_log_frame = bytes_to_log_frames(start - cur_state->start, cur_state->frame_width);
    
    if (cur_state->frames[*first_log_frame].free < *n_log_frames) 
    {
	    /* not enough space at requested address: give as much as possible */
        kconsole << "alloc_range: less than " << int(*n_log_frames << cur_state->frame_width) << " bytes free at requested address " << start;
        *n_log_frames = cur_state->frames[*first_log_frame].free;
        kconsole << ", returning as much as available - " << int(*n_log_frames << cur_state->frame_width) << endl;
	    return cur_state;
    }

    int bytes = int(*n_log_frames << cur_state->frame_width);
    kconsole << "alloc_range: allocated " << bytes << " bytes at requested address " << start << "->" << start + bytes << endl;
    return cur_state;
}

static frames_module_v1_state* alloc_range(system_frame_allocator_v1_closure* self, size_t n_frames, address_t start, address_t* first_log_frame, size_t* n_log_frames)
{
    return alloc_range(reinterpret_cast<frame_allocator_v1_closure*>(self), n_frames, start, first_log_frame, n_log_frames);
}

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
        if (e->type() == multiboot_t::mmap_entry_t::non_free)
            return;

        n_frames += e->size() >> FRAME_WIDTH;
        ++n_regions;
    });

    res = sizeof(frame_allocator_v1_closure) + sizeof(frames_client_state) + n_regions * sizeof(frames_module_v1_state) + n_frames * sizeof(frame_st);
    res = page_align_up(res);
    
    kconsole << " +-frames_mod: counted " << n_regions << " memory regions" << endl;
    return res;
}

//Don't need where_to_start, bootinfo page has logic for finding an allocatable place. We use it currently because MMU mod has to enter some initial mappings - but this could be streamlined.

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
        if (e->type() == multiboot_t::mmap_entry_t::non_free)
            return;
        
        running_state->start = e->address();
        running_state->n_logical_frames = e->size() >> FRAME_WIDTH;
        running_state->frame_width = FRAME_WIDTH;
        if ((e->type() == multiboot_t::mmap_entry_t::free) || (e->type() == multiboot_t::mmap_entry_t::acpi_reclaimable))
        {
            running_state->attrs = memory_v1_attrs_regular;
            running_state->ramtab = rtab;
            kconsole << "Adding RAM at " << e->address() << " is " << e->size() << " bytes of type " << e->type() << endl;
        }
        else
        {
            running_state->attrs = memory_v1_attrs_non_memory;
            running_state->ramtab = 0;
            kconsole << "Adding non-RAM at " << e->address() << " is " << e->size() << " bytes of type " << e->type() << endl;
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
    kconsole << " +-frames_mod: and finished at address " << page_align_up(reinterpret_cast<address_t>(&last_state->frames[last_state->n_logical_frames])) << endl;

    /*
     * Mark already used frames allocated.
     */
    std::for_each(bi->mmap_begin(), bi->mmap_end(), [ret, client_state](const multiboot_t::mmap_entry_t* e)
    {
        if (e->type() != multiboot_t::mmap_entry_t::non_free)
            return;

        kconsole << "Used memory at " << e->address() << " is " << e->size() << " bytes of type " << e->type() << endl;
        
        address_t first_frame;
        size_t n_frames;
        auto running_state = alloc_range(ret, e->size() >> FRAME_WIDTH, e->address(), &first_frame, &n_frames);
        if (n_frames == 0)
            PANIC("Already allocated range deemed unavailable!");

        if (running_state->ramtab)
        {
            uint32_t ridx = running_state->start >> FRAME_WIDTH;
            for (size_t j = first_frame; j < (first_frame + n_frames); ++j)
            {
                running_state->frames[j].free = 0;
              //  for(k = 0; k < (1UL << fshift); ++k)
                {
                    running_state->ramtab->put(ridx + (j) /*<< fshift) + k*/, client_state->owner, running_state->frame_width, ramtab_v1_state_e_mapped); // doesn't it conflict with mmu_mod:325 ?
                }
            }
        }
        else
        {
            for (size_t j = first_frame; j < (first_frame + n_frames); ++j)
                running_state->frames[j].free  = 0;
        }
    });

    return ret;
}

static void frames_module_v1_finish_init(frames_module_v1_closure* self, system_frame_allocator_v1_closure* frames, heap_v1_closure* heap)
{
    frames_client_state* state = reinterpret_cast<frames_client_state*>(frames->state);

    if (state->heap != NULL) 
    {
        kconsole << __FUNCTION__ << ": called with heap=" << heap << ", but already have " << state->heap << endl;
        PANIC("Double initialisation of frames module!");
    }

    state->heap = heap;
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
