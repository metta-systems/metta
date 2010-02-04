//
// Copyright 2010, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Part of Metta OS project. Check http://metta.exquance.com for latest version.
//
#pragma once

#include "config.h"

#if DWARF_DEBUG
#include <stdio.h>
#define DPRINT(...) printf(__VA_ARGS__)
#else
#define DPRINT(...)
#endif
