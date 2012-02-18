#pragma once

#define DECLARE_ANY(_name,_type,_val) \
        types::any _name = { _type##__code, (Type_Val) _type##__wordconv (_val) }

