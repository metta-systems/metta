//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2012, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "doubly_linked_list.h"

/**
DCB is taken mostly verbatim from Nemesis, here's the original diagram:

 Structure of a Nemesis DCB:

<pre>
---->+------------------------------------------------+	0x0000
     |                                                |
     |  dcb_ro: read-only to the user domain.         |
     |                                                |
     +------------------------------------------------+
     |  VP closure                                    |
     +------------------------------------------------+
     |             padding                            |
     +------------------------------------------------+ 0x2000 (page) XXX
     |                                                |
     |  dcb_rw: read/writeable by user domain         |
     |                                                |
     +------------------------------------------------+
     |                                                |
     |  Array of context slots                        |
     |                                                |
     ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
     ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
     |                                                |
     +------------------------------------------------+
</pre>
*/

struct region_list_t : public dl_link_t<region_list_t>
{
    address_t  start;         /* Start of physical memory region         */
    size_t     n_phys_frames; /* No of *physical* frames it extends      */
    size_t     frame_width;   /* Logical frame width within region       */
};

struct dcb_rw_t;
struct ramtab_entry_t; // defined by mmu_mod

/**
 * Read-only part of domain control block.
 */
struct dcb_ro_t
{
    dcb_rw_t* rw;
    uint32_t min_phys_frame_count;
    uint32_t max_phys_frame_count;
    ramtab_entry_t* ramtab;
    region_list_t memory_region_list;
};

/**
 * Domain-writable part of domain control block.
 */
struct dcb_rw_t
{
    dcb_ro_t* ro;
};

/**
 * Protection domains are implemented as arrays of 4-bit elements, indexed by stretch id.
 */
typedef uint16_t  sid_t;
#define SID_NULL  0xFFFF
#define SID_MAX   16384

