#include "stretch_table_module_v1_interface.h"
#include "stretch_table_module_v1_impl.h"
#include "default_console.h"
#include "heap_new.h"

//======================================================================================================================
// stretch_table_v1 implementation
//======================================================================================================================

#include "stretch_table_v1_interface.h"
#include "stretch_table_v1_impl.h"
#include "stretch_driver_v1_interface.h"
#include "stretch_v1_interface.h"
#include "heap_v1_interface.h"
#include "hash_map"
#include "memory"
#include "functional"

namespace std {
void do_checkpoint(const char* chk)
{
    kconsole << chk << endl;
}
}

class heap_allocator_implementation : public std::allocator_implementation
{
    heap_v1_closure* heap;
public:
    typedef size_t size_type;

    heap_allocator_implementation(heap_v1_closure* h) : heap(h) 
    {
        kconsole << "** heap_allocator_impl: ctor this " << this << ", heap " << h << endl;
    }

    void* allocate(size_type __n, void* = 0)
    {
        kconsole << "** heap_allocator_impl: allocate " << __n << endl;
        return reinterpret_cast<void*>(heap->allocate(__n));
    }
    void deallocate(void* __p)
    { 
        kconsole << "** heap_allocator_impl: deallocate " << __p << endl;
        heap->free(reinterpret_cast<memory_v1_address>(__p));
    }
};

struct hash_fn : std::unary_function<size_t, const stretch_v1_closure*>
{
    size_t operator()(const stretch_v1_closure* s) const
    {
        address_t w = reinterpret_cast<address_t>(s);
        return w ^ (w >> 18);
    }
};

struct equal_fn : std::binary_function<bool, const stretch_v1_closure*, const stretch_v1_closure*>
{
    bool operator()(stretch_v1_closure* a, stretch_v1_closure* b) const
    {
        return a == b;
    }
};

// tuple might be a better choice here?
struct driver_rec
{
    driver_rec(stretch_driver_v1_closure* d, size_t p) : driver(d), page_width(p) {}
    stretch_driver_v1_closure* driver;
    size_t page_width;
};

typedef std::allocator<std::pair<stretch_v1_closure*, driver_rec>> stretch_heap_allocator;
typedef std::hash_map<stretch_v1_closure*, driver_rec, hash_fn, equal_fn, stretch_heap_allocator> stretch_map;

struct stretch_table_v1_state
{
    stretch_map* stretches;
    heap_v1_closure* heap;
};

static bool get(stretch_table_v1_closure* self, stretch_v1_closure* stretch, uint32_t* page_width, stretch_driver_v1_closure** stretch_driver)
{
    stretch_map::iterator it = self->state->stretches->find(stretch);
    if (it != self->state->stretches->end())
    {
        driver_rec result = (*it).second;
        *page_width = result.page_width;
        *stretch_driver = result.driver;
        return true;
    }
    return false;
}

static bool put(stretch_table_v1_closure* self, stretch_v1_closure* stretch, uint32_t page_width, stretch_driver_v1_closure* stretch_driver)
{
    return self->state->stretches->insert(std::make_pair(stretch, driver_rec(stretch_driver, page_width))).second;
}

static bool remove(stretch_table_v1_closure* self, stretch_v1_closure* stretch, uint32_t* page_width, stretch_driver_v1_closure** stretch_driver)
{
    stretch_map::iterator it = self->state->stretches->find(stretch);
    if (it != self->state->stretches->end())
    {
        driver_rec result = (*it).second;
        *page_width = result.page_width;
        *stretch_driver = result.driver;
        self->state->stretches->erase(it);
        return true;
    }
    return false;
}

static void destroy(stretch_table_v1_closure* self)
{
    delete self->state->stretches; //hmmmm, need to use right allocators and stuff..
}

static const stretch_table_v1_ops stretch_table_v1_methods =
{
    get,
    put,
    remove,
    destroy
};

//======================================================================================================================
// stretch_table_module_v1 implementation: TODO rename _module to _factory?
//======================================================================================================================

static stretch_table_v1_closure* create(stretch_table_module_v1_closure* self, heap_v1_closure* heap)
{
    kconsole << "WHOO"<<endl;
    stretch_table_v1_state* new_state = new(heap) stretch_table_v1_state;
    kconsole << "CHOO"<<endl;
    auto heap_alloc = new(heap) heap_allocator_implementation(heap);
    kconsole << "WHEE"<<endl;

    new_state->heap = heap;
    new_state->stretches = new(heap) stretch_map(heap_alloc);
    kconsole << "TIHII" << endl;

    stretch_table_v1_closure* cl = new(heap) stretch_table_v1_closure;
    kconsole << "BUP!" << endl;
    cl->state = new_state;
    cl->methods = &stretch_table_v1_methods;

    return cl;
}

static const stretch_table_module_v1_ops stretch_table_module_v1_methods =
{
    create
};

static const stretch_table_module_v1_closure clos = {
    &stretch_table_module_v1_methods,
    NULL
};

EXPORT_CLOSURE_TO_ROOTDOM(stretch_table_module, v1, clos);
