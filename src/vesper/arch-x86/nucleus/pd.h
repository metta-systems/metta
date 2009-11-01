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

//! Domain
/*!
 * The term domain is used to refer to an executing program and can be thought of as analogous to a UNIX process - i.e.
 * a domain encapsulates the execution state of an application. Each domain has an associated scheduling domain
 * (determining CPU time allocation) and protection domain (determining access rights to regions of the
 * virtual address space).
 */
class domain_t
{
    domain_t();
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
