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
#include "nucleus.h"
#include "panic.h"
#include "frame.h"
#include "default_console.h"

// Instead of creating complex allocation machinery for nucleus, we rely on the fact that we do not do any
// allocations before memory manager is initialized.
/*
using nucleus_n::nucleus;

void* frame_t::operator new(size_t)
{
    kconsole << "NUCLEUS: " << __PRETTY_FUNCTION__ << endl;
    return reinterpret_cast<void*>(frame_allocator->alloc_frame());
}

void* frame_t::operator new(size_t, address_t virt)
{
    kconsole << "NUCLEUS: " << __PRETTY_FUNCTION__ << endl;
    return reinterpret_cast<void*>(frame_allocator->alloc_frame(virt));
}

void frame_t::operator delete(void* ptr)
{
    nucleus.mem_mgr().get_current_directory()->remove_mapping((address_t)ptr);
    frame_allocator->free_frame((address_t)ptr); // need a version that removes mapping
}

void* operator new(UNUSED_ARG size_t size, uint32_t place)
{
    return (void *)place;
}

void* operator new(size_t size, bool page_align, address_t* physical_address)
{
    kconsole << "NUCLEUS: " << __PRETTY_FUNCTION__ << endl;
    return nucleus.mem_mgr().allocate(size, page_align, physical_address);
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
    nucleus.mem_mgr().free(p);
}

void operator delete[](void* p)
{
    nucleus.mem_mgr().free(p);
}
*/
// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
