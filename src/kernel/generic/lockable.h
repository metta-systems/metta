//
// Part of Metta OS. Check http://atta-metta.net for latest version.
//
// Copyright 2007 - 2012, Stanislav Karchebnyy <berkus@atta-metta.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "atomic.h"
#include "types.h"
#include "default_console.h"//debug

/**
 * A class that implements a spinlock/binary semaphore.
 */
class lockable_t
{
public:
    inline lockable_t() : lock_value(0) {}

    /**
     * Spin until we get the lock.
     */
    inline void lock()
    {
        uint32_t new_val = 1;
        // If we exchange the lock value with 1 and get 1 out, it was locked.
        while (atomic_ops::tas(&lock_value, new_val) == 1)
        {
            // Do nothing. Could notify scheduler here.
            kconsole << "object already locked, spinning!" << endl;
        }
        // We got the lock, return.
    }

    /**
     * Spin once.
     * @return true is lock was obtained, false if lock was not obtained.
     */
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

    /**
     * Atomically release the lock.
     */
    inline void unlock()
    {
        atomic_ops::release(&lock_value);
    }

private:
    uint32_t lock_value; //!< The actual lock variable.
};

/**
 * Scoped lock for locking lockable objects.
 * type_t must implement interface methods lock() and unlock().
 */
template <class type_t>
class scope_lock_t
{
    type_t& lockable;

    scope_lock_t();
    scope_lock_t(const scope_lock_t&);
    scope_lock_t& operator =(const scope_lock_t&);

public:
    scope_lock_t(type_t& obj) : lockable(obj)
    {
        lockable.lock();
    }
    ~scope_lock_t()
    {
        lockable.unlock();
    }
    // Manually release the lock (for Nemesis exception support we need to do some stuff manually...)
    void unlock()
    {
        lockable.unlock();
    }
};

/**
 * Spinlock scoped lock object.
 */
typedef scope_lock_t<lockable_t> lockable_scope_lock_t;
