#pragma once

// yuck!
#define interface struct
#define method virtual

// base class for component interfaces
interface component_interface_t
{
    method ~component_interface_t() {}
    int interface_version;
};
