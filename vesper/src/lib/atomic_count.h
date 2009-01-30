#pragma once

/**
* Atomic counter for use in e.g. reference counting.
**/
class atomic_count
{
public:
    atomic_count();
    atomic_count(uint32_t init_value);

    /**
    * Postfix increment operator.
    * Atomically increment counter by one.
    * Returns the old value.
    **/
    uint32_t operator ++();
    /**
    * Prefix increment operator.
    * Atomically increment counter by one.
    * Returns the new value.
    **/
    uint32_t operator ++(int);
    /**
    * Postfix decrement operator.
    * Atomically decrement counter by one.
    * Returns the old value.
    **/
    uint32_t operator --();
    /**
    * Prefix decrement operator.
    * Atomically decrement counter by one.
    * Returns the new value.
    **/
    uint32_t operator --(int);
};
