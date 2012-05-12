//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2012, Stanislav Karchebnyy <berkus@exquance.com>
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
    enum { ADDRESS = 0x1000 };

    volatile time_v1::ns  now;       /* 00 Current system time              */
    volatile time_v1::ns  alarm;     /* 08 Alarm time                       */
    volatile uint32_t     pcc;       /* 10 Cycle count at last tick         */
    uint32_t              scale;     /* 14 Cycle count scale factor         */
    uint32_t              cycle;     /* 18 Cycle time in picoseconds        */

    pervasives_v1::rec*   pervasives;   /* Pervasives pointer for current thread */
    uint64_t scheduler_heartbeat,
             irqs_heartbeat,
             glue_heartbeat,
             faults_heartbeat;

    uint32_t cpu_features;

    void* protection_domains;

    bool mmu_ok;

    stretch_v1::closure_t** stretch_mapping;
};

#define INFO_PAGE (*((information_page_t*)information_page_t::ADDRESS))

// Pervasives accessor.
#define PVS(member) (INFO_PAGE.pervasives->member)
