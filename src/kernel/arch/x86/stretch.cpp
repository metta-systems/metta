//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2010, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "protection_domain.h"

//! Allocate a stretch of virtual addresses of size @c size with optional start address @c base.
// Stretch is created in privileged space (thus not editable by application).
stretch_t* stretch_t::create(size_t size, access_t access, address_t base)
{
    stretch_t* stretch = new stretch_t;
    if (protection_domain_t::privileged().allocate_stretch(stretch, size, access, base))
        return stretch;
    delete stretch;
    return 0;
}

stretch_t::stretch_t()
    : address(0)
    , size(0)
    , access_rights(0)
    , stretch_driver(0)
{
}

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
