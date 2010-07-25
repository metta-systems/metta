//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2010, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "lockable.h"
#include "frame_allocator.h"
#include "range_list.h"
#include "multiboot.h"

class x86_frame_allocator_t : public frame_allocator_t, public lockable_t /* TODO: use per-CPU locks! */
{
public:
    static x86_frame_allocator_t& instance();

    /*!
     * Build pages free-lists before paging is enabled, to avoid setting up mappings.
     */
    void initialise_before_paging(multiboot_t::mmap_t* mmap, memory_range_t reserved_boot_range) INIT_ONLY;
    /*!
     * Unmap and free init memory pages.
     */
    void initialisation_complete();

    /* Allocation interface. */
    virtual physical_address_t allocate_frame();
    virtual void free_frame(physical_address_t frame);
    virtual bool allocate_range(memory_range_t& range,
                                size_t num_frames,
                                flags_t page_constraints,
                                flags_t flags,
                                physical_address_t start = -1);
    virtual bool free_range(memory_range_t& range);

    // Helpers for bootstrap initialisation.
    inline static void set_allocation_start(address_t start) { allocation_address = start; }
    /*frame_allocator_t::*/memory_range_t reserved_range();

private:
    x86_frame_allocator_t();

    class page_stack_t
    {
    public:
        page_stack_t()
            : next_free_phys(0)
            , total_frames(0)
            , free_frames(0)
            , reserved_frames(0)
//             , ranges()
        {}

        physical_address_t next_free_phys; //!< Top of the stack, this is where we get new frame from. This is physical address.
        size_t             total_frames;
        size_t             free_frames;
        size_t             reserved_frames;
        range_list_t<physical_address_t> ranges;
    };

    page_stack_t& stack(physical_address_t address, size_t size);
    page_stack_t& first_usable_stack();

    static const uint32_t below_1mb  = 0;
    static const uint32_t below_16mb = 1;
    static const uint32_t above_16mb = 2;
    static const uint32_t num_stacks = 3;

    page_stack_t       page_stacks[num_stacks];
    bool               stack_initialised;
    address_t          reserved_area_start;

    static x86_frame_allocator_t allocator_instance;
    static physical_address_t    allocation_address;
};

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
