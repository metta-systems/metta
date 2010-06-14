#pragma once

struct information_page_t
{
    void* pervasives;
    uint64_t scheduler_heartbeat, irqs_heartbeat, glue_heartbeat, faults_heartbeat;
};

#define INFO_PAGE_ADDR 0x1000
#define INFO_PAGE (*((information_page_t*)INFO_PAGE_ADDR))
