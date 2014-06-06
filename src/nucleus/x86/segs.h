//
// Architecture-specific segment registers configuration (IA32).
//
// Part of Metta OS. Check http://atta-metta.net for latest version.
//
// Copyright 2007 - 2012, Stanislav Karchebnyy <berkus@atta-metta.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#define KERNEL_TS 0x08
#define KERNEL_CS 0x10
#define KERNEL_DS 0x18
#define USER_CS   0x23
#define USER_DS   0x2b
#define PRIV_CS   0x32
#define PRIV_DS   0x3a

#define GDT_ENTRIES 7
