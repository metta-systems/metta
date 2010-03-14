//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2010, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "types.h"

#define LINKSYM(LS) ((address_t)&LS)

// defined by linker
extern address_t image_end;

extern address_t K_SPACE_START;
extern address_t K_HEAP_START;
extern address_t K_HEAP_END;
extern address_t K_STACK_START;
extern address_t K_STACK_END;

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
