#include "nucleus_frame_allocator.h"

nucleus_frame_allocator_t::nucleus_frame_allocator_t(address_t alloc_start)
    : alloc_start(page_align_up<address_t>(alloc_start))
    , pagedir(0)
{
}

void kickstart_frame_allocator_t::init(multiboot_t::mmap_t* mmap, page_directory_t* pd)
{
    UNUSED(mmap);
    pagedir = pd;
}

address_t kickstart_frame_allocator_t::alloc_frame()
{
    address_t ret = alloc_start;
    alloc_start += PAGE_SIZE;
    return ret;
}

address_t kickstart_frame_allocator_t::alloc_frame(address_t virt)
{
    address_t ret = alloc_start;
    alloc_start += PAGE_SIZE;
    pagedir->create_mapping(virt, ret);
    return ret;
}

void kickstart_frame_allocator_t::free_frame(address_t frame)
{
    // do nothing
    //TODO: remove mapping!
    UNUSED(frame);
}
