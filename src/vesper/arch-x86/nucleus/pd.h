//
// Copyright 2007 - 2009, Stanislav Karchebnyy <berkus+metta@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "page_directory.h"

namespace nucleus
{

//! Protection Domain
/*!
* Protection domain governs an address space and threads that execute in this space.
*/
class pd_t
{
    pd_t();
private:
    page_directory_t* pagedir;

//     portal_table portals;
//     mapping_t mappings;
//     region_t regions;
//     security_id_t sec_id;
};

} // namespace nucleus

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
