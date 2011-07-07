//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2011, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "memory_v1_interface.h"
#include "stretch_v1_interface.h"
#include "doubly_linked_list.h"
#include "domain.h"
#include "console.h"

struct stretch_allocator_v1_closure;
struct mmu_v1_closure;

/*!
 * State record for a stretch
 * We expose it for the benefit of the MMU code.
 * Would prefer not to, but reckon it's better than exposing SIDs in IDL...
 */
struct stretch_v1_state : public dl_link_t<stretch_v1_state>
{
    stretch_allocator_v1_closure* allocator;
    mmu_v1_closure*               mmu;
    stretch_v1_closure            closure;
    sid_t sid;
    
    memory_v1_address base;
    memory_v1_size    size;
    
    stretch_v1_rights global_rights;
};

const uint32_t stretch_v1_right_none = 0;

inline console_t& operator << (console_t& con, stretch_v1_rights rights)
{
    con << "["
        << (rights.has(stretch_v1_right_meta)    ? "M" : "-")
        << (rights.has(stretch_v1_right_read)    ? "R" : "-")
        << (rights.has(stretch_v1_right_write)   ? "W" : "-")
        << (rights.has(stretch_v1_right_execute) ? "X" : "-")
        << "]";
    return con;
}

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
