//
// Part of Metta OS. Check http://atta-metta.net for latest version.
//
// Copyright 2007 - 2012, Stanislav Karchebnyy <berkus@exquance.com>
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

namespace stretch_allocator_v1 { struct closure_t; }
namespace mmu_v1 { struct closure_t; }

namespace stretch_v1
{

/**
 * State record for a stretch
 * We expose it for the benefit of the MMU code.
 * Would prefer not to, but reckon it's better than exposing SIDs in IDL...
 */
struct state_t : public dl_link_t<state_t>
{
    stretch_allocator_v1::closure_t* allocator;
    mmu_v1::closure_t*               mmu;
    stretch_v1::closure_t            closure;
    sid_t                            sid;

    memory_v1::address               base;
    memory_v1::size                  size;

    stretch_v1::rights               global_rights;

    state_t() : dl_link_t<state_t>() {
        init(this);
    }
};

/** Convenience constant to denote "no rights". */
const uint32_t right_none = 0;

} // namespace stretch_v1

inline console_t& operator << (console_t& con, stretch_v1::rights rights)
{
    con << "["
        << (rights.has(stretch_v1::right_meta)    ? "M" : "-")
        << (rights.has(stretch_v1::right_read)    ? "R" : "-")
        << (rights.has(stretch_v1::right_write)   ? "W" : "-")
        << (rights.has(stretch_v1::right_execute) ? "X" : "-")
        << "]";
    return con;
}
