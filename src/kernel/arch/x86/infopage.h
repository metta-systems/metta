//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2011, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "time_v1_interface.h"
#include "pervasives_v1_interface.h"
#include "stretch_v1_interface.h"

struct information_page_t
{
    volatile time_v1_ns   now;       /* 00 Current system time              */
    volatile time_v1_ns   alarm;     /* 08 Alarm time                       */
    volatile uint32_t     pcc;       /* 10 Cycle count at last tick         */
    uint32_t              scale;     /* 14 Cycle count scale factor         */
    uint32_t              cycle;     /* 18 Cycle time in picoseconds        */

    pervasives_v1_rec* pervasives;   /* Pervasives pointer for current thread */
    uint64_t scheduler_heartbeat,
             irqs_heartbeat,
             glue_heartbeat,
             faults_heartbeat;

    uint32_t cpu_features;

    void* protection_domains;

    bool mmu_ok;

    stretch_v1_closure** stretch_mapping;
};

#define INFO_PAGE_ADDR 0x1000
#define INFO_PAGE (*((information_page_t*)INFO_PAGE_ADDR))

// Pervasives accessor.
#define PVS(member) (INFO_PAGE.pervasives->member)

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
