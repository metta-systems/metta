#include "heap_new.h"
#include "default_console.h"

void* operator new(size_t size, heap_v1_closure* heap) throw()
{
    kconsole<<__PRETTY_FUNCTION__<<" size "<<size<<", heap "<<heap<<endl;
    return reinterpret_cast<void*>(heap->allocate(size));
}

void* operator new[](size_t size, heap_v1_closure* heap) throw()
{
    kconsole<<__PRETTY_FUNCTION__<<" size "<<size<<", heap "<<heap<<endl;
    return reinterpret_cast<void*>(heap->allocate(size));
}

void operator delete(void* p, heap_v1_closure* heap) throw()
{
    kconsole<<__PRETTY_FUNCTION__<<" p "<<p<<", heap "<<heap<<endl;
    heap->free(reinterpret_cast<memory_v1_address>(p));
}

void operator delete[](void* p, heap_v1_closure* heap) throw()
{
    kconsole<<__PRETTY_FUNCTION__<<" p "<<p<<", heap "<<heap<<endl;
    heap->free(reinterpret_cast<memory_v1_address>(p));
}
