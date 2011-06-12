#include "frames_module_v1_interface.h"
#include "frames_module_v1_impl.h"
#include "system_frame_allocator_v1_impl.h"
#include "frame_allocator_v1_impl.h"
#include "types.h"
#include "macros.h"
#include "default_console.h"
#include "bootinfo.h"
#include "domain.h"
#include "algorithm"

/*!
 * Frame allocator client record.
 */
struct frame_allocator_v1_state
{
    frame_allocator_v1_closure closure;

    dcb_ro_t* domain; //<! Virtual address of client's RO DCB.
    flink_t* region_list; //<! List of frame regions allocated for this client.

    uint32_t n_allocated_phys_frames; //<! Number of already allocated RAM frames.

    uint32_t owner; //!< Owner ID.
    size_t guaranteed_frames;
    size_t extra_frames;

    heap_v1_closure* heap;
    frames_module_v1_state* module_state; //<! Back pointer to shared state.
};

struct frame_st
{
    uint32_t free;
};

/*!
 * Frame allocator region record.
 */
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
// frame_allocator_v1 implementation
// A C++-tastic casting mess, but can be helpful for instrumentation once we start using frames allocator in apps.
// All the forward function mumbo-jumbo is just because we cannot forward-declare a static const struct in C++.
//======================================================================================================================

static memory_v1_address system_frame_allocator_v1_allocate(system_frame_allocator_v1_closure* self, memory_v1_size bytes, uint32_t frame_width);
static memory_v1_address system_frame_allocator_v1_allocate_range(system_frame_allocator_v1_closure* self, memory_v1_size bytes, uint32_t frame_width, memory_v1_address start, memory_v1_attrs attr);
static uint32_t system_frame_allocator_v1_query(system_frame_allocator_v1_closure* self, memory_v1_address addr, memory_v1_attrs* attr);
static void system_frame_allocator_v1_free(system_frame_allocator_v1_closure* self, memory_v1_address addr, memory_v1_size bytes);
static void system_frame_allocator_v1_destroy(system_frame_allocator_v1_closure* self);

static memory_v1_address frame_allocator_v1_allocate(frame_allocator_v1_closure* self, memory_v1_size bytes, uint32_t frame_width)
{
    return system_frame_allocator_v1_allocate(reinterpret_cast<system_frame_allocator_v1_closure*>(self), bytes, frame_width);
}

static memory_v1_address frame_allocator_v1_allocate_range(frame_allocator_v1_closure* self, memory_v1_size bytes, uint32_t frame_width, memory_v1_address start, memory_v1_attrs attr)
{
    return system_frame_allocator_v1_allocate_range(reinterpret_cast<system_frame_allocator_v1_closure*>(self), bytes, frame_width, start, attr);
}

static uint32_t frame_allocator_v1_query(frame_allocator_v1_closure* self, memory_v1_address addr, memory_v1_attrs* attr)
{
    return system_frame_allocator_v1_query(reinterpret_cast<system_frame_allocator_v1_closure*>(self), addr, attr);
}

static void frame_allocator_v1_free(frame_allocator_v1_closure* self, memory_v1_address addr, memory_v1_size bytes)
{
    system_frame_allocator_v1_free(reinterpret_cast<system_frame_allocator_v1_closure*>(self), addr, bytes);
}

static void frame_allocator_v1_destroy(frame_allocator_v1_closure* self)
{
    system_frame_allocator_v1_destroy(reinterpret_cast<system_frame_allocator_v1_closure*>(self));
}

static const frame_allocator_v1_ops frame_allocator_v1_methods =
{
    frame_allocator_v1_allocate,
    frame_allocator_v1_allocate_range,
    frame_allocator_v1_query,
    frame_allocator_v1_free,
    frame_allocator_v1_destroy
};

//======================================================================================================================
// implementation helper functions
//======================================================================================================================

