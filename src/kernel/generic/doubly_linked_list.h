#pragma once

// Just a doubly linked list, plain and simple.
template <class _Base>
struct dl_link_t
{
    _Base* next;
    _Base* prev;

    inline void init()
    {
        next = prev = static_cast<_Base*>(this);
    }
    
    inline bool is_empty()
    {
        return next == static_cast<_Base*>(this);
    }
    
    inline void remove()
    {
        next->prev = prev;
        prev->next = next;
    }
    
    /*!
     * Insert another before this.
     */
    inline void insert_before(_Base* another)
    {
        another->next = static_cast<_Base*>(this);
        another->prev = prev;
        prev->next = another;
        prev = another;
    }
    /*!
     * Add another to the tail of the list, which means insert it before us (assuming this is called on list head).
     */
    inline void add_to_tail(_Base* another)
    {
        insert_before(another);
    }
    /*!
     * Insert another after this.
     */
    inline void insert_after(_Base* another)
    {
        another->next = next;
        another->prev = static_cast<_Base*>(this);
        next->prev = another;
        next = another;
    }
    /*!
     * Add another to head of the list, which means insert it after us (assuming this is called on list head).
     */
    inline void add_to_head(_Base* another)
    {
        insert_after(another);
    }
/*    
    inline _Base* dequeue()
    {
        if (is_empty())
            return NULL;
        next->remove();
        return next;
    }
*/
};
