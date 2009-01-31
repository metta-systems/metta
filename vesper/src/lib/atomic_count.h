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

    /**
    * Postfix increment operator.
    * Atomically increment counter by one.
    * Returns the old value.
    **/
    inline uint32_t operator ++()
    {
        return atomic_ops::faa(&count, 1);
    }
    /**
    * Prefix increment operator.
    * Atomically increment counter by one.
    * Returns the new value.
    **/
    inline uint32_t operator ++(int)
    {
        return atomic_ops::aaf(&count, 1);
    }
    /**
    * Postfix decrement operator.
    * Atomically decrement counter by one.
    * Returns the old value.
    **/
    inline uint32_t operator --()
    {
        return atomic_ops::fas(&count, 1);
    }
    /**
    * Prefix decrement operator.
    * Atomically decrement counter by one.
    * Returns the new value.
    **/
    inline uint32_t operator --(int)
    {
        return atomic_ops::saf(&count, 1);
    }

    // TODO implement adding/removing an arbitrary count atomically.
    inline uint32_t operator += (int incr)
    {
        return atomic_ops::faa(&count, incr);
    }

private:
    uint32_t count;
};
