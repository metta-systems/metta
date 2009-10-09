//
// Copyright 2007 - 2009, Stanislav Karchebnyy <berkus+metta@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "memory/new.h"
#include "macros.h"
#include "nucleus.h"
#include "panic.h"
#include "frame.h"

using nucleus_n::nucleus;

void* frame_t::operator new(size_t, address_t& virt)
{
    address_t phys = nucleus.mem_mgr().page_frame_allocator().alloc_frame();
    nucleus.mem_mgr().get_current_directory()->create_mapping(virt, phys);
    return (void*)phys;
}

void frame_t::operator delete(void* ptr)
{
    nucleus.mem_mgr().get_current_directory()->remove_mapping((address_t)ptr);
    nucleus.mem_mgr().page_frame_allocator().free_frame((address_t)ptr);
}

static inline void* placement_alloc(size_t size)
{
    uint32_t tmp = nucleus.mem_mgr().get_placement_address();
    nucleus.mem_mgr().set_placement_address(tmp+size);
    return (void *)tmp;
}

void* operator new(UNUSED_ARG size_t size, uint32_t place)
{
    return (void *)place;
}

void* operator new(size_t size, bool page_align, address_t* physical_address)
{
    if (nucleus.mem_mgr().is_heap_initialized())
    {
        return nucleus.mem_mgr().allocate(size, page_align, physical_address);
    }
    else
    {
        if (page_align)
            nucleus.mem_mgr().align_placement_address();
        void *tmp = placement_alloc(size);
        if (physical_address)
            *physical_address = (address_t)tmp;
        return tmp;
    }
}

void* operator new(size_t size)
{
    return operator new(size, false, NULL);
}

void* operator new[](size_t size)
{
    return operator new(size, false, NULL);
}

void* operator new[](size_t size, bool page_align, address_t* physical_address)
{
    return operator new(size, page_align, physical_address);
}

void operator delete(void* p)
{
    if (nucleus.mem_mgr().is_heap_initialized())
        nucleus.mem_mgr().free(p);
}

void operator delete[](void* p)
{
    if (nucleus.mem_mgr().is_heap_initialized())
        nucleus.mem_mgr().free(p);
}

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
