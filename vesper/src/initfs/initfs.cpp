//
// Copyright 2007 - 2008, Stanislav Karchebnyy <berkus+metta@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "initfs.h"

initfs::initfs(address_t s) : start(s)
{
    initfs_header *header = (initfs_header*)start;
    initfs_index *index = (initfs_index*)(start + header->index_offset);
    strtab = (const char*)(start + header->names_offset);
}

address_t initfs::get_file(string spec)
{
}

// kate: indent-width 4; replace-tabs on;
// vi:set ts=4:set expandtab=on:
