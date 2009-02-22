//
// Copyright 2007 - 2009, Stanislav Karchebnyy <berkus+metta@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "types.h"
#include "atomic_count.h"

namespace metta {
namespace common {

// Allocator class for containers.
// class allocator
// {
// public:
//     virtual void* allocate(size_t size, int alignment = 1) = 0;
//     virtual void deallocate(void* ptr) = 0;
//     virtual ~allocator();
// };

/**
* Implicitly shared copy-on-write UTF-16 string class.
*
* @ingroup containers
**/
class string
{
public:
    /** Construct an empty string with no memory preallocated. */
    string();
    /** Construct a string consisting of a single character @p c. */
//     string(char c);
    /** Construct a string consisting of a single character @p c. */
//     string(unsigned char c);
    /** Construct a string from a C string. */
    string(const char *s);
    /** Copy constructor. */
    string(const string& other);
    /** Construct a string by repeatedly copying @p c for @p len times. */
//     string(char c, int len);
    /** Construct a string from specified region of memory. */
//     string(const void * blk, int len);
    ~string();

    /** Return current size of the string. Synonymous with count(). */
    size_t size() const;
    /** Return current size of the string. Synonymous with size(). */
    size_t count() const;
    /** Return current length of the string. */
    size_t length() const;
    bool is_empty() const;

    size_t  capacity() const;

    string left(int len) const;
    string right(int len) const;
    string mid(int position, int len = -1) const;

    string upcase();
    string downcase();
    string camelcase(bool first_upper = true);

    string repeated(int times);

private:
    struct data
    {
        atomic_count ref;
        uint16_t *data;
        int32_t length;
        int32_t allocated;
    };

    static const data shared_null;
    static const data shared_empty;
    data* d;
};

/**
* Non-modifyable string class for wrapping classic "const char *" strings.
**/
class const_string : public string
{
};

}
}

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
