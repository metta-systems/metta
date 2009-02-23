//
// Copyright 2007 - 2009, Stanislav Karchebnyy <berkus+metta@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "config.h"
#include "types.h"
#include "macros.h"
#include "common.h" // for panic_assert() - NB shouldn't depend on this?

namespace metta {
namespace kernel {

class object;

/**
* Link is a shared_ptr - it contains a pointer to an object and keeps track of reference counting.
*
* FIXME: what's the point of Fluke's distinction between NULL (0) target and INVALID (-1) target?
** As seen: INVALID link is made for tracking some double frees on links when debugging?
**/
class link
{
public:
    /**
    * Initialize a link to null (pointing to no object).
    **/
    link() : target(0) {}
    link(const link& o);
    ~link() { destroy(); }

    /**
    * Destroy a link and reinitialize it to null (pointing to no object).
    **/
    void reinit();

    /**
    * Initialize a link to point to an object.
    * The target object must be known to exist, but it need not be locked.
    **/
    void produce(object* target);

    /**
    * Destroy a link, leaving its contents undefined.
    **/
    void destroy();

    /**
    * Given two already-initialized links,
    * change the 'dest' link to point to whatever this link points to.
    */
    void transfer(link* dest);

    /**
    * Try to follow a link and lock the object it points to.
    * If successful, returns a pointer to the now-locked object.
    * If the link is stale (or becomes stale before the object can be locked),
    * this function returns null.
    **/
    object* follow();

    /**
    * Try to follow a link and obtain the hash of the referenced object.
    * If successful, returns the hash.
    * If the link is stale this function returns 0.
    * The object is not locked at any point during this operation.
    **/
    hash_t hash();

    /**
    * Find the target of a link and consume the link itself.
    * If successful, returns a pointer to the now-locked object.
    * If the link was stale, returns null.
    * In either case, the contents of the link is left uninitialized.
    **/
    object* consume();

    /**
    * Compare two links to see if they point to the same thing.
    * Stale links become invalid immediately, so this routine merely compares the targets.
    **/
    bool equals(link* other);

    /**
    * Compare a link against a pointer to an object.
    **/
    bool equals(object* other);

private:
    object* target;
};

#if CONFIG_INLINING
#define LINK_INVALID ((object*)-1)

inline link::link(const link& other)
{
    ASSERT(other.target != LINK_INVALID);

    target = other.target;
    if (target)
        target->ref();
}

inline void link::reinit()
{
    ASSERT(target != LINK_INVALID);
    if (target)
        target->unref();
    target = 0;
}

// add a ctor
inline void link::produce(object* t)
{
    ASSERT(t && t != LINK_INVALID);
    target = t;
    target->ref();
}

inline void link::destroy()
{
    ASSERT(target != LINK_INVALID);

    if (target)
        target->unref();

    target = LINK_INVALID; // FIXME: why debug-only? (for ASSERTs obviously)
}

inline void link::transfer(link* dest)
{
    ASSERT(target != LINK_INVALID);
    ASSERT(dest && dest->target != LINK_INVALID);

    dest->destroy();
    dest->target = target;
    if (target)
        target->ref();
}

inline object* link::follow()
{
    object* ob;

    ASSERT(target != LINK_INVALID);

    ob = target;
    if (ob == 0)
        return 0;

    ASSERT(ob->type() != object_type::null);

    ob->lock();

    if (ob->active())
        return ob;

    /* The link is stale, so clear it and drop the reference.  */
    target = 0;

    ob->unlock();
    ob->unref();

    return 0;
}

inline hash_t link::hash()
{
    object* ob;

    ASSERT(target != LINK_INVALID);

    ob = target;
    if (ob == 0)
        return 0;

    ASSERT(ob->type() != object_type::null);

    if (ob->active())
        return ob->hash();

    /* The link is stale, so clear it and drop the reference.  */
    target = 0;
    ob->unref();

    return 0;
}

inline object* link::consume()
{
    object* ob = follow();

    if (ob)
        ob->unref();

    target = LINK_INVALID; // debug only

    return ob;
}

inline bool link::equals(link* other)
{
    return target == other->target;
}

inline bool link::equals(object* other)
{
    ASSERT(other != 0);
    ASSERT(target != LINK_INVALID);

    return target == other;
}

#undef LINK_INVALID
#endif

}
}

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
