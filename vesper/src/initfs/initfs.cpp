//
// Copyright 2007 - 2008, Stanislav Karchebnyy <berkus+metta@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "initfs.h"

initfs::initfs(address_t s) : start(s), entries(s + ((header*)s)->index_offset)
{
}

address_t initfs::get_file(string spec)
{
    for (int i = 0; i < start.count; i++)
    {
        if (strncmp())
            return (address_t)start + entries[i].location;
    }
    return 0; // not found
}

// kate: indent-width 4; replace-tabs on;
// vi:set ts=4:set expandtab=on:
