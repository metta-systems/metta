void* frame_t::operator new(size_t)
{
    return reinterpret_cast<void*>(frame_allocator_t::instance().allocate_frame());
}

void* frame_t::operator new(size_t, address_t /*virt*/)
{
    return reinterpret_cast<void*>(frame_allocator_t::instance().allocate_frame());
//     return reinterpret_cast<void*>(frame_allocator->alloc_frame(virt));
}

void frame_t::operator delete(void*)
{
    // we cannot free memory in kickstart
}

// void* page_table_t::operator new(size_t size, address_t* physical_address)
// {
//     return ::operator new(size, true, physical_address);
// }