static void mark_frames_used(frame_allocator_v1_state* client_state, frames_module_v1_state* state, address_t first_frame, size_t n_frames)
{
    for (size_t j = first_frame; j < (first_frame + n_frames); ++j)
        state->frames[j].free  = 0;

    if (state->ramtab)
    {
        uint32_t ridx = state->start >> FRAME_WIDTH;
        size_t fshift = state->frame_width - FRAME_WIDTH; /* frame_width >= FRAME_WIDTH */
        for (size_t j = first_frame; j < (first_frame + n_frames); ++j)
        {
            for(size_t k = 0; k < (1UL << fshift); ++k)
            {
                state->ramtab->put(ridx + (j << fshift) + k, client_state->owner, state->frame_width, ramtab_v1_state_e_unused);
            }
        }
    }
}

/*!
 * After allocation, update predecessors free frames info.
 */
static void alloc_update_free_predecessors(frames_module_v1_state* cur_state, address_t first_frame)
{
    uint32_t start_free = cur_state->frames[first_frame].free;
    for (address_t i = first_frame; i != 0; )
    {
        --i;
        if (cur_state->frames[i].free == 0)
            break;
        cur_state->frames[i].free -= start_free;
    }
}

static bool add_range(frame_allocator_v1_state* client_state, address_t base, size_t n_phys_frames, size_t frame_width)
{
    if (!client_state->heap /*|| !client_state->headp*/ || !n_phys_frames)
        return true;

    return false;
}

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

/* Convert "bytes" into a number of frames of logical width "frame_width" */
inline size_t bytes_to_log_frames(size_t bytes, size_t frame_width)
{
    return align_to_frame_width(bytes, frame_width) >> frame_width;
}

inline size_t log_frames_to_bytes(size_t frames, size_t frame_width)
{
    return frames << frame_width;
}

inline address_t frame_address(frames_module_v1_state* cur_state, address_t frame_index)
{
    return cur_state->start + (frame_index << cur_state->frame_width);
}

static frames_module_v1_state* alloc_any(frame_allocator_v1_closure* self, size_t n_physical_frames, uint32_t align, address_t* first_log_frame, size_t* n_log_frames)
{
    frame_allocator_v1_state* client_state = self->state;
    frames_module_v1_state* state = client_state->module_state;
    frames_module_v1_state* cur_state = state;

    kconsole << __FUNCTION__ << ": requested " << n_physical_frames << " frames." << endl;

    while (cur_state)
    {
        if (!cur_state->attrs)
        {
            *n_log_frames = n_physical_frames >> (cur_state->frame_width - FRAME_WIDTH);

            if (*n_log_frames != n_physical_frames)
            {
                PANIC("Region with non-standard frame width! Unsupported.");
            }

            // We need at least n_physical_frames contiguous frames starting aligned to "align"
            for (*first_log_frame = 0; *first_log_frame < cur_state->n_logical_frames; ++(*first_log_frame))
            {
                if (cur_state->frames[*first_log_frame].free >= *n_log_frames && is_aligned_to_frame_width(frame_address(cur_state, *first_log_frame), align))
                    return cur_state;
            }
        }
        cur_state = cur_state->next;
    }

    return NULL; // out of physical memory
}

static frames_module_v1_state* alloc_any(system_frame_allocator_v1_closure* self, size_t n_physical_frames, uint32_t align, address_t* first_log_frame, size_t* n_log_frames)
{
    return alloc_any(reinterpret_cast<frame_allocator_v1_closure*>(self), n_physical_frames, align, first_log_frame, n_log_frames);
}

