#pragma once

#include "time_v1_interface.h"

struct information_page_t
{
    volatile time_v1_ns   now;       /* 00 Current system time              */
    volatile time_v1_ns   alarm;     /* 08 Alarm time                       */
    volatile uint32_t     pcc;       /* 10 Cycle count at last tick         */
    uint32_t              scale;     /* 14 Cycle count scale factor         */
    uint32_t              cycle;     /* 18 Cycle time in picoseconds        */

    void* pervasives;                /* Pervasives pointer for current thread */
    uint64_t scheduler_heartbeat,
             irqs_heartbeat,
             glue_heartbeat,
             faults_heartbeat;

    uint32_t cpu_features;
};

#define INFO_PAGE_ADDR 0x1000
#define INFO_PAGE (*((information_page_t*)INFO_PAGE_ADDR))
