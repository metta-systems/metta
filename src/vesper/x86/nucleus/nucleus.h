#pragma once

#include "memory/memory_manager.h"
#include "bootinfo.h"

namespace nucleus_n
{

/*!
Minimal supervisor-mode nucleus, responsible for little more than context switches.

- Most functionality is provided by servers that execute in user mode without special privileges.
- The kernel is responsible only for switching between protection domains.
- If something can be run at user level, it is.
*/
class nucleus_t
{
public:
    nucleus_t();
    void init(bootinfo_t bi_page);

    memory_manager_t& mem_mgr() { return memory_manager; }
//     page_directory_t& root_pagedir() { return pagedir; }

    void enter_trap(int portal_no); // called from assembler glue code to process client trap and call corresponding portal
    void create_pd(); // portal to create new address space and assign it to a pd
    void destroy_pd();
private:
    memory_manager_t memory_manager;
//     vector_base<pd_t> spaces;
//    page_directory_t pagedir; //!< Kernel page directory.
};

extern nucleus_t nucleus;

} // namespace nucleus
