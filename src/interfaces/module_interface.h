#pragma once

// A mixed influence of OSKit COM and Nemesis component interfaces.

// retain = objc-ish for Ref
// release = objc-ish for Unref

// closure = ops + state
// ops = typed fn ptrs array
// state = opaque pointer for clients, specific pointer for owner/implementor

// generic part
// TODO: use as virtual base mixin class? - easier to just generate full method list from idl hierarchy
template <class ops_type, class state_type>
struct module_interface
{
    ops_type* methods;
    state_type* state;
};

/* #define DECLARE_CLOSURE_(name, parent) \
    struct name##_ops; struct name##_state; template <class ops_type = name##_ops, class state_type = name##_state> struct name##_closure : public parent##_closure<name##_ops, name##_state>

#define DECLARE_CLOSURE(name) \
    struct name##_ops; struct name##_state; template <class ops_type = name##_ops, class state_type = name##_state> struct name##_closure : public module_interface<name##_ops, name##_state>*/

#define DECLARE_CLOSURE_(name, parent) \
    struct name##_ops; struct name##_state; struct name##_closure : public parent##_closure<name##_ops, name##_state>

#define DECLARE_CLOSURE(name) \
    struct name##_ops; struct name##_state; struct name##_closure : public module_interface<name##_ops, name##_state>

#define EXPORT_CLOSURE_TO_ROOTDOM(_type, _name, _cl) \
    extern "C" const _type##_closure* const exported_##_name##_rootdom = &_cl
