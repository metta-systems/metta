//
// Copyright 2007 - 2009, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Minimal operator new/delete implementation for use during bootstrap.
//
#include "new.h"
#include "macros.h"
#include "memory.h"
#include "frame.h"
#include "linksyms.h"
#include "default_console.h"

bootstrap_frame_allocator& bootstrap_frame_allocator::instance()
{
    static bootstrap_frame_allocator allocator_instance;
    return allocator_instance;
}

extern "C" address_t KICKSTART_BASE;

bootstrap_frame_allocator::bootstrap_frame_allocator()
    : reserved_area_start(LINKSYM(KICKSTART_BASE))
    , allocation_address(0)
{
}

void* bootstrap_frame_allocator::alloc_frame()
{
    allocation_address = page_align_up(allocation_address);
    address_t tmp = allocation_address;
    allocation_address += PAGE_SIZE;
    return reinterpret_cast<void*>(tmp);
}

frame_allocator_t::memory_range_t bootstrap_frame_allocator::reserved_range()
{
    return frame_allocator_t::memory_range_t(reinterpret_cast<void*>(reserved_area_start), 0, allocation_address - reserved_area_start, "reserved during boot");
}

void* frame_t::operator new(size_t)
{
    return bootstrap_frame_allocator::instance().alloc_frame();
}

void* frame_t::operator new(size_t, address_t /*virt*/)
{
    return bootstrap_frame_allocator::instance().alloc_frame();
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

static inline void* placement_alloc(size_t size)
{
    kconsole << " .normal frame alloc. ";
    UNUSED(size);
    return bootstrap_frame_allocator::instance().alloc_frame();
}

// Allocate small non-page-aligned objects from a preallocated page,
// switching to a new page as necessary.
static inline void* small_alloc(size_t size)
{
    kconsole << " .small alloc. ";
    ASSERT(size < PAGE_SIZE/4);
    static address_t alloc_page = 0;
    static size_t alloc_pos = 0;

    if (!alloc_page || (alloc_pos + size > PAGE_SIZE))
    {
        alloc_page = reinterpret_cast<address_t>(placement_alloc(PAGE_SIZE));
        alloc_pos = 0;
    }

    address_t tmp = alloc_page + alloc_pos;
    alloc_pos += (size + 3) & ~3;
    return (void*)tmp;
}

void* operator new(UNUSED_ARG size_t size, uint32_t place)
{
    return (void *)place;
}

void* operator new(size_t size, void* place)
{
    return operator new(size, (uint32_t)place);
}

void* operator new(size_t size, bool page_align, address_t* addr)
{
    kconsole << RED << "operator new(" << size << ", " << (int)page_align << ", " << (address_t)addr << ")";
    void* tmp;

    if (!page_align && (size < PAGE_SIZE/4))
        tmp = small_alloc(size);
    else
        tmp = placement_alloc(size);

    if (addr)
        *addr = (address_t)tmp;

    kconsole << LIGHTRED << "return " << (address_t)tmp << endl;
    return tmp;
}

void* operator new(size_t size)
{
    return operator new(size, false, NULL);
}

void* operator new[](size_t size)
{
    return operator new(size, false, NULL);
}

void* operator new[](size_t size, bool page_align, address_t* addr)
{
    return operator new(size, page_align, addr);
}

void operator delete(void*)
{
}

void operator delete[](void*)
{
}

// Compliant functions for STL allocator (TODO: replace stl malloc_alloc with other default allocator)
extern "C" void* malloc(size_t size)
{
    return operator new(size, false, NULL);
}

extern "C" void free(void*)
{
}

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
