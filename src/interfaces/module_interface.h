#pragma once

// A mixed influence of OSKit COM and Nemesis component interfaces.

// retain = objc-ish for Ref
// release = objc-ish for Unref

// closure = ops + state
// ops = typed fn ptrs array
// state = opaque pointer for clients, specific pointer for owner/implementor

// generic part
template <class ops_type, class state_type>
struct module_interface
{
    ops_type* methods;
    state_type* state;
};

#define DECLARE_CLOSURE_(name, parent) \
    struct name##_ops; struct name##_state; struct name##_closure : public parent

#define DECLARE_CLOSURE(name) \
    struct name##_ops; struct name##_state; struct name##_closure : public module_interface<name##_ops, name##_state>
