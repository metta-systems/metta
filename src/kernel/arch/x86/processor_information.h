//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2011, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

enum processor_type_e
{
    processor_type_unknown,
    processor_type_486,
    processor_type_pentium,
    processor_type_ppro
};

struct processor_info_t
{
//     cpu_id_t cpu_id;
//     apic_id_t apic_id;
    processor_type_e type;
    uint8_t          vendor[16];
    unsigned int     stepping : 4;
    unsigned int     model : 4;
    unsigned int     family: 4;
    uint32_t         features;
};

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
