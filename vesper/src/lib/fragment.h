//
// Copyright 2007 - 2008, Stanislav Karchebnyy <berkus+metta@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

namespace metta {
namespace kernel {

/**
* Physical mapping information.
*
* A fragment describes a contiguous range of physical memory (a "page").
* mapped into a particular task mapping.
**/
class fragment
{
private:
    address_t           offset;      /**< offset in mapping */
    size_t              size;        /**< size of range */
    protection_t        prot;        /**< protection attribute */
    queue<T>            mlink;       /**< other frags in mapping */
    oskit_addr_t        start_pa;    /**< physical address */
    mapping*            mapping_;    /**< mapping we are a part of */
    bool                obs_cached;  /**< task has cached objects */
    security_id_t       segment_sid; /**< SID */
};

}
}

// kate: indent-width 4; replace-tabs on;
// vi:set ts=4:set expandtab=on:
