//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2010, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "types.h"

/*!
* Use gcc atomic builtins (gcc 4.1+).
*
* See http://lists.boost.org/Archives/boost/2006/11/113380.php for ideas on how to make this a nice template.
* (in the end, use Boost's version of concurrent anyway)
*
* See http://www.gelato.unsw.edu.au/lxr/source/Documentation/atomic_ops.txt
* for better names for the methods and memory barrier constraints.
*
* TODO: error out if not compiling with GCC
* FIXME: I know these methods have silly names.
* FIXME: GCC builtins are implemented only on x86? Check.
*/
class atomic_ops
{
public:
    /*!
    * This is a full memory barrier.
    */
    static inline void membar()
    {
        __sync_synchronize();
    }

    /*!
    * An atomic exchange operation. It writes value into @p *lock, and returns the previous contents of @p *lock.
    * Use carefully, as the only allowed @p new_val could be 1.
    * NB: This is not a full barrier, but rather an acquire barrier.
    */
    static inline address_t tas(address_t *lock, address_t new_val)
    {
        return __sync_lock_test_and_set(lock, new_val);
    }

    /*!
    * Releases the lock acquired by __sync_lock_test_and_set. Normally this means writing the constant 0 to @p *lock.
    * NB: This is not a full barrier, but rather a release barrier.
    */
    static inline void release(address_t *lock)
    {
        __sync_lock_release(lock);
    }

    /*!
    * Atomic compare and swap. If the current value of @c *lock is @c old_val, then write @c new_val into @c *lock.
    * @return the contents of @c *lock before the operation.
    */
    static inline address_t vcas(address_t *lock, address_t old_val, address_t new_val)
    {
        return __sync_val_compare_and_swap(lock, old_val, new_val);
    }

    /*!
    * Atomic compare and swap. If the current value of @c *lock is @c old_val, then write @c new_val into @c *lock.
    * @return true if the comparison is successful and @c new_val was written.
    */
    static inline bool bcas(address_t *lock, address_t old_val, address_t new_val)
    {
        return __sync_bool_compare_and_swap(lock, old_val, new_val);
    }

    // Prefix and postfix increments/decrements implemented as atomic ops.
    // (Since GCC provides them, anyway)

    /*!
    * Fetch and add.
    * @return the value that had previously been in memory.
    */
    static inline address_t faa(address_t *lock, address_t inc)
    {
        return __sync_fetch_and_add(lock, inc);
    }

    /*!
    * Add and then fetch.
    * @return the new value.
    */
    static inline address_t aaf(address_t *lock, address_t inc)
    {
        return __sync_add_and_fetch(lock, inc);
    }

    /*!
    * Fetch and sub.
    * @return the value that had previously been in memory.
    */
    static inline address_t fas(address_t *lock, address_t inc)
    {
        return __sync_fetch_and_sub(lock, inc);
    }

    /*!
    * Sub and then fetch.
    * @return the new value.
    */
    static inline address_t saf(address_t *lock, address_t inc)
    {
        return __sync_sub_and_fetch(lock, inc);
    }
};

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
