#include "heap_new.h"

void* operator new(size_t size, heap_v1_closure* heap)
{
    return reinterpret_cast<void*>(heap->allocate(size));
}

void* operator new[](size_t size, heap_v1_closure* heap)
{
    return reinterpret_cast<void*>(heap->allocate(size));
}

void operator delete(void* p, heap_v1_closure* heap) throw()
{
    heap->free(reinterpret_cast<memory_v1_address>(p));
}

void operator delete[](void* p, heap_v1_closure* heap) throw()
{
    heap->free(reinterpret_cast<memory_v1_address>(p));
}
