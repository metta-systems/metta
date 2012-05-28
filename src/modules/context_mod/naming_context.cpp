//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2012, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "naming_context_v1_interface.h"
#include "naming_context_v1_impl.h"
#include "naming_context_factory_v1_interface.h"
#include "naming_context_factory_v1_impl.h"
#include "default_console.h"
#include "module_interface.h"
#include "heap_new.h"
#include "exceptions.h"
#include "panic.h"
#include <zstl/include/ZHashMap.hpp>

// required:
// types.any implementation
// sequence<> meddler support

// steps:
// implement types.any and type_system
// implement naming_context

typedef ZHashMap<const char*, types::any> context_map;
// typedef std::unordered_map<const char*, types::any*> context_map;

struct naming_context_v1::state_t
{
	context_map map;
	heap_v1::closure_t* heap;
	naming_context_v1::closure_t closure;

	template<typename T>
	state_t(ZAllocator<T>* alloc, heap_v1::closure_t* heap_) : map(alloc), heap(heap_) {}
};

static naming_context_v1::names
naming_context_v1_list(naming_context_v1::closure_t* self)
{
	ZList<const char*> keys = self->d_state->map.Keys();

	ZList<const char*>::Iterator it;
	for(it = keys.Begin(); it != keys.End(); it++)
	{
		kconsole << "Returning naming_context key " << (*it) << endl;
	}

	return naming_context_v1::names();
}

// This is incomplete, doesn't support compound arc-names.
static bool
naming_context_v1_get(naming_context_v1::closure_t *self, const char *key, types::any *out_value)
{
	return self->d_state->map.TryGet(key, *out_value);
}

// This is incomplete, doesn't support compound arc-names.
static void
naming_context_v1_add(naming_context_v1::closure_t *self, const char *key, types::any value)
{
	self->d_state->map.Put(key, value);
}

// This is incomplete, doesn't support compound arc-names.
static void
naming_context_v1_remove(naming_context_v1::closure_t *self, const char *key)
{
	self->d_state->map.Remove(key);
}

static void
naming_context_v1_destroy(naming_context_v1::closure_t *)
{

}

static const naming_context_v1::ops_t naming_context_v1_methods =
{
	naming_context_v1_list,
	naming_context_v1_get,
	naming_context_v1_add,
	naming_context_v1_remove,
	naming_context_v1_destroy
};

//=====================================================================================================================
// The Factory
//=====================================================================================================================

template <typename T>
class MyZAllocator : public ZAllocator<T>
{
	heap_v1::closure_t* heap;
public:
	MyZAllocator(heap_v1::closure_t* h) : heap(h) {}
	//Virtual Destructor
	virtual ~MyZAllocator() { }

	/*
	virtual public ZListAllocator<T>::Allocate

	Allocator function which allocates a ZListNode<T>.

	@return - an allocated ZListNode<T>
	*/
	virtual T* Allocate(size_t bytes)
	{
		return reinterpret_cast<T*>(new(heap) char [bytes]);
	}

	/*
	virtual public ZListAllocator<T>::Clone

	Clone function, which is required to allocate and return a new instance of this
	type of allocator.

	@return - allocated instance of this type of allocator
	*/
	virtual ZAllocator<T>* Clone()
	{
		return new(heap) MyZAllocator<T>(heap);
	}

	/*
	virtual public ZListAllocator<T>::Deallocate

	Deallocation function which deallocates a previously allocated ZListNode<T>.

	@param _node - node to deallocate
	*/
	virtual void Deallocate(T* _node)
	{
		delete(heap) reinterpret_cast<char*>(_node);
	}

	/*
	virtual public ZListAllocator<T>::Destroy

	Destroy method.  Called when the allocator is no longer needed by the ZList.
	Heap allocated allocators should delete themselves (suicide).

	@return (void)
	*/
	virtual void Destroy()
	{
		delete(heap) this;
		// Generally I don't know if I'm heap allocated or not, wtf?
	}
};


static naming_context_v1::closure_t*
naming_context_factory_v1_create_context(naming_context_factory_v1::closure_t* self, heap_v1::closure_t* heap, type_system_v1::closure_t* type_system)
{
	kconsole << " ** Creating new naming context." << endl;

	MyZAllocator<const char*>* alloc = new(heap) MyZAllocator<const char*>();
	naming_context_v1::state_t* state = new(heap) naming_context_v1::state_t(alloc, heap);

	closure_init(&state->closure, &naming_context_v1_methods, state);
	return &state->closure;
}

static const naming_context_factory_v1::ops_t naming_context_factory_v1_methods =
{
	naming_context_factory_v1_create_context
};

static const naming_context_factory_v1::closure_t clos =
{
    &naming_context_factory_v1_methods,
    NULL
};

EXPORT_CLOSURE_TO_ROOTDOM(naming_context_factory, v1, clos);
