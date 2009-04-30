//
// Copyright 2007 - 2009, Stanislav Karchebnyy <berkus+metta@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "string.h"
#include "macros.h"
#include "common.h"
#include "memutils.h"

namespace metta {
namespace common {

string::data string::shared_null = {1, 0, 0, shared_null.array, {0}};
string::data string::shared_empty = {1, 0, 0, shared_empty.array, {0}};

string::data* string::dalloc(size_t size)
{
    return reinterpret_cast<data*>(new char [size]);/*NB use allocator?*/
}

void string::dfree(data* ptr)
{
    delete [] reinterpret_cast<char*>(ptr);
}

string::string() : d(&shared_null)
{
    d->ref++;
}

string::string(const char *s)
{
    UNUSED(s);
//     fromUtf8(s);
}

string::string(const uint16_t *unicode, int32_t size)
{
    if (!unicode) {
        d = &shared_null;
        d->ref++;
    }
    else if (size <= 0) {
        d = &shared_empty;
        d->ref++;
    }
    else {
        d = dalloc(sizeof(data) + size * sizeof(uint16_t));
        d->ref = 1;
        d->length = d->allocated = size;
        d->data = d->array;
        memutils::copy_memory(d->array, unicode, size * sizeof(uint16_t));
        d->array[size] = 0;
    }
}

string::string(const string& other) : d(other.d)
{
    ASSERT(&other != this);
    d->ref++;
}

string& string::operator = (const string& other)
{
    other.d->ref++;
    if (!--d->ref)
        dfree(d);
    d = other.d;
    return *this;
}

string::~string()
{
    if (!--d->ref)
        dfree(d);
}

bool string::is_empty() const
{
    return true;
}

size_t  string::capacity() const
{
    return 0;
}

string string::left(int len) const
{
    UNUSED(len);
    return string();
}

string string::right(int len) const
{
    UNUSED(len);
    return string();
}

string string::mid(int position, int len) const
{
    UNUSED(position);
    UNUSED(len);
    return string();
}

string string::upcase()
{
    return string();
}

string string::downcase()
{
    return string();
}

string string::camelcase(bool first_upper)
{
    UNUSED(first_upper);
    return string();
}

string string::repeated(int times)
{
    UNUSED(times);
    return string();
}

}
}

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
