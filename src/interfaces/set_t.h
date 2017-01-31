//
// Part of Metta OS. Check https://atta-metta.net for latest version.
//
// Copyright 2007 - 2017, Stanislav Karchebnyy <berkus@atta-metta.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "types.h"

// Simple wrapper for IDL set class.
// Enum must be an interface enum type.
template <typename Enum>
class set_t
{
    uint32_t value; // It has a limit of 32 entries in the set, but we're unlikely to exceed it for any IDL set types.
public:
    set_t() : value(0)                                              {}
    set_t(uint32_t v) : value(v)                                    {} // allows implicit uint32_t conversion
    set_t(Enum v) : value(0)                                        { add(v); }
    set_t(const set_t<Enum>& other) : value(other.value)            {}
    void operator = (uint32_t v)                                    { value = v; }
    void operator = (Enum v)                                        { value = 0; add(v); }
    operator uint32_t()                                             { return value; }

    // @todo add asserts for n < 32
    inline uint32_t    element(Enum n) const                        { return 1 << uint32_t(n); }
    inline bool        has(Enum id) const                           { return value & element(id); }
    inline set_t<Enum> add(Enum id)                                 { value |= element(id); return *this; }
    inline set_t<Enum> remove(Enum id)                              { value &= ~element(id); return *this; }
    inline uint32_t    intersection(const set_t<Enum>& other) const { return value & other.value; }
    inline uint32_t    join(const set_t<Enum>& other) const         { return value | other.value; } // C++ took union...
    inline void        clear()                                      { value = 0; }
};
