//
// Part of Metta OS. Check https://atta-metta.net for latest version.
//
// Copyright 2007 - 2017, Stanislav Karchebnyy <berkus@atta-metta.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "heap_new.h"
#include "default_console.h"
#include "logger.h"

void* operator new(size_t size, heap_v1::closure_t* heap) throw()
{
    logger::trace() << __PRETTY_FUNCTION__ << " size " << size << ", heap " << heap;
    return reinterpret_cast<void*>(heap->allocate(size));
}

void* operator new[](size_t size, heap_v1::closure_t* heap) throw()
{
    logger::trace() << __PRETTY_FUNCTION__ << " size " << size << ", heap " << heap;
    return reinterpret_cast<void*>(heap->allocate(size));
}

void operator delete(void* p, heap_v1::closure_t* heap) throw()
{
    logger::trace() << __PRETTY_FUNCTION__ << " p " << p << ", heap " << heap;
    heap->free(reinterpret_cast<memory_v1::address>(p));
}

void operator delete[](void* p, heap_v1::closure_t* heap) throw()
{
    logger::trace() << __PRETTY_FUNCTION__ << " p " << p << ", heap " << heap;
    heap->free(reinterpret_cast<memory_v1::address>(p));
}
