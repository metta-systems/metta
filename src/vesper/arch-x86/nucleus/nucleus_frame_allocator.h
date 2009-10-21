#pragma once

#include "frame_allocator.h"

/*!
 * @internal
 * This version of frame allocator allocates frames linearly starting from alloc_start onwards.
 * It uses physical addresses and enters them into page directory.
 */
class nucleus_frame_allocator_t : public frame_allocator_t
{
public:
    nucleus_frame_allocator_t(address_t alloc_start);
    virtual void init(multiboot_t::mmap_t* mmap, page_directory_t* pd);
    virtual address_t alloc_frame();
    virtual address_t alloc_frame(address_t virt);
    virtual void free_frame(address_t frame, address_t virt = 0);

private:
    address_t alloc_start;
    page_directory_t* pagedir;
};
