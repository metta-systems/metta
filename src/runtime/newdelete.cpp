//
// Minimal operator new/delete implementation.
//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2010, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "new.h"
#include "macros.h"
#include "memory.h"
#include "frame.h"
#include "frame_allocator.h"
#include "default_console.h"

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

static inline void* placement_alloc(size_t size)
{
    kconsole << " .normal frame alloc. ";
    UNUSED(size);
    return reinterpret_cast<void*>(frame_allocator_t::instance().allocate_frame());
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

void* operator new(size_t size, bool page_align, address_t* addr)
{
    kconsole << RED << "operator new(" << (int)size << ", " << (int)page_align << ", " << (address_t)addr << ")";
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
    kconsole << RED << "malloc(" << size << ") -> ";
    return operator new(size, false, NULL);
}

extern "C" void free(void*)
{
}

// we assume that stl uses realloc only to grow storage
extern "C" void *realloc(void *ptr, size_t size)
{
    void* ptr2 = malloc(size);
    memutils::copy_memory(ptr2, ptr, size); // may fail since new size is assumed to be larger than the old.
    free(ptr);
    return ptr2;
}

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