static frames_module_v1_state* alloc_range(frame_allocator_v1_closure* self, size_t n_physical_frames, address_t start, address_t* first_log_frame, size_t* n_log_frames)
{
    frame_allocator_v1_state* client_state = self->state;
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
    *n_log_frames = align_to_frame_width(n_physical_frames, fshift) >> fshift; //bytes_to_log_frames, actually, too?
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

static memory_v1_address system_frame_allocator_v1_allocate_range(system_frame_allocator_v1_closure* self, memory_v1_size bytes, uint32_t frame_width, memory_v1_address start, memory_v1_attrs attr)
{
    frame_allocator_v1_state* client_state = reinterpret_cast<frame_allocator_v1_state*>(self->state);
    frames_module_v1_state* cur_state;
    address_t first_frame;
    size_t n_frames;

    if (bytes == 0)
    {
        kconsole << __FUNCTION__ << ": request to allocate 0 bytes." << endl;
        return NO_ADDRESS;
    }

    frame_width = std::max(frame_width, FRAME_WIDTH);
    size_t n_phys_frames = align_to_frame_width(bytes, frame_width) >> FRAME_WIDTH;

    if (client_state->n_allocated_phys_frames + n_phys_frames > client_state->guaranteed_frames)
    {
        kconsole << __FUNCTION__ << ": client exceeded quota!" << endl;
        return NO_ADDRESS;
    }

    if (unaligned(start))
    {
        cur_state = alloc_any(self, n_phys_frames, frame_width, &first_frame, &n_frames);
        if (!cur_state)
        {
            kconsole << __FUNCTION__ << ": failed to allocate " << bytes << " bytes." << endl;
            return NO_ADDRESS;
        }
    }
    else
    {
        // for alloc_range we check alignment externally
        if (!is_aligned_to_frame_width(start, frame_width))
        {
            kconsole << __FUNCTION__ << ": start " << start << " not aligned to width " << frame_width << endl;
            return NO_ADDRESS;
        }
        cur_state = alloc_range(self, n_phys_frames, start, &first_frame, &n_frames);
        if (!cur_state)
        {
            kconsole << __FUNCTION__ << ": failed to allocate " << bytes << " bytes at " << start << endl;
            return NO_ADDRESS;
        }
    }

    /*
     * Check if our requested frame width has been rounded up;
     * this can only happen if we've allocated from a non-standard
     * region with a default logical frame width greater than our
     * requested one.
     */
    if (cur_state->frame_width > frame_width)
    {
        frame_width  = cur_state->frame_width;
        n_phys_frames = align_to_frame_width(bytes, frame_width) >> FRAME_WIDTH;
    }

    start = frame_address(cur_state, first_frame);

    alloc_update_free_predecessors(cur_state, first_frame);
    mark_frames_used(client_state, cur_state, first_frame, n_frames);

    client_state->n_allocated_phys_frames += n_phys_frames;

    // Add the info about this newly allocated region to our list.
    if(!add_range(client_state, start, n_phys_frames, frame_width))
    {
        PANIC("Something's wrong.");
    }

    kconsole << "  allocated " << start << endl;
    return start;
}

static memory_v1_address system_frame_allocator_v1_allocate(system_frame_allocator_v1_closure* self, memory_v1_size bytes, uint32_t frame_width)
{
    return system_frame_allocator_v1_allocate_range(self, bytes, frame_width, ANY_ADDRESS, memory_v1_attrs_regular);
}

static uint32_t system_frame_allocator_v1_query(system_frame_allocator_v1_closure* self, memory_v1_address addr, memory_v1_attrs* attr)
{
    frame_allocator_v1_state* client_state = reinterpret_cast<frame_allocator_v1_state*>(self->state);
    frames_module_v1_state* state = client_state->module_state;

    frames_module_v1_state* cur_state = get_region(state, addr);

    if (!cur_state)
    {
        kconsole << __FUNCTION__ << ": address " << addr << " is non-existant" << endl;
        return 0;
    }

    *attr = cur_state->attrs;
    return cur_state->frame_width;
}

inline address_t phys_frame_number(address_t addr)
{
    return addr >> FRAME_WIDTH;
}

static void system_frame_allocator_v1_free(system_frame_allocator_v1_closure* self, memory_v1_address addr, memory_v1_size bytes)
{
    frame_allocator_v1_state* client_state = reinterpret_cast<frame_allocator_v1_state*>(self->state);
    frames_module_v1_state* cur_state = get_region(client_state->module_state, addr);

    if (!cur_state)
    {
        kconsole << __FUNCTION__ << ": cannot handle non-existant address " << addr << endl;
        PANIC("Frame allocator misuse."); // FIXME: just return error or raise xcp
    }

    /*
    ** Ok: we have a current region in our hands, and have potentially
    ** two different logical frame widths to cope with:
    **
    **    - region logical frame width, RLFW == cur_state->frame_width
    **    - allocation logical frame width, ALFW.
    **
    ** We use RLFW for handling the indices within the region structure,
    ** while we use ALFW for rounding down/up the region to be freed.
    **
    ** If the region is not within RAM, then we know RLFW == ALFW,
    ** and hence there is no problem.
    ** If the region *is* within RAM, we need to lookup the ALFW in
    ** the RamTab; we do this by looking up the *physical* frame
    ** number of [base] and checking its frame width.
    */

    address_t owner;
    ramtab_v1_state_e mem_state;
    size_t region_frame_width = cur_state->frame_width;
    size_t allocation_frame_width = region_frame_width;
    if (cur_state->ramtab)
    {
        owner = cur_state->ramtab->get(phys_frame_number(addr), &allocation_frame_width, &mem_state);
    }

    // Now we need to round "addr+bytes" up, and "addr" down to allocation_frame_width.
    address_t end  = align_to_frame_width(addr + bytes, allocation_frame_width);
    addr = ((addr >> allocation_frame_width) << allocation_frame_width);
    size_t n_phys_frames = phys_frame_number(end - addr);

    // Check that we actually own these frames, and they're not in use.
    if (cur_state->ramtab)
    {
        uint32_t first_frame = phys_frame_number(addr);
        for (uint32_t i = 0; i < n_phys_frames; ++i)
        {
            size_t frame_width;
            owner = cur_state->ramtab->get(first_frame + i, &frame_width, &mem_state);
            if (owner != client_state->owner)
            {
                kconsole << __FUNCTION__ << ": we do not own the frame at " << ((first_frame + i) << FRAME_WIDTH) << endl;
                PANIC("Frame allocator misuse.");
            }
            if (frame_width != allocation_frame_width)
            {
                kconsole << __FUNCTION__ << ": frame " << (first_frame + i) << " width is " << frame_width << ", should be " << allocation_frame_width << endl;
                PANIC("Frame allocator misuse.");
            }
            if ((mem_state == ramtab_v1_state_e_mapped) || (mem_state == ramtab_v1_state_e_nailed))
            {
                kconsole << __FUNCTION__ << ": frame at " << ((first_frame + i) << FRAME_WIDTH) << " is " << (mem_state == ramtab_v1_state_e_mapped ? "mapped" : "nailed") << endl;
                PANIC("Frame allocator misuse.");
            }
        }
    }

    // Now get the frame indices in the region (i.e. logical frames of width region_frame_width).
    address_t start_log_frame = bytes_to_log_frames(addr - cur_state->start, region_frame_width);
    address_t end_log_frame = bytes_to_log_frames(end - cur_state->start, region_frame_width);

    if (end_log_frame > cur_state->n_logical_frames)
    {
        kconsole << __FUNCTION__ << ": not all addresses are in the same region." << endl;
        PANIC("Frame allocator misuse.");
    }

    /*
     * First sort out the frames we're freeing.
     * We do this from the back so can keep "free" counts consistent.
     */
    address_t i;
    size_t end_free = (end_log_frame == cur_state->n_logical_frames) ? 0 : cur_state->frames[end_log_frame].free;

    for (i = end_log_frame; i >= start_log_frame; --i)
    {
        cur_state->frames[i].free = ++end_free;
        kconsole << "1. Log frame " << i << " free set to " << cur_state->frames[i].free << endl;
    }

    /*
     * Now need to update all empty frames immediately before the first
     * frame we've freed; hopefully this will not be too many since we
     * alloc first fit.
     * [Note: i points to frame before start_log_frame already]
     */
    for (; /*wrap protect:*/(i < start_log_frame) && (cur_state->frames[i].free != 0); --i)
    {
        cur_state->frames[i].free = ++end_free;
        kconsole << "2. Log frame " << i << " free set to " << cur_state->frames[i].free << endl;
    }

    /* Now update the ramtab (if appropriate) */
    if(cur_state->ramtab)
    {
        uint32_t ridx = cur_state->start >> FRAME_WIDTH;
        size_t fshift = cur_state->frame_width - FRAME_WIDTH; /* frame_width >= FRAME_WIDTH */
        for (size_t j = start_log_frame; j < end_log_frame; ++j)
        {
            //FIXME: indexing will be wrong with fshift > 0
            cur_state->ramtab->put(ridx + (j << fshift), OWNER_NONE, cur_state->frame_width, ramtab_v1_state_e_unused);
        }
    }

    /* Finally, update number of allocated frames (protect from wrapping), and our linked list of regions */
    size_t new_phys_frames = client_state->n_allocated_phys_frames - n_phys_frames;
    if (new_phys_frames > client_state->n_allocated_phys_frames)
    {
        kconsole << __FUNCTION__ << ": freeing more frames than I own (ignored)" << endl;
        new_phys_frames = 0;
    }
    client_state->n_allocated_phys_frames = new_phys_frames;

/*  if(!del_range(cst, base, npf, alfw)) {
        eprintf("Frames$Free: something's wrong.\n");
        ntsc_dbgstop();
    }*/
}

static void system_frame_allocator_v1_destroy(system_frame_allocator_v1_closure* self)
{
    PANIC("frames_mod: destroy is not implemented!");
}

static frame_allocator_v1_closure* system_frame_allocator_v1_new_client(system_frame_allocator_v1_closure* self, memory_v1_address owner_dcb_virt, memory_v1_address owner_dcb_phys, uint32_t granted_frames, uint32_t extra_frames, uint32_t init_alloc_frames)
{
    frame_allocator_v1_state* client_state = reinterpret_cast<frame_allocator_v1_state*>(self->state);
    frames_module_v1_state* state = client_state->module_state;

    if (!client_state->heap)
    {
        kconsole << __FUNCTION__ << ": called before initialisation is complete." << endl;
        return NULL;
    }

    if (init_alloc_frames > granted_frames)
        init_alloc_frames = granted_frames;
    if (extra_frames < granted_frames)
        extra_frames = granted_frames;

    // Invariant: extra_frames >= granted_frames >= init_alloc_frames

    frame_allocator_v1_state* new_client_state = reinterpret_cast<frame_allocator_v1_state*>(client_state->heap->allocate(sizeof(*new_client_state)));
    if (!new_client_state)
    {
        kconsole << __FUNCTION__ << ": out of memory in frames allocator." << endl;
        PANIC("Out of memory.");
    }

    dcb_ro_t *domain = reinterpret_cast<dcb_ro_t*>(owner_dcb_virt);

    domain->min_phys_frame_count = 0;
    domain->max_phys_frame_count = state->ramtab->size();
    domain->ramtab = reinterpret_cast<ramtab_entry_t*>(state->ramtab->base());
    domain->memory_region_list.next = domain->memory_region_list.prev = &domain->memory_region_list; //FIXME: dllist init

    new_client_state->domain = domain;
    new_client_state->region_list = &domain->memory_region_list;
    new_client_state->n_allocated_phys_frames = init_alloc_frames;
    new_client_state->owner = owner_dcb_virt; // use owner_dcb_phys instead?
    new_client_state->guaranteed_frames = granted_frames;
    new_client_state->extra_frames = extra_frames;
    new_client_state->heap = client_state->heap;
    new_client_state->module_state = client_state->module_state;

    // Allocate init_alloc_frames.
    address_t first_frame;
    size_t n_frames;
    frames_module_v1_state* cur_state;

    cur_state = alloc_any(self, init_alloc_frames, FRAME_WIDTH, &first_frame, &n_frames);
    if (cur_state == NULL)
    {
        kconsole << __FUNCTION__ << ": failed to allocate " << init_alloc_frames << " frames." << endl;
        PANIC("Out of physical memory.");
    }
    if (n_frames != init_alloc_frames)
    {
        PANIC("Region with non-standard frame width! Unsupported.");
    }

    address_t start = frame_address(cur_state, first_frame);
    kconsole << __FUNCTION__ << ": allocated " << init_alloc_frames << " physical frames at " << start << endl;

    alloc_update_free_predecessors(cur_state, first_frame);
    mark_frames_used(new_client_state, cur_state, first_frame, n_frames);

    /* Update the number of frames we've allocated on this interface */
    client_state->n_allocated_phys_frames += init_alloc_frames;

    // Add the info about this newly allocated region to our list.
    if(!add_range(new_client_state, start, init_alloc_frames, FRAME_WIDTH))
    {
        PANIC("Something's wrong.");
    }

    // And that is it.
    new_client_state->closure.methods = &frame_allocator_v1_methods;
    new_client_state->closure.state = new_client_state;
    return &new_client_state->closure;
}

static bool system_frame_allocator_v1_add_frames(system_frame_allocator_v1_closure* self, memory_v1_physmem_desc region)
{
    PANIC("Unimplemented!");
    return false;
}

static const system_frame_allocator_v1_ops system_frame_allocator_v1_methods =
{
    system_frame_allocator_v1_allocate,
    system_frame_allocator_v1_allocate_range,
    system_frame_allocator_v1_query,
    system_frame_allocator_v1_free,
    system_frame_allocator_v1_destroy,
    system_frame_allocator_v1_new_client,
    system_frame_allocator_v1_add_frames,
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

    res = sizeof(frame_allocator_v1_closure) + sizeof(frame_allocator_v1_state) + n_regions * sizeof(frames_module_v1_state) + n_frames * sizeof(frame_st);
    res = page_align_up(res);

    kconsole << " +-frames_mod: counted " << n_regions << " memory regions" << endl;
    return res;
}

//Don't need where_to_start, bootinfo page has logic for finding an allocatable place. We do not use it currently because MMU mod has to enter some initial mappings - but this could be streamlined.

static system_frame_allocator_v1_closure* frames_module_v1_create(frames_module_v1_closure* self, ramtab_v1_closure* rtab, memory_v1_address where_to_start)
{
    UNUSED(self);

    kconsole << "frames_mod create @ " << where_to_start << endl;
    frame_allocator_v1_state* client_state = reinterpret_cast<frame_allocator_v1_state*>(where_to_start);

    system_frame_allocator_v1_closure* ret = reinterpret_cast<system_frame_allocator_v1_closure*>(&client_state->closure);
    ret->state = reinterpret_cast<system_frame_allocator_v1_state*>(client_state);
    ret->methods = &system_frame_allocator_v1_methods;

    frames_module_v1_state* frames_state = reinterpret_cast<frames_module_v1_state*>(where_to_start + sizeof(frame_allocator_v1_state));

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

        mark_frames_used(client_state, running_state, first_frame, n_frames);
    });

    return ret;
}

static void frames_module_v1_finish_init(frames_module_v1_closure* self, system_frame_allocator_v1_closure* frames, heap_v1_closure* heap)
{
    frame_allocator_v1_state* state = reinterpret_cast<frame_allocator_v1_state*>(frames->state);

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
