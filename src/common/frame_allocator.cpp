#include "page_directory.h"
#include "frame_allocator.h"

void frame_allocator_t::alloc_frame(page_t* p, bool is_kernel, bool is_writeable)
{
    ASSERT(p);
    ASSERT(!p->frame());

    p->set_frame(alloc_frame());
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
