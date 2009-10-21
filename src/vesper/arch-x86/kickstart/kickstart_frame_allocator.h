#pragma once

#include "frame_allocator.h"
#include "page_directory.h"

/*!
 * @internal
 * This version of frame allocator allocates frames linearly starting from alloc_start onwards.
 * It uses physical addresses and enters them into page directory.
 */
class kickstart_frame_allocator_t : public frame_allocator_t
{
public:
    kickstart_frame_allocator_t();
    void set_start(address_t alloc_start);
    virtual void init(multiboot_t::mmap_t* mmap, page_directory_t* pd);
    virtual address_t alloc_frame();
    virtual address_t alloc_frame(address_t);
    virtual void free_frame(address_t frame, address_t virt = 0);

private:
    address_t alloc_start;
    page_directory_t* pagedir;
};

inline kickstart_frame_allocator_t::kickstart_frame_allocator_t()
    : alloc_start(-1)
    , pagedir(0)
//     : alloc_start(page_align_up<address_t>(alloc_start))
//     , pagedir(0)
{
}

inline void kickstart_frame_allocator_t::set_start(address_t alloc_start)
{
    this->alloc_start = page_align_up<address_t>(alloc_start);
}

inline void kickstart_frame_allocator_t::init(multiboot_t::mmap_t* mmap, page_directory_t* pd)
{
    UNUSED(mmap);
    pagedir = pd;
}

inline address_t kickstart_frame_allocator_t::alloc_frame()
{
    address_t ret = alloc_start;
    alloc_start += PAGE_SIZE;
    return ret;
}

inline address_t kickstart_frame_allocator_t::alloc_frame(address_t virt)
{
    address_t ret = alloc_start;
    alloc_start += PAGE_SIZE;
    pagedir->create_mapping(virt, ret);
    return ret;
}

inline void kickstart_frame_allocator_t::free_frame(address_t frame, address_t virt)
{
    if (virt)
        pagedir->remove_mapping(virt);
    UNUSED(frame);
}
