//
// Copyright 2007 - 2009, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Minimal operator new/delete implementation for use during bootstrap.
//
#include "c++boot.h"
#include "macros.h"
#include "memory.h"
#include "frame.h"
#include "page_directory.h"
#include "default_console.h"
#include "kickstart_frame_allocator.h"

extern kickstart_frame_allocator_t frame_allocator; //FIXME: doh

void* frame_t::operator new(size_t)
{
    return reinterpret_cast<void*>(frame_allocator->alloc_frame());
}

void* frame_t::operator new(size_t, address_t virt)
{
    return reinterpret_cast<void*>(frame_allocator->alloc_frame(virt));
}

void frame_t::operator delete(void*)
{
    // we cannot free memory in kickstart
}

void* page_table_t::operator new(size_t size, address_t* physical_address)
{
    return ::operator new(size, true, physical_address);
}

static inline void* placement_alloc(size_t size)
{
    kconsole << " .normal frame alloc. ";
    UNUSED(size);
    return reinterpret_cast<void*>(frame_allocator.alloc_frame());
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
        alloc_page = frame_allocator.alloc_frame();
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

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
