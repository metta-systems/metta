//
// Copyright 2007 - 2009, Stanislav Karchebnyy <berkus+metta@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "atomic.h"

/**
* Atomic counter for use in e.g. reference counting.
**/
class atomic_count
{
public:
    atomic_count() : count(0) {}
    atomic_count(uint32_t init_value) : count(init_value) {}

    atomic_count& operator = (uint32_t value)
    {
        count = value;
        return *this;
    }

    operator uint32_t ()
    {
        return count;
    }

    /**
    * Postfix increment operator.
    * Atomically increment counter by one.
    * Returns if counter became non-zero.
    **/
    inline bool operator ++(int)
    {
        unsigned char ret;
        asm volatile("lock\n"
                     "incl %0\n"
                     "setne %1"
                    : "=m" (count), "=qm" (ret)
                    : "m" (count)
                    : "memory");
        return ret != 0;
    }
    /**
    * Prefix decrement operator.
    * Atomically decrement counter by one.
    * Returns the new value.
    **/
    inline bool operator --()
    {
        unsigned char ret;
        asm volatile("lock\n"
                     "decl %0\n"
                     "setne %1"
                    : "=m" (count), "=qm" (ret)
                    : "m" (count)
                    : "memory");
        return ret != 0;
    }

private:
    volatile uint32_t count;
};

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
