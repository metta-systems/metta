//
// Copyright 2007 - 2008, Stanislav Karchebnyy <berkus+metta@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

namespace metta {
namespace common {

/**
* A simple doubly-linked list.
* Called queue just because OSKit did it.
* And since semantically we use it for
* simple non-prioritised queues it's okay.
*
* TODO: ideally, it should be an std::queue
* adaptor around a simple stl-compatible dllist.
* But we don't do STL yet.
* So for now it's an ugly NIH-ridden patch.
*
* NOT thread-safe, so be sure to use a mutex
* when modifying it from multiple threads.
**/
template <class T>
class queue
{
public:
    queue() { next_ = prev_ = this; }
    queue(T& data) : body(data) { next_ = prev_ = this; }
    ~queue() { /* unlink self from the chain */ prev_->next_ = next_; next_->prev_ = prev_; }

    queue<T>* first() { return *this; }
    queue<T>* next()  { return *next_; }
    queue<T>* last()  { return *prev_; }
    queue<T>* prev()  { return *prev_; }

    bool empty() const { return this == next_; }
    bool is_last(queue<T> *entry) const { return entry == prev_; }

    /** Append a new element to the queue */
    void append(queue<T>* new_elem)
    {
        register queue<T> *previous = prev_;
        if (this == previous) // actually, empty()
        {
            next_ = new_elem;
        }
        else
        {
            previous->next_ = new_elem;
        }
        new_elem.prev_ = previous;
        new_elem.next_ = this;
        prev_ = new_elem;
    }

private:
    queue<T>* next_; /* next element */
    queue<T>* prev_; /* previous element */
    T         body;
};

}
}

// kate: indent-width 4; replace-tabs on;
// vi:set ts=4:set expandtab=on:
