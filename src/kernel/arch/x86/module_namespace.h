#pragma once

// Module namespace represents a namespace for module, a kind of environment.
// This parser 
class module_namespace_t
{
public:
    module_namespace_t(address_t namespace_data, const char* /*prefix*/) : location(namespace_data) {} // a-la set_namespace()
    void set_location(address_t namespace_data) { location = namespace_data; }
//     closure* lookup(const char* name);

private:
    address_t location;
};
