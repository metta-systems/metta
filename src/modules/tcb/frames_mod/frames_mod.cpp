//
// Part of Metta OS. Check http://atta-metta.net for latest version.
//
// Copyright 2007 - 2012, Stanislav Karchebnyy <berkus@atta-metta.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "frames_module_v1_interface.h"
#include "frames_module_v1_impl.h"
#include "ramtab_v1_interface.h"
#include "heap_v1_interface.h"
#include "frame_allocator_v1_interface.h"
#include "frame_allocator_v1_impl.h"
#include "system_frame_allocator_v1_interface.h"
#include "system_frame_allocator_v1_impl.h"
#include "types.h"
#include "macros.h"
#include "default_console.h"
#include "heap_new.h"
#include "bootinfo.h"
#include "domain.h"
#include "algorithm"
#include "logger.h"

/**
 * Frame allocator client record.
 */
struct frame_allocator_v1::state_t
{
    frame_allocator_v1::closure_t closure;

    dcb_ro_t* domain;                      //<! Virtual address of client's RO DCB.
    region_list_t* region_list;            //<! List of frame regions allocated for this client.

    uint32_t n_allocated_phys_frames;      //<! Number of already allocated RAM frames.

    uint32_t owner;                        //!< Owner ID.
    size_t guaranteed_frames;
    size_t extra_frames;

    heap_v1::closure_t* heap;
    frames_module_v1::state_t* module_state;  //<! Back pointer to shared state.
};

struct frame_st
{
    uint32_t free;
};

/**
 * Frame allocator region record.
 */
struct frames_module_v1::state_t
{
    address_t start;
    size_t n_logical_frames;
    uint32_t frame_width;
    memory_v1::attrs attrs;
    ramtab_v1::closure_t* ramtab;
    frames_module_v1::state_t* next;
    frame_st* frames;
};

//======================================================================================================================
// frame_allocator_v1 implementation
// A C++-tastic casting mess, but can be helpful for instrumentation once we start using frames allocator in apps.
// All the forward function mumbo-jumbo is just because we cannot forward-declare a static const struct in C++.
//======================================================================================================================

static memory_v1::address system_frame_allocator_v1_allocate(frame_allocator_v1::closure_t* self, memory_v1::size bytes, uint32_t frame_width);
static memory_v1::address system_frame_allocator_v1_allocate_range(frame_allocator_v1::closure_t* self, memory_v1::size bytes, uint32_t frame_width, memory_v1::address start, memory_v1::attrs attr);
static uint32_t system_frame_allocator_v1_query(frame_allocator_v1::closure_t* self, memory_v1::address addr, memory_v1::attrs* attr);
static void system_frame_allocator_v1_free(frame_allocator_v1::closure_t* self, memory_v1::address addr, memory_v1::size bytes);
static void system_frame_allocator_v1_destroy(frame_allocator_v1::closure_t* self);

static memory_v1::address frame_allocator_v1_allocate(frame_allocator_v1::closure_t* self, memory_v1::size bytes, uint32_t frame_width)
{
    return system_frame_allocator_v1_allocate(self, bytes, frame_width);
}

static memory_v1::address frame_allocator_v1_allocate_range(frame_allocator_v1::closure_t* self, memory_v1::size bytes, uint32_t frame_width, memory_v1::address start, memory_v1::attrs attr)
{
    return system_frame_allocator_v1_allocate_range(self, bytes, frame_width, start, attr);
}

static uint32_t frame_allocator_v1_query(frame_allocator_v1::closure_t* self, memory_v1::address addr, memory_v1::attrs* attr)
{
    return system_frame_allocator_v1_query(self, addr, attr);
}

static void frame_allocator_v1_free(frame_allocator_v1::closure_t* self, memory_v1::address addr, memory_v1::size bytes)
{
    system_frame_allocator_v1_free(self, addr, bytes);
}

static void frame_allocator_v1_destroy(frame_allocator_v1::closure_t* self)
{
    system_frame_allocator_v1_destroy(self);
}

static const frame_allocator_v1::ops_t frame_allocator_v1_methods =
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

