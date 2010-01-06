
void frame_allocator_t::alloc_frame(page_t* p, bool is_kernel, bool is_writeable)
{
    ASSERT(p);
    ASSERT(!p->frame());

    address_t frame = alloc_frame();
    ASSERT(frame); //TODO: handle out of memory more gracefully later.
    p->set_frame(frame);

    p->set_writable(is_writeable);
    p->set_user(!is_kernel);
    p->set_present(true);
}

void frame_allocator_t::free_frame(page_t* p)
{
    ASSERT(p);
    ASSERT(p->frame());

    free_frame(p->frame());
    p->set_frame(0);
    p->set_present(false);
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
    if (virt)
        pagedir->remove_mapping(virt);
    UNUSED(frame);
}

#include "stack_page_allocator.h"
#include "default_console.h"
#include "page_directory.h"
#include "memory/memory_manager.h" // for RPAGETAB_VBASE
#include "memory.h"
#include "panic.h"
#include "debugger.h"
#include "config.h"

address_t stack_frame_allocator_t::alloc_frame(address_t virt)
{
    lock();
    address_t next_frame = next_free_phys;
    pagedir->create_mapping(virt, next_frame);

    next_free_phys = *(address_t*)virt;
    free_frames--;
    ASSERT(free_frames <= total_frames);// catch underflow

    // wipe it clean
    memutils::fill_memory((void*)virt, 0, PAGE_SIZE);

    unlock();

    return next_frame;
}
