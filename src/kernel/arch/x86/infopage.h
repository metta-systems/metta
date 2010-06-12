#pragma once

struct information_page_t
{
    void* pervasives;
    uint64_t scheduler_heartbeat, irqs_heartbeat, glue_heartbeat, faults_heartbeat;
};

#define INFO_PAGE (*((information_page_t*)0x1000))
