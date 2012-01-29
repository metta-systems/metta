#pragma once

#define DECLARE_ANY(_name,_type,_val) \
        Type_Any _name = { _type##__code, (Type_Val) _type##__wordconv (_val) }

