//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2011, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Architecture-specific segment registers configuration (IA32).
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

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
