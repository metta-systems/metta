//
// Copyright 2007 - 2009, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "atomic.h"
#include "types.h"

//! A class that implements a spinlock / binary semaphore.
class lockable_t
{
public:
    inline lockable_t() : lock_value(0) {}

    //! Spin until we get the lock.
    inline void lock()
    {
        uint32_t new_val = 1;
        // If we exchange the lock value with 1 and get 1 out, it was locked.
        while (atomic_ops::tas(&lock_value, new_val) == 1)
        {
            // Do nothing. Could notify scheduler here.
        }
        // We got the lock, return.
    }

    //! Spin once.
    inline bool try_lock()
    {
        uint32_t new_val = 1;
        if (atomic_ops::tas(&lock_value, new_val) == 0) // will actually lock!
        {
            return true;
        }
        return false;
    }

    inline bool has_lock()
    {
        return lock_value;
    }

    inline void unlock()
    {
        atomic_ops::release(&lock_value);
    }

private:
    uint32_t lock_value; //!< The actual lock variable.
};

/*!
 * Scoped lock for locking lockable objects.
 */
class lockable_scope_lock_t
{
    lockable_t* lockable;

public:
    lockable_scope_lock_t(lockable_t* obj, bool lock = true) : lockable(obj)
    {
//         ASSERT(lockable);
        if (lock)
            lockable->lock();
    }
    ~lockable_scope_lock_t()
    {
        lockable->unlock();
    }
};

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
