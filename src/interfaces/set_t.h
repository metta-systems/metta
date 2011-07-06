//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2011, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

// Simple wrapper for IDL set class.
// T cannot be uint32_t...
template <typename T>
class set_t
{
    uint32_t value; // It has a limit of 32 entries in the set, but we're unlikely to exceed it for any IDL set types.
public:
    set_t() : value(0) {}
    set_t(uint32_t v) : value(v) {} // allows implicit uint32_t conversion
    set_t(T v) : value(0) { add(v); }
    set_t(const set_t<T>& other) : value(other.value) {}
    void operator =(uint32_t v) { value = v; }
    void operator =(T v) { value = 0; add(v); }
    operator uint32_t() { return value; }

    // TODO: add asserts for n < 32
    inline uint32_t element(uint32_t n) const                 { return 1 << n; }
    inline uint32_t element(T n) const                        { return 1 << uint32_t(n); }
    inline bool     has(uint32_t id) const                    { return value & element(id); }
    inline bool     has(T id) const                           { return value & element(id); }
    inline set_t<T>&add(uint32_t id)                          { value |= element(id); return *this; }
    inline set_t<T>&add(T id)                                 { value |= element(id); return *this; }
    inline set_t<T>&remove(uint32_t id)                       { value &= ~element(id); return *this; }
    inline set_t<T>&remove(T id)                              { value &= ~element(id); return *this; }
    inline uint32_t intersection(const set_t<T>& other) const { return value & other.value; }
    inline uint32_t join(const set_t<T>& other) const         { return value | other.value; } // C++ took union...
    inline void     clear()                                   { value = 0; }
};

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