static void mark_frames_used(frame_allocator_v1::state_t* client_state, frames_module_v1::state_t* state, address_t first_frame, size_t n_frames)
{
    for (size_t j = first_frame; j < (first_frame + n_frames); ++j)
        state->frames[j].free = 0;

    if (state->ramtab)
    {
        uint32_t ridx = state->start >> FRAME_WIDTH;
        size_t fshift = state->frame_width - FRAME_WIDTH; /* frame_width >= FRAME_WIDTH */
        for (size_t j = first_frame; j < (first_frame + n_frames); ++j)
        {
            for(size_t k = 0; k < (1UL << fshift); ++k)
            {
                // Effectively, set only owner and frame_width. Frames are yet unused (neither mapped nor nailed).
                state->ramtab->put(ridx + (j << fshift) + k, client_state->owner, state->frame_width, ramtab_v1::state_unused);
            }
        }
    }
}

/**
 * After allocation, update predecessors free frames info.
 */
static void alloc_update_free_predecessors(frames_module_v1::state_t* cur_state, address_t first_frame)
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

// FIXME: Lots of reinterpret casts suck, do something about it!

static bool add_range_element(frame_allocator_v1::state_t* client_state, address_t start, size_t n_phys_frames, size_t frame_width)
{
    region_list_t* new_entry = new(client_state->heap) region_list_t;
    if (new_entry == nullptr) {
        return false;
    }

    new_entry->start = start;
    new_entry->n_phys_frames = n_phys_frames;
    new_entry->frame_width = frame_width;

    client_state->region_list->insert_after(*new_entry);

    return true;
}

static bool add_range(frame_allocator_v1::state_t* client_state, address_t start, size_t n_phys_frames, size_t frame_width)
{
    if (!client_state->heap || !client_state->region_list || !n_phys_frames)
        return true;

    logger::trace() << __FUNCTION__ << ": " << n_phys_frames << " frames at " << start << " frame width " << frame_width;
    address_t end = start + (n_phys_frames << FRAME_WIDTH);

    if (client_state->region_list->is_empty())
    {
        logger::trace() << __FUNCTION__ << ": region list is empty, allocating new entry";
        return add_range_element(client_state, start, n_phys_frames, frame_width);
    }
    else
    {
        // Try to find the correct place to insert it.
        region_list_t* link;
        address_t current_start, current_end, next_start;
        for (link = *client_state->region_list->next(); link != client_state->region_list; link = *link->next())
        {
            current_start = (*link)->start;
            current_end = current_start + ((*link)->n_phys_frames << FRAME_WIDTH);
            next_start = (*link->next())->start;
            if ((start >= current_end) && (end <= next_start))
                break;
        }

        // We wrapped, no any elements before this one.
        if (link == client_state->region_list)
        {
            // Check if we can merge on rhs
            // FIXME: doesn't check frame_width??
            if (end == next_start)
            {
                logger::debug() << __FUNCTION__ << ": no prior elements, merging on rhs.";
                (*link->next())->n_phys_frames += n_phys_frames;
                (*link->next())->start = start;
                return true;
            }
            else
            {
                logger::debug() << __FUNCTION__ << ": no prior elements, allocating new entry.";
                return add_range_element(client_state, start, n_phys_frames, frame_width);
            }
        }
        PANIC("Unimplemented!");
    }
    return false;
}

/*
** get_region(): a utility function which returns a pointer to
** the 'state' of whatever region the address addr is in, or NULL
** if addr is not in any of the regions which we manage.
*/
static frames_module_v1::state_t* get_region(frames_module_v1::state_t* state, address_t addr)
{
    frames_module_v1::state_t* ret = state;

    while(ret)
    {
        if ((addr >= ret->start) && addr < (ret->start + (ret->n_logical_frames << ret->frame_width)))
            break;
        ret = ret->next;
    }

    return ret;
}

inline address_t frame_address(frames_module_v1::state_t* cur_state, address_t frame_index)
{
    return cur_state->start + (frame_index << cur_state->frame_width);
}

static frames_module_v1::state_t* alloc_any(frame_allocator_v1::closure_t* self, size_t n_physical_frames, uint32_t align, address_t* first_log_frame, size_t* n_log_frames)
{
    frame_allocator_v1::state_t* client_state = self->d_state;
    frames_module_v1::state_t* state = client_state->module_state;
    frames_module_v1::state_t* cur_state = state;

    logger::debug() << __FUNCTION__ << ": requested " << n_physical_frames << " frames.";

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

    return nullptr; // out of physical memory
}

