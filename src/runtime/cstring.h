//
// Copyright 2007 - 2009, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "types.h"

struct string_ascii_trait
{
    typedef char code_point;

    size_t get_sequence_length(const char data) const;
    code_point get_code_point(const char* data) const;
};

struct string_utf8_trait
{
    typedef uint32_t code_point;

    size_t get_sequence_length(const char data) const;
    code_point get_code_point(const char* data) const;
};

struct string_utf16_trait
{
    typedef uint32_t code_point;

    size_t get_sequence_length(const char data) const;
    code_point get_code_point(const char* data) const;
};

template <class string_type_trait>
class string_t
{
public:
//     typedef string_type_trait::code_point code_point;

    string_t();
    string_t(const char* data);

    size_t length() { return size; }

    bool operator ==(string_t<string_type_trait>& other) const;

private:
    size_t size;
    char*  data;

/*    struct data
    {
        atomic_count ref;
        size_t       length;
        int32_t      allocated;
        code_point*  data;
        uint16_t     array[8];//use 8 (will make a 32 bytes struct with some space for short strings)
    };

    static data shared_null;
    data* d;

    data* dalloc(size_t size);
    void dfree(data* d);*/
};

typedef string_t<string_ascii_trait> cstring_t;
typedef string_t<string_utf8_trait>  utf8_string_t;
typedef string_t<string_utf16_trait> utf16_string_t;
