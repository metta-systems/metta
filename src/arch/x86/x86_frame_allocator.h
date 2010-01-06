#pragma once

#include "frame_allocator.h"
#include "range_list.h"

class x86_frame_allocator_t : public frame_allocator_t, public lockable_t
{
public:
    inline static x86_frame_allocator_t& instance() { return instance_; }

    /*!
     * Build pages free-lists before paging is enabled, to avoid setting up mappings.
     */
    void initialise_before_paging(multiboot_t::mmap_t* mmap) INIT_ONLY;
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

private:
    x86_frame_allocator_t();
//     virtual void unmap_range(memory_range_t* range);

private:
    map_t<protection_domain_t*, memory_resrec_t*> qos_allocations;
    range_list_t<physical_address_t> ram_ranges;

    // Page Stack structures (TODO: move to subclass, like in Pedigree?)
    physical_address_t next_free_phys; //!< Top of the stack, this is where we get new frame from. This is physical address.
    size_t             total_frames;
    size_t             free_frames;
    size_t             reserved_frames;

    static x86_frame_allocator_t instance_;
};