static frames_module_v1::state_t* alloc_any(system_frame_allocator_v1::closure_t* self, size_t n_physical_frames, uint32_t align, address_t* first_log_frame, size_t* n_log_frames)
{
    return alloc_any(reinterpret_cast<frame_allocator_v1::closure_t*>(self), n_physical_frames, align, first_log_frame, n_log_frames);
}

static frames_module_v1::state_t* alloc_range(frame_allocator_v1::closure_t* self, size_t n_physical_frames, address_t start, address_t* first_log_frame, size_t* n_log_frames)
{
    frame_allocator_v1::state_t* client_state = self->d_state;
    frames_module_v1::state_t* state = client_state->module_state;
    frames_module_v1::state_t* cur_state = state;
    uint32_t fshift;

    if ((cur_state = get_region(state, start)) == nullptr)
    {
        logger::warning() << "alloc_range: we don't own requested address " << start;
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
    logger::debug() << "alloc_range: allocated " << bytes << " bytes at requested address " << start << "->" << start + bytes;
    return cur_state;
}

static frames_module_v1::state_t* alloc_range(system_frame_allocator_v1::closure_t* self, size_t n_frames, address_t start, address_t* first_log_frame, size_t* n_log_frames)
{
    return alloc_range(reinterpret_cast<frame_allocator_v1::closure_t*>(self), n_frames, start, first_log_frame, n_log_frames);
}

//======================================================================================================================
// system_frame_allocator_v1 implementation
//======================================================================================================================

static memory_v1::address system_frame_allocator_v1_allocate_range(frame_allocator_v1::closure_t* self, memory_v1::size bytes, uint32_t frame_width, memory_v1::address start, memory_v1::attrs attr)
{
    frame_allocator_v1::state_t* client_state = reinterpret_cast<frame_allocator_v1::state_t*>(self->d_state);
    frames_module_v1::state_t* cur_state;
    address_t first_frame;
    size_t n_frames;

    if (bytes == 0)
    {
        logger::warning() << __FUNCTION__ << ": request to allocate 0 bytes.";
        return NO_ADDRESS;
    }

    frame_width = std::max(frame_width, FRAME_WIDTH);
    size_t n_phys_frames = align_to_frame_width(bytes, frame_width) >> FRAME_WIDTH;

    if (client_state->n_allocated_phys_frames + n_phys_frames > client_state->guaranteed_frames)
    {
        logger::warning() << __FUNCTION__ << ": client exceeded quota!";
        return NO_ADDRESS;
    }

    if (unaligned(start))
    {
        cur_state = alloc_any(self, n_phys_frames, frame_width, &first_frame, &n_frames);
        if (!cur_state)
        {
            logger::warning() << __FUNCTION__ << ": failed to allocate " << bytes << " bytes.";
            return NO_ADDRESS;
        }
    }
    else
    {
        // for alloc_range we check alignment externally
        if (!is_aligned_to_frame_width(start, frame_width))
        {
            logger::warning() << __FUNCTION__ << ": start " << start << " not aligned to width " << frame_width;
            return NO_ADDRESS;
        }
        cur_state = alloc_range(self, n_phys_frames, start, &first_frame, &n_frames);
        if (!cur_state)
        {
            logger::warning() << __FUNCTION__ << ": failed to allocate " << bytes << " bytes at " << start;
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

    logger::debug() << __FUNCTION__ << ": allocated " << start;
    return start;
}

static memory_v1::address system_frame_allocator_v1_allocate(frame_allocator_v1::closure_t* self, memory_v1::size bytes, uint32_t frame_width)
{
    return system_frame_allocator_v1_allocate_range(self, bytes, frame_width, ANY_ADDRESS, memory_v1::attrs_regular);
}

static uint32_t system_frame_allocator_v1_query(frame_allocator_v1::closure_t* self, memory_v1::address addr, memory_v1::attrs* attr)
{
    frame_allocator_v1::state_t* client_state = reinterpret_cast<frame_allocator_v1::state_t*>(self->d_state);
    frames_module_v1::state_t* state = client_state->module_state;

    frames_module_v1::state_t* cur_state = get_region(state, addr);

    if (!cur_state)
    {
        logger::warning() << __FUNCTION__ << ": address " << addr << " is non-existant";
        return 0;
    }

    *attr = cur_state->attrs;
    return cur_state->frame_width;
}

static void system_frame_allocator_v1_free(frame_allocator_v1::closure_t* self, memory_v1::address addr, memory_v1::size bytes)
{
    frame_allocator_v1::state_t* client_state = reinterpret_cast<frame_allocator_v1::state_t*>(self->d_state);
    frames_module_v1::state_t* cur_state = get_region(client_state->module_state, addr);

    if (!cur_state)
    {
        logger::warning() << __FUNCTION__ << ": cannot handle non-existant address " << addr;
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
    ramtab_v1::state mem_state;
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
                logger::warning() << __FUNCTION__ << ": we do not own the frame at " << ((first_frame + i) << FRAME_WIDTH);
                PANIC("Frame allocator misuse.");
            }
            if (frame_width != allocation_frame_width)
            {
                logger::warning() << __FUNCTION__ << ": frame " << (first_frame + i) << " width is " << frame_width << ", should be " << allocation_frame_width;
                PANIC("Frame allocator misuse.");
            }
            if ((mem_state == ramtab_v1::state_mapped) || (mem_state == ramtab_v1::state_nailed))
            {
                logger::warning() << __FUNCTION__ << ": frame at " << ((first_frame + i) << FRAME_WIDTH) << " is " << (mem_state == ramtab_v1::state_mapped ? "mapped" : "nailed");
                PANIC("Frame allocator misuse.");
            }
        }
    }

    // Now get the frame indices in the region (i.e. logical frames of width region_frame_width).
    address_t start_log_frame = bytes_to_log_frames(addr - cur_state->start, region_frame_width);
    address_t end_log_frame = bytes_to_log_frames(end - cur_state->start, region_frame_width);

    if (end_log_frame > cur_state->n_logical_frames)
    {
        logger::warning() << __FUNCTION__ << ": not all addresses are in the same region.";
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
        logger::trace() << "1. Log frame " << i << " free set to " << cur_state->frames[i].free;
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
        logger::trace() << "2. Log frame " << i << " free set to " << cur_state->frames[i].free;
    }

    /* Now update the ramtab (if appropriate) */
    if(cur_state->ramtab)
    {
        uint32_t ridx = cur_state->start >> FRAME_WIDTH;
        size_t fshift = cur_state->frame_width - FRAME_WIDTH; /* frame_width >= FRAME_WIDTH */
        for (size_t j = start_log_frame; j < end_log_frame; ++j)
        {
            //FIXME: indexing will be wrong with fshift > 0
            // Effectively, just set the owner to none and state to unused.
            cur_state->ramtab->put(ridx + (j << fshift), OWNER_NONE, cur_state->frame_width, ramtab_v1::state_unused);
        }
    }

    /* Finally, update number of allocated frames (protect from wrapping), and our linked list of regions */
    size_t new_phys_frames = client_state->n_allocated_phys_frames - n_phys_frames;
    if (new_phys_frames > client_state->n_allocated_phys_frames)
    {
        logger::warning() << __FUNCTION__ << ": freeing more frames than I own (ignored)";
        new_phys_frames = 0;
    }
    client_state->n_allocated_phys_frames = new_phys_frames;

/*  if(!del_range(cst, base, npf, alfw)) {
        eprintf("Frames$Free: something's wrong.\n");
        ntsc_dbgstop();
    }*/
}

static void system_frame_allocator_v1_destroy(frame_allocator_v1::closure_t* self)
{
    PANIC("frames_mod: destroy is not implemented!");
}

static frame_allocator_v1::closure_t* system_frame_allocator_v1_create_client(system_frame_allocator_v1::closure_t* self, memory_v1::address owner_dcb_virt, memory_v1::address owner_dcb_phys, uint32_t granted_frames, uint32_t extra_frames, uint32_t init_alloc_frames)
{
    frame_allocator_v1::state_t* client_state = reinterpret_cast<frame_allocator_v1::state_t*>(self->d_state);
    frames_module_v1::state_t* state = client_state->module_state;

    if (!client_state->heap)
    {
        logger::warning() << __FUNCTION__ << ": called before initialisation is complete.";
        return nullptr;
    }

    if (init_alloc_frames > granted_frames)
        init_alloc_frames = granted_frames;
    if (extra_frames < granted_frames)
        extra_frames = granted_frames;

    // Invariant: extra_frames >= granted_frames >= init_alloc_frames
    logger::debug() << __FUNCTION__ << ": allocating new client state";

    frame_allocator_v1::state_t* new_client_state = reinterpret_cast<frame_allocator_v1::state_t*>(client_state->heap->allocate(sizeof(*new_client_state)));
    if (!new_client_state)
    {
        logger::fatal() << __FUNCTION__ << ": out of memory in frames allocator.";
        PANIC("Out of memory.");
    }

    dcb_ro_t *domain = reinterpret_cast<dcb_ro_t*>(owner_dcb_virt);

    logger::debug() << __FUNCTION__ << ": initialising domain record";

    domain->min_phys_frame_count = 0;
    domain->max_phys_frame_count = state->ramtab->size();
    domain->ramtab = reinterpret_cast<ramtab_entry_t*>(state->ramtab->base());
    domain->memory_region_list.init(&domain->memory_region_list);

    logger::debug() << __FUNCTION__ << ": initialising new client record";
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
    frames_module_v1::state_t* cur_state;

    logger::debug() << __FUNCTION__ << ": allocating " << init_alloc_frames << " init frames";

    cur_state = alloc_any(self, init_alloc_frames, FRAME_WIDTH, &first_frame, &n_frames);
    if (cur_state == nullptr)
    {
        logger::fatal() << __FUNCTION__ << ": Out of physical memory, failed to allocate " << init_alloc_frames << " frames.";
        PANIC("Out of physical memory.");
    }
    if (n_frames != init_alloc_frames)
    {
        PANIC("Region with non-standard frame width! Unsupported.");
    }

    address_t start = frame_address(cur_state, first_frame);
    logger::debug() << __FUNCTION__ << ": allocated " << init_alloc_frames << " physical frames at " << start;

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
    closure_init(&new_client_state->closure, &frame_allocator_v1_methods, new_client_state);

    return &new_client_state->closure;
}

static bool system_frame_allocator_v1_add_frames(system_frame_allocator_v1::closure_t* self, memory_v1::physmem_desc region)
{
    PANIC("Unimplemented!");
    return false;
}

static const system_frame_allocator_v1::ops_t system_frame_allocator_v1_methods =
{
    system_frame_allocator_v1_allocate,
    system_frame_allocator_v1_allocate_range,
    system_frame_allocator_v1_query,
    system_frame_allocator_v1_free,
    system_frame_allocator_v1_destroy,
    system_frame_allocator_v1_create_client,
    system_frame_allocator_v1_add_frames,
};

//======================================================================================================================
// frames_module_v1 implementation
//======================================================================================================================

static memory_v1::size frames_module_v1_required_size(frames_module_v1::closure_t* self)
{
    UNUSED(self);
    size_t n_regions = 0, n_frames = 0, res = 0;
    bootinfo_t* bi = new(bootinfo_t::ADDRESS) bootinfo_t; // simplify memory map operations

    // Scan through the set of mem desc and count the number of logical frames they contain in total.
    std::for_each(bi->mmap_begin(), bi->mmap_end(), [&n_regions, &n_frames](const multiboot_t::mmap_entry_t* e)
    {
        if (e->type() == multiboot_t::mmap_entry_t::non_free)
            return;

        n_frames += e->size() >> FRAME_WIDTH;
        ++n_regions;
    });

    res = sizeof(frame_allocator_v1::closure_t) + sizeof(frame_allocator_v1::state_t) + n_regions * sizeof(frames_module_v1::state_t) + n_frames * sizeof(frame_st);
    res = page_align_up(res);

    logger::debug() << "frames_mod: required_size counted " << int(n_regions) << " memory regions";
    return res;
}

//Don't need where_to_start, bootinfo page has logic for finding an allocatable place. We do not use it currently because MMU mod has to enter some initial mappings - but this could be streamlined.

static system_frame_allocator_v1::closure_t* frames_module_v1_create(frames_module_v1::closure_t* self, ramtab_v1::closure_t* rtab, memory_v1::address where_to_start)
{
    UNUSED(self);

    logger::debug() << "frames_mod create @ " << where_to_start << endl;
    frame_allocator_v1::state_t* client_state = reinterpret_cast<frame_allocator_v1::state_t*>(where_to_start);

    system_frame_allocator_v1::closure_t* ret = reinterpret_cast<system_frame_allocator_v1::closure_t*>(&client_state->closure);
    closure_init(ret, &system_frame_allocator_v1_methods, reinterpret_cast<system_frame_allocator_v1::state_t*>(client_state));

    frames_module_v1::state_t* frames_state = reinterpret_cast<frames_module_v1::state_t*>(where_to_start + sizeof(frame_allocator_v1::state_t));

    client_state->owner = OWNER_SYSTEM;
    client_state->n_allocated_phys_frames = 0;
    client_state->guaranteed_frames = -1;
    client_state->extra_frames = -1;
    client_state->heap = 0;
    client_state->module_state = frames_state;

    frames_module_v1::state_t* running_state = frames_state;
    frames_module_v1::state_t* last_state = running_state;
    size_t n_regions = 0;

    bootinfo_t* bi = new(bootinfo_t::ADDRESS) bootinfo_t;
    std::for_each(bi->mmap_begin(), bi->mmap_end(), [&running_state, &last_state, &n_regions, rtab](const multiboot_t::mmap_entry_t* e)
    {
        if (e->type() == multiboot_t::mmap_entry_t::non_free)
            return;

        running_state->start = e->address();
        running_state->n_logical_frames = phys_frame_number(e->size());
        running_state->frame_width = FRAME_WIDTH;
        if ((e->type() == multiboot_t::mmap_entry_t::free) || (e->type() == multiboot_t::mmap_entry_t::acpi_reclaimable))
        {
            running_state->attrs = memory_v1::attrs_regular;
            running_state->ramtab = rtab;
            logger::debug() << "Adding RAM at " << e->address() << " is " << e->size() << " bytes of type " << e->type();
        }
        else
        {
            running_state->attrs = memory_v1::attrs_non_memory;
            running_state->ramtab = 0;
            logger::debug() << "Adding non-RAM at " << e->address() << " is " << e->size() << " bytes of type " << e->type();
        }
        running_state->frames = reinterpret_cast<frame_st*>(running_state + 1);

        for(size_t j = 0; j < running_state->n_logical_frames; ++j)
            running_state->frames[j].free = running_state->n_logical_frames - j;

        running_state->next = reinterpret_cast<frames_module_v1::state_t*>(&running_state->frames[running_state->n_logical_frames]);
        last_state = running_state;
        running_state = running_state->next;
        ++n_regions;
    });
    last_state->next = 0;

    logger::debug() << "frames_mod: counted " << int(n_regions) << " memory regions again";
    logger::debug() << "frames_mod: and finished at address " << page_align_up(reinterpret_cast<address_t>(&last_state->frames[last_state->n_logical_frames]));

    /*
     * Mark already used frames allocated.
     */
    std::for_each(bi->mmap_begin(), bi->mmap_end(), [ret, client_state](const multiboot_t::mmap_entry_t* e)
    {
        if (e->type() != multiboot_t::mmap_entry_t::non_free)
            return;

        logger::debug() << "Used memory at " << e->address() << " is " << e->size() << " bytes of type " << e->type();

        address_t first_frame;
        size_t n_frames;
        auto running_state = alloc_range(ret, size_in_whole_frames(e->size(), FRAME_WIDTH), e->address(), &first_frame, &n_frames);
        if (n_frames == 0)
            PANIC("Already allocated range deemed unavailable!");

        alloc_update_free_predecessors(running_state, first_frame);
        mark_frames_used(client_state, running_state, first_frame, n_frames);

        client_state->n_allocated_phys_frames += n_frames;
    });

    return ret;
}

static void frames_module_v1_finish_init(frames_module_v1::closure_t* self, system_frame_allocator_v1::closure_t* frames, heap_v1::closure_t* heap)
{
    frame_allocator_v1::state_t* state = reinterpret_cast<frame_allocator_v1::state_t*>(frames->d_state);

    if (state->heap != nullptr)
    {
        logger::warning() << __FUNCTION__ << ": called with heap=" << heap << ", but already have " << state->heap;
        PANIC("Double initialisation of frames module!");
    }

    state->heap = heap;
}

static const frames_module_v1::ops_t frames_module_v1_methods =
{
    frames_module_v1_required_size,
    frames_module_v1_create,
    frames_module_v1_finish_init
};

static frames_module_v1::closure_t clos =
{
    &frames_module_v1_methods,
    nullptr
};

EXPORT_CLOSURE_TO_ROOTDOM(frames_module, v1, clos);
