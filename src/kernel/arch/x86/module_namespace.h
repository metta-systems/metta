#pragma once

// Module namespace represents a namespace for module, a kind of environment.
// This parser 
class module_namespace_t
{
    module_namespace_t(address_t namespace_data, const char* prefix); // a-la set_namespace()
    closure* lookup(const char* name);
};
