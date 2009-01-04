//
// Copyright 2007 - 2008, Stanislav Karchebnyy <berkus+metta@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "object.h"
#include "link.h"

/**
* A region defines a contiguous range of virtual address space
* in a task that can be exported to other tasks to map into their
* address space via mappings.
**/
class region : public object
{
private:
    link           task_ref; /** task over which region is defined */
    address_t      start_va; /** start VA of the region */
    size_t         size;     /** size of the region */
    protection_t   prot;     /** protection attributes granted */
};

// kate: indent-width 4; replace-tabs on;
// vi:set ts=4:set expandtab=on:
