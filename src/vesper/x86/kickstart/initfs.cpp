//
// Copyright 2007 - 2009, Stanislav Karchebnyy <berkus+metta@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "initfs.h"

initfs_t::initfs_t(address_t s)
    : start((header_t*)s)
    , entries((entry_t*)(s + ((header_t*)s)->index_offset))
{
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
