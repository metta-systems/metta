//
// Part of Metta OS. Check http://atta-metta.net for latest version.
//
// Copyright 2007 - 2012, Stanislav Karchebnyy <berkus@atta-metta.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

/*
ops:
 convert _Base to dl_link_t instance
 convert dl_link_t to _Base
 traverse dl_link_t back and forth
 remove direct access?
 work on pointers? this makes c++ification harder (e.g. operators can only be called on instances)

! Interface accepts references as it makes no sense to insert nullptr nodes.
*/

/**
 * This doubly-linked list can be used as a member of a bigger structure.
 * There can be multiple list structures for single object, as a result, each entry contains a pointer
 * to the "base" containing object. next and prev pointers point to dl_link_t.
 */
template <class _Base>
class dl_link_t
{
    dl_link_t<_Base>* next_;
    dl_link_t<_Base>* prev_;
    _Base* base;

public:
    dl_link_t(_Base* b = nullptr)
    {
        init(b);
    }

    inline void init(_Base* b = nullptr)
    {
        base = b;
        next_ = prev_ = this;
    }

    inline bool is_empty()
    {
        return (next_ == this) && (prev_ == this);
    }

    inline dl_link_t<_Base>* next()
    {
        return next_ == this ? nullptr : next_; //@todo consider not returning null anywhere, just to be safer.
    }

    inline dl_link_t<_Base>* prev()
    {
        return prev_ == this ? nullptr : prev_; // the structure will be fully cyclic in that case (at least frames_mod assumes it)
    }

    inline void remove()
    {
        next_->prev_ = prev_;
        prev_->next_ = next_;
    }

    /**
     * Insert another before this.
     */
    inline void insert_before(dl_link_t<_Base>& another)
    {
        another.next_ = this;
        another.prev_ = prev_;
        prev_->next_ = &another;
        prev_ = &another;
    }

    /**
     * Add another to the tail of the list, which means insert it before us (assuming this is called on list head).
     */
    inline void add_to_tail(dl_link_t<_Base>& another)
    {
        insert_before(another);
    }

    /**
     * Insert another after this.
     */
    inline void insert_after(dl_link_t<_Base>& another)
    {
        another.next_ = next_;
        another.prev_ = this;
        next_->prev_ = &another;
        next_ = &another;
    }

    /**
     * Add another to head of the list, which means insert it after us (assuming this is called on list head).
     */
    inline void add_to_head(dl_link_t<_Base>& another)
    {
        insert_after(another);
    }

    inline operator _Base*() { return base; }
    inline _Base* operator ->() { return base; }
/*
    inline _Base* dequeue()
    {
        if (is_empty())
            return nullptr;
        next->remove();
        return next;
    }
*/
};
