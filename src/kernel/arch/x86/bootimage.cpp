//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2010, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "initfs.h"
#include "default_console.h"

initfs_t::initfs_t(address_t s)
/*    : start(reinterpret_cast<header_t*>(s))
    , entries(reinterpret_cast<entry_t*>(s + start->index_offset))*/
{
    kconsole << "initfs_t::ctor " << s << endl;
    start = reinterpret_cast<header_t*>(s);

    kconsole << " magic        " << start->magic << endl
             << " version      " << start->version << endl
             << " index_offset " << start->index_offset << endl
             << " names_offset " << start->names_offset << endl
             << " names_size   " << start->names_size << endl
             << " count        " << start->count << endl;

    entries = reinterpret_cast<entry_t*>(s + start->index_offset);
    for (uint32_t k = 0; k < start->count; k++)
    {
        kconsole << "** entry " << k+1 << endl
                 << " * magic " << entries[k].magic << endl
                 << " * name_offset " << entries[k].name_offset << endl
                 << " * location " << entries[k].location << endl
                 << " * size " << entries[k].size << endl;
    }
}

bool initfs_t::valid()
{
    return start->magic == FOURCC_MAGIC('I','i','f','S') and start->version == 2;
}

address_t initfs_t::get_file(uint32_t num)
{
    if (num >= count())
        return 0;

    return (address_t)start + entries[num].location;
}

const char* initfs_t::get_file_name(uint32_t num)
{
    if (num >= count())
        return 0;

    return (const char*)start + entries[num].name_offset;
}

uint32_t initfs_t::get_file_size(uint32_t num)
{
    if (num >= count())
        return 0;

    return entries[num].size;
}

uint32_t initfs_t::count()
{
    return start->count;
}

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
