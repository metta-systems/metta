//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2012, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "types.h"
#include "memutils.h"

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
    typedef uint32_t code_point; // should be uint16_t for utf16, i think

    size_t get_sequence_length(const char data) const;
    code_point get_code_point(const char* data) const;
};

template <class string_type_trait>
class string_t
{
public:
    typedef typename string_type_trait::code_point code_point;

    string_t() : data(0), size(0) {}
    string_t(const char* data) : data(const_cast<char*>(data)), size(~0) {} // calculate size lazily..

    size_t length() const { ++length_ncalls; if (size == ~0UL) size = memutils::string_length(data); return size; }

    bool operator ==(const string_t<string_type_trait>& other) const;
    bool operator ==(const string_t<string_type_trait>::code_point* other) const;

    typename string_type_trait::code_point operator [](int idx) const { return data[idx]; }

    const char* c_str() const { return data; } // meh, bad idea, make something better for console output

private:
    char*  data;
    mutable size_t size;
    size_t cp_length;

    static size_t length_ncalls;
    static size_t operator_eq_ncalls;
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

template<class string_type_trait>
bool string_t<string_type_trait>::operator ==(const string_t<string_type_trait>& other) const
{
    return (length() == other.length()) && memutils::is_memory_equal(data, other.data, length());
}

template<class string_type_trait>
size_t string_t<string_type_trait>::length_ncalls = 0;
template<class string_type_trait>
size_t string_t<string_type_trait>::operator_eq_ncalls = 0;

template<class string_type_trait>
bool string_t<string_type_trait>::operator ==(const string_t<string_type_trait>::code_point* other) const
{
    // FIXME: use string_type_trait::str_length TODO: looks like this is a bottleneck for elf-loader!
    ++operator_eq_ncalls;
    return (length() == memutils::string_length(other)) && memutils::is_memory_equal(data, other, length());
}

typedef string_t<string_ascii_trait> cstring_t;
typedef string_t<string_utf8_trait>  utf8_string_t;
typedef string_t<string_utf16_trait> utf16_string_t;
