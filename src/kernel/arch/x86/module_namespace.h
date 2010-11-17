#pragma once

#include "cstring.h"

// Module namespace represents a namespace for module, a kind of environment.
class module_namespace_t
{
public:
    module_namespace_t(address_t namespace_data, const char* /*prefix*/) : location(namespace_data) {} // a-la set_namespace()
    void set_location(address_t namespace_data) { location = namespace_data; }

    address_t lookup(cstring_t name);

private:
    address_t location;
};

inline
address_t module_namespace_t::lookup(cstring_t /*name*/)
{
    return 0;
}
