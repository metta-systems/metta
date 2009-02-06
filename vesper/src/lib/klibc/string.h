//
// Copyright 2007 - 2009, Stanislav Karchebnyy <berkus+metta@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "types.h"

#ifdef __cplusplus
#include "bstrwrap.h"
namespace metta {
namespace common {
typedef Bstrlib::CBString string;
}
}
#endif

// for bstring
extern "C" {
int vsnprintf(char *buf, size_t size, const char *fmt, va_list args);
int sscanf(const char * buf, const char * fmt, ...);
}

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
