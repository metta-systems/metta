//
// Part of Metta OS. Check http://atta-metta.net for latest version.
//
// Copyright 2007 - 2012, Stanislav Karchebnyy <berkus@atta-metta.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

// This file is included by all generated interfaces.
// Include built-in types, meddler-generated code uses them.
#include "types.h"
// Export macros below need type any.
#include "any.h"

// A mixed influence of OSKit COM and Nemesis component interfaces.

// retain = objc-ish for Ref
// release = objc-ish for Unref

// closure = ops + state
// ops = typed fn ptrs array
// state = opaque pointer for clients, specific pointer for owner/implementor

// Initialise closure pointer members, simple wrapper for greppability.
template <class C, class O, class S>
void closure_init(C* closure, O* ops, S* state)
{
    closure->d_methods = ops;
    closure->d_state = state;
}

// Make given closure available to the root domain during bootup.
#define EXPORT_CLOSURE_TO_ROOTDOM(_name, _version, _cl) \
extern "C" const _name##_##_version::closure_t* const exported_##_name##_rootdom = &_cl; \
extern "C" const any exported_##_name##_any = { _name##_##_version::type_code, { .ptr32value = &_cl } }
