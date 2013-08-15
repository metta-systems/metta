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
#include "type_system_v1_interface.h"
#include "default_console.h"
#include "module_interface.h"
#include "exceptions.h"
#include "panic.h"
#include <unordered_map>
#include "heap_new.h"
#include "heap_allocator.h"
#include "stringref.h"
#include "stringstuff.h"

#include "elf.h"//temporary for hash test

// required:
// sequence<> meddler support - std::vector<T> for now, but looking into using sequence_t<T> wrapper instead

using namespace std;

// Things aren't exactly as simple as it would seem, STL doesn't have specializations of hash<> and equal_to<> for
// C strings, so their interpretation as keys of an unordered_map is ... pointer based.
// Unsurprisingly, this breaks just about any attempt to find a string key.
// @todo Check the hash specialization for std::string and implement the similar for C string, or even better
// @todo use stringref_t/cstring_t for key_type.

template<>
struct hash<const char*>
{
    size_t operator ()(const char* key) const
    {
        size_t hash = elf32::elf_hash(key);
        logger::trace() << "hash: key '" << key << "', hash " << hash;
        return hash;
    }
};

template<>
class equal_to<const char*>
{
public:
    bool operator ()(const char* left, const char* right) const
    {
        logger::trace() << "equal_to: left '" << left << "' and right '" << right << "'";
        return stringref_t(left) == stringref_t(right);
    }
};

typedef const char* key_type;
typedef types::any value_type;
typedef pair<key_type, value_type> pair_type;
typedef heap_allocator<pair_type> context_allocator;
typedef unordered_map<key_type, value_type, hash<key_type>, equal_to<key_type>, context_allocator> context_map;

struct naming_context_v1::state_t
{
    naming_context_v1::closure_t closure;
    context_map map;
    heap_v1::closure_t* heap;
    type_system_v1::closure_t* typesystem;

    state_t(context_allocator* alloc, heap_v1::closure_t* heap_) : map(*alloc), heap(heap_) {}
};

static naming_context_v1::names
list(naming_context_v1::closure_t* self)
{
    naming_context_v1::names n(context_allocator(self->d_state->heap));
    for (auto x : self->d_state->map)
    {
        n.push_back(x.first);
    }

    return n;
}

/**
 * Look up a name in the context.
 */
static bool
get(naming_context_v1::closure_t *self, const char *key, types::any *out_value)
{
    naming_context_v1::state_t* state = self->d_state;

    stringref_t name_sr(key);
    // Split key into first component and the rest.
    std::pair<stringref_t, stringref_t> refs = name_sr.split('.');

    // @todo: move this allocation to stringref_t guts?
    if (!refs.second.empty())
        key = string_n_copy(refs.first.data(), refs.first.size(), PVS(heap)); // @todo MEMLEAK

    logger::trace() << "naming_context.get: listing existing keys";
    for (auto x : self->d_state->map) // TODO: remove this loop, debug only!
    {
        stringref_t k(x.first);
        logger::trace() << k.data() << ", length " << k.size();
    }

    // D(stringref_t k(key);
    // kconsole << "naming_context.get: finding key " << k.data() << ", length " << k.size();)
    context_map::iterator it = state->map.find(key);

    // D(if (it != state->map.end())
        // kconsole << ", FOUND" << endl;
    // else
        // kconsole << ", NOT FOUND" << endl;)

    // There was only one component and so we can get the value and return.
    if (refs.second.empty())
    {
        if (it != state->map.end())
        {
            *out_value = (*it).second;
            return true;
        }
        return false;
    }
    else
    {
        // At this stage there is another component. Thus we need to check
        // that the types.any actually is a subtype of naming_context, and then
        // recurse.
        // D(kconsole << "..there are subkeys, need to recurse" << endl;
        // kconsole << "First key " << key << ", length " << refs.first.size() << endl;
        // if (!refs.second.empty())
            // kconsole << "Second key " << refs.second.data() << ", length " << refs.second.size() << endl;)

        // Check conformance with the context type 
        if (it != state->map.end())
        {
            types::any result((*it).second);
            if (state->typesystem->is_type(result.type_, naming_context_v1::type_code))
            {
                naming_context_v1::closure_t* nctx = reinterpret_cast<naming_context_v1::closure_t*>(state->typesystem->narrow(result, naming_context_v1::type_code));
                // @todo support either passing stringrefs or extracting cstrings out of stringrefs...
                return nctx->get(refs.second.data(), out_value);
            }
            else
            {
                // Have to check for exceptions presence, since get is caled before exception system is set up.
                if(PVS(exceptions)) {
                    OS_RAISE((exception_support_v1::id)"naming_context_v1.not_context", 0);
                } else {
                    logger::warning() << __FUNCTION__ << ": not a context " << (*it).first;
                    return false;
                }
            }
        }
        // Haven't found this item
        logger::warning() << "naming_context.get: failed to go deeper.";
        return false;
    }
}

/**
 * Bind a name in the context (not necessarily this one!).
 */
