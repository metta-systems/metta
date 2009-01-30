//
// Copyright 2007 - 2008, Stanislav Karchebnyy <berkus+metta@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "atomic.h"
#include "types.h"

/**
* A class that implements a spinlock / binary semaphore.
**/
class lockable
{
public:
    lockable()
    {
        lock_ = 0;
    }

    // Spin until we get the lock.
    void lock()
    {
        uint32_t new_val = 1;
        // If we exchange the lock value with 1 and get 1 out, it was locked.
        while (atomic_ops::exchange(&lock_, new_val) == 1)
        {
            // Do nothing.
        }
        // We got the lock, return.
    }

    bool try_lock()
    {
        // Spin once.
        uint32_t new_val = 1;
        if (atomic_ops::exchange(&lock_, new_val) == 0)
        {
            return true;
        }
        return false;
    }

    bool test_lock()
    {
        return lock_;
    }

    void unlock()
    {
        lock_ = 0;
    }

private:
    uint32_t lock_; /**< The actual lock variable. */
};

// kate: indent-width 4; replace-tabs on;
// vi:set ts=4:set expandtab=on:
