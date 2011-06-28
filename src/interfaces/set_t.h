#pragma once

// Simple wrapper for IDL set class.
template <typename T>
class set_t
{
    uint32_t value; // It has a limit of 32 entries in the set, but we're unlikely to exceed it for any IDL set types.
public:
    set_t(uint32_t v) : value(v) {} // allows implicit conversion
    set_t(const set_t<T>& other) : value(other.value) {}

    // TODO: add asserts for n < 32
    inline uint32_t element(uint32_t n) const                 { return 1 << n; }
    inline uint32_t element(T n) const                        { return 1 << uint32_t(n); }
    inline bool     has(uint32_t id) const                    { return value & element(id); }
    inline bool     has(T id) const                           { return value & element(id); }
    inline void     add(uint32_t id)                          { value |= element(id); }
    inline void     add(T id)                                 { value |= element(id); }
    inline void     remove(uint32_t id)                       { value &= ~element(id); }
    inline void     remove(T id)                              { value &= ~element(id); }
    inline uint32_t intersection(const set_t<T>& other) const { return value & other.value; }
    inline uint32_t join(const set_t<T>& other) const         { return value | other.value; } // C++ took union...
};
