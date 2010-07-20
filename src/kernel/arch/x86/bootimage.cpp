//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2010, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "bootimage.h"
#include "bootimage_private.h"
#include "default_console.h"

/*!
 * Internally bootimage has a tagged format with multiple entries one after another.
 * Each entry has a tag, which specifies type of the entry, it's size and extra information depending on typ.
 */

// <data blob>
// address
// size
// name ofs
// <module>
// address
// size
// name ofs
// upcall record (PCB) location
// dependencies list (ndeps * name ofs entries)

bootimage_t::bootimage_t(const char* name, address_t start, address_t _end)
    : location(start)
    , end(_end)
{
    kconsole << "Bootimage at " << start << " till " << end << " named " << name << endl;
    kconsole << "Bootimage is " << (valid() ? "valid" : "not valid") << endl;
}

bool bootimage_t::valid()
{
    bootimage_header_t* header = reinterpret_cast<bootimage_header_t*>(location);
    return header->magic == FourCC<'B','I','M','G'>::value and header->version == 1;
}

address_t bootimage_t::find_root_domain(size_t* size)
{
    bootimage_info_t info;
    info.generic = reinterpret_cast<char*>(location + sizeof(bootimage_header_t));
    while (info.generic < (char*)end)
    {
        if (info.rec->tag == kind_root_domain)
        {
            if (size)
                *size = info.rootdom->size - sizeof(bootimage_root_domain_t);
            return reinterpret_cast<address_t>(info.generic + sizeof(bootimage_root_domain_t));
        }
        info.generic += info.rec->size;
    }
    return 0;
}

address_t bootimage_t::get_file(uint32_t num)
{
    if (num >= count())
        return 0;

    return 0;//(address_t)start + entries[num].location;
}

const char* bootimage_t::get_file_name(uint32_t num)
{
    if (num >= count())
        return 0;

    return 0;//(const char*)start + entries[num].name_offset;
}

uint32_t bootimage_t::get_file_size(uint32_t num)
{
    if (num >= count())
        return 0;

    return 0;//entries[num].size;
}

uint32_t bootimage_t::count()
{
    return 0;//start->count;
}

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
