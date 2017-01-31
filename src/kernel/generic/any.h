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
#include "console.h"

// Template struct that types::any will reference. @todo this is a special case because of braindead 32 bits compiler/linker misunderstanding. LLVM patch?
struct any {
    uint64_t type_;
    union {
        uint64_t value;
        void*    ptr32value; // this is used to assign from 32-bits ptrs statically.
    };
};

template<class C>
inline any closure_to_any(C* closure, uint64_t type)
{
	any a;
	a.type_ = type;
	a.value = 0;
	a.ptr32value = closure;
	return a;
}

inline console_t&
operator << (console_t& con, any& v)
{
	con << "any { type: " << v.type_ << ", value: " << v.value << "}";
	return con;
}
