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
    /** Construct string from array of unicode (UTF-16) codepoints */
    string(const uint16_t *unicode, int32_t size);
    /** Construct a string by repeatedly copying @p c for @p len times. */
//     string(char c, int len);
    /** Construct a string from specified region of memory. */
//     string(const void * blk, int len);
    /** Assigns @p other to this string and returns a reference to this string. */
    string& operator = (const string& other);
    ~string();

    inline size_t size() const  { return d->length; }
    inline size_t count() const  { return d->length; }
    inline size_t length() const  { return d->length; }
    bool is_empty() const;

    size_t  capacity() const;

    /** Clears the contents of the string and makes it empty. */
    void clear();

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
        int32_t length;
        int32_t allocated;
        uint16_t *data;
        uint16_t array[1];//use 8 (will make a 32 bytes struct with some space for short strings)
    };

    static data shared_null;
    static data shared_empty;
    data* d;

    data* dalloc(size_t size);
    void dfree(data* d);
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
