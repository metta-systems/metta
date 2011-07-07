//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2011, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "types.h" // include builtin types for generated interfaces

// A mixed influence of OSKit COM and Nemesis component interfaces.

// retain = objc-ish for Ref
// release = objc-ish for Unref

// closure = ops + state
// ops = typed fn ptrs array
// state = opaque pointer for clients, specific pointer for owner/implementor

// generic part
// TODO: use as virtual base mixin class? - easier to just generate full method list from idl hierarchy
// ops_types should inherit from each other, leaving closure wrappers separate.
template <class ops_type, class state_type>
struct module_interface
{
    ops_type* methods;
    state_type* state;
};

template <class C, class O, class S>
void closure_init(C* closure, O* ops, S* state)
{
    closure->methods = ops;
    closure->state = state;
}

/* #define DECLARE_CLOSURE_(name, parent) \
    struct name##_ops; struct name##_state; template <class ops_type = name##_ops, class state_type = name##_state> struct name##_closure : public parent##_closure<name##_ops, name##_state>

#define DECLARE_CLOSURE(name) \
    struct name##_ops; struct name##_state; template <class ops_type = name##_ops, class state_type = name##_state> struct name##_closure : public module_interface<name##_ops, name##_state>*/

#define DECLARE_CLOSURE_(name, parent) \
    struct name##_ops; struct name##_state; struct name##_closure : public parent##_closure<name##_ops, name##_state>

#define DECLARE_CLOSURE(name) \
    struct name##_ops; struct name##_state; struct name##_closure : public module_interface<name##_ops, name##_state>

#define EXPORT_CLOSURE_TO_ROOTDOM(_name, _version, _cl) \
    extern "C" const _name##_##_version##_closure* const exported_##_name##_rootdom = &_cl

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
