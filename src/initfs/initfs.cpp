//
// Copyright 2007 - 2009, Stanislav Karchebnyy <berkus+metta@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "initfs.h"

initfs::initfs(address_t s) : start((header*)s), entries((entry*)(s + ((header*)s)->index_offset))
{
}

bool initfs::valid()
{
    return true;
}

/*address_t initfs::get_file(string spec)
{
    for (int i = 0; i < start->count; i++)
    {*/
//         if (strncmp())
//             return (address_t)start + entries[i]->location;
//     }
//     return 0; // not found
// }

address_t initfs::get_file(uint32_t num)
{
    if (num >= count())
        return 0;

    return (address_t)start + entries[num].location;
}

uint32_t initfs::count()
{
    return start->count;
}

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