static void
add(naming_context_v1::closure_t *self, const char *key, types::any value)
{
    naming_context_v1::state_t* state = self->d_state;

    stringref_t name_sr(key);
    // Split key into first component and the rest.
    std::pair<stringref_t, stringref_t> refs = name_sr.split('.');

    // @todo: move this allocation to stringref_t guts?
    if (!refs.second.empty())
        key = string_n_copy(refs.first.data(), refs.first.size(), PVS(heap)); // @todo MEMLEAK

    // D(kconsole << "naming_context.add: listing existing keys" << endl;
    // for (auto x : self->d_state->map)
    // {
        // stringref_t k(x.first);
        // kconsole << k.data() << ", length " << k.size() << endl;
    // })

    // D(stringref_t k(key);
    // kconsole << "naming_context.add: finding key " << k.data() << ", length " << k.size();)

    context_map::iterator it = state->map.find(key);

    // D(if (it != state->map.end())
        // kconsole << ", FOUND" << endl;
    // else
        // kconsole << ", NOT FOUND" << endl;)

    // There was only one component and so we can add the value and return.
    if (refs.second.empty())
    {
        if (it != state->map.end())
        {
            OS_RAISE((exception_support_v1::id)"naming_context_v1.exists", 0);
        }
        else
        {
            logger::trace() << "adding " << key << "=>" << value;
            state->map.insert(make_pair(key, value));
            return;
        }
    }
    else
    {
        // At this stage there is another component. Thus we need to check
        // that the types.any actually is a subtype of naming_context, and then
        // recurse.
        // D(kconsole << "..there are subkeys, need to recurse" << endl;
        // kconsole << "First key " << key << ", length " << refs.first.size() << endl;
        // if (!refs.second.empty())
            // kconsole << "Second key " << refs.second.data() << ", length " << refs.second.size() << endl;)

        // Check conformance with the context type 
        if (it != state->map.end())
        {
            types::any result((*it).second);
            if (state->typesystem->is_type(result.type_, naming_context_v1::type_code))
            {
                naming_context_v1::closure_t* nctx = reinterpret_cast<naming_context_v1::closure_t*>(state->typesystem->narrow(result, naming_context_v1::type_code));
                // @todo support either passing stringrefs or extracting cstrings out of stringrefs...
                nctx->add(refs.second.data(), value);
                return;
            }
            else
            {
                OS_RAISE((exception_support_v1::id)"naming_context_v1.not_context", 0);
            }
        }
        // Haven't found this item
        OS_RAISE((exception_support_v1::id)"naming_context_v1.not_found", (exception_support_v1::args)key);
    }
}

/**
 * Remove a name from a context (not necessarily this one!).
 */
static void
remove(naming_context_v1::closure_t *self, const char *key)
{
    naming_context_v1::state_t* state = self->d_state;

    stringref_t name_sr(key);
    // Split key into first component and the rest.
    std::pair<stringref_t, stringref_t> refs = name_sr.split('.');

    // @todo: move this allocation to stringref_t guts?
    if (!refs.second.empty())
        key = string_n_copy(refs.first.data(), refs.first.size(), PVS(heap)); // @todo MEMLEAK

    context_map::iterator it = state->map.find(key);
    // There was only one component and so we can add the value and return.
    if (refs.second.empty())
    {
        if (it != state->map.end())
        {
            state->map.erase(it);
        }
        else
        {
            OS_RAISE((exception_support_v1::id)"naming_context_v1.not_found", (exception_support_v1::args)key);
        }
    }
    else
    {
        // At this stage there is another component. Thus we need to check
        // that the types.any actually is a subtype of naming_context, and then
        // recurse.
        
        // Check conformance with the context type 
        if (it != state->map.end())
        {
            types::any result((*it).second);
            if (state->typesystem->is_type(result.type_, naming_context_v1::type_code))
            {
                naming_context_v1::closure_t* nctx = reinterpret_cast<naming_context_v1::closure_t*>(state->typesystem->narrow(result, naming_context_v1::type_code));
                // @todo support either passing stringrefs or extracting cstrings out of stringrefs...
                nctx->remove(refs.second.data());
            }
            else
            {
                OS_RAISE((exception_support_v1::id)"naming_context_v1.not_context", 0);
            }
        }
        // Haven't found this item
        OS_RAISE((exception_support_v1::id)"naming_context_v1.not_found", (exception_support_v1::args)key);
    }
}

static void
destroy(naming_context_v1::closure_t* self)
{
    self->d_state->map.clear();
}

static const naming_context_v1::ops_t naming_context_v1_methods =
{
    list,
    get,
    add,
    remove,
    destroy
};

//=====================================================================================================================
// The Factory
//=====================================================================================================================

static naming_context_v1::closure_t*
create_context(naming_context_factory_v1::closure_t* self, heap_v1::closure_t* heap, type_system_v1::closure_t* type_system)
{
    logger::debug() << " ** Creating new naming context.";

    context_allocator* alloc = new(heap) context_allocator(heap);
    naming_context_v1::state_t* state = new(heap) naming_context_v1::state_t(alloc, heap);
    state->typesystem = type_system;

    logger::debug() << " ** Created new naming context.";

    closure_init(&state->closure, &naming_context_v1_methods, state);
    return &state->closure;
}

static const naming_context_factory_v1::ops_t naming_context_factory_v1_methods =
{
    create_context
};

static naming_context_factory_v1::closure_t clos =
{
    &naming_context_factory_v1_methods,
    NULL
};

EXPORT_CLOSURE_TO_ROOTDOM(naming_context_factory, v1, clos);
BEGIN_MODULE_DEPENDS
MODULE_DEPENDS_ON(heap_mod)
END_MODULE_DEPENDS
