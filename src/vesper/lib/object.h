//
// Copyright 2007 - 2009, Stanislav Karchebnyy <berkus+metta@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "types.h"
#include "lockable.h"
#include "security.h"
#include "atomic_count.h"
#include "object_types.h"
#include "string.h"

namespace metta {
namespace kernel {

using metta::common::string;
using object_type::object_type_e;

// thread *target = object_cast<thread>(th);
// template <typename T>
// T *object_cast(object *ptr)
// {
//     return ptr->type() == T::type ? (T*)ptr : 0;
// }

/**
* All kernel entities are objects and have this common fields at their start.
*
* Common system part of kernel objects.
* Provides locking and allocation management.
**/
class object : public lockable
{
public:
    object(object_type_e t = object_type::null) : type_(t) {}

    /**
    * Reference this object from the outside, increasing its reference count
    * and deferring its death.
    **/
    inline void ref() { refs++; }
    /**
    * Unreference this object, when reference count goes down to zero
    * object will rest in peace.
    **/
    inline bool unref() { return --refs; }
    /**
    * Return whether object is alive or merely waiting for its reference count
    * to drop down to zero.
    * Any actions except decreasing reference count are not allowed in an
    * inactive object.
    **/
    inline bool active() { return active_; }

    inline object_type_e type() { return type_; }
    hash_t hash();

private:
    /**
    * Object type.
    **/
    object_type_e     type_;
    /**
    * @c true if this object is currently active;
    * @c false if it has been destroyed and is merely waiting
    *          for all references to it to be dropped.
    **/
    bool              active_;
    /**
    * Count of outstanding references to this object.
    **/
    atomic_count      refs;
    /**
    * Physical address at which the user part of the object is located.
    **/
    address_t         user_pa;
    /**
    * Hash value. Set at object creation time, exported to user via get_state.
    **/
    hash_t            user_hash;
    /**
    * Object security identifier, usually assigned by security server upon creation.
    **/
    security_id_t     sid;
    /**
    * Label for user identification (used in statistic printouts).
    **/
    string            label_;
};

}
}

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
