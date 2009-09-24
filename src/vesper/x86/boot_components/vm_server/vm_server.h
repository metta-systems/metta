//
// Copyright 2007 - 2009, Stanislav Karchebnyy <berkus+metta@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "memory/memory_manager.h"

/*!
* VM server manages available memory
* - by giving applications memory frames upon request
*   - use page_frame_allocator_t interface for this
* - by supporting export of memory regions to other PDs.
*   - use region_t/mapping_t interface for this
*/
class vm_server_t
{
    vm_server_t();
    void init(bootinfo_t bi_page);

private:
    memory_manager_t memory_manager;
};

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
