//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2011, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "bootimage.h"
#include "bootimage_private.h"
#include "default_console.h"
#include "memutils.h"

using namespace bootimage_n;

//======================================================================================================================
// bootimage_t - TODO: use iterators
//======================================================================================================================

/*!
 * Internally bootimage has a tagged format with multiple entries one after another.
 * Each entry has a tag, which specifies type of the entry, it's size and extra information depending on type.
 */

bootimage_t::bootimage_t(const char* name, address_t start, address_t _end)
    : location(start)
    , end(_end)
{
    kconsole << "Bootimage at " << start << " till " << end << " named " << name << endl;
    kconsole << "Bootimage is " << (valid() ? "valid" : "not valid") << endl;
}

bool bootimage_t::valid()
{
    header_t* header = reinterpret_cast<header_t*>(location);
    return header->magic == four_cc<'B','I','M','G'>::value and header->version == 1;
}

#if BOOTIMAGE_DEBUG
static void dump_module(module_t* mod, address_t location)
{
    kconsole << "tag          : " << mod->tag << endl
             << "length       : " << mod->length << endl
             << "address      : " << mod->address << endl
             << "size         : " << mod->size << endl
             << "name         : " << mod->name + location << endl
             << "local ns ofs : " << mod->local_namespace_offset << endl;
}

static void dump_rootdom(root_domain_t* dom, address_t location)
{
    dump_module(dom, location);
    kconsole << "entry point  : " << dom->entry_point << endl;
}
#endif

static info_t find_entry(address_t location, address_t end, kind_e kind, const char* name)
{
    info_t info;
    info.generic = reinterpret_cast<char*>(location + sizeof(header_t));
    while (info.generic < (char*)end)
    {
        if (info.rec->tag == kind)
        {
#if BOOTIMAGE_DEBUG
            if (info.rec->tag == kind_module)
            {
                dump_module(info.module, location);
            }
#endif

            if (info.rec->tag == kind_root_domain || memutils::is_string_equal(info.module->name + location, name))
            {
                return info;
            }
        }
#if BOOTIMAGE_DEBUG
        kconsole << "location " << (address_t)info.generic << " moving by " << info.rec->length << endl;
#endif
        info.generic += info.rec->length;
    }
    info.generic = 0;
    return info;
}

bootimage_t::modinfo_t bootimage_t::find_root_domain(module_namespace_t* namesp)
{
    info_t info = find_entry(location, end, kind_root_domain, 0);
    if (!info.generic)
        return modinfo_t(0,0);
    if (namesp)
        namesp->set_location(info.rootdom->local_namespace_offset);

#if BOOTIMAGE_DEBUG
    dump_rootdom(info.rootdom, location);
#endif

    return modinfo_t(location + info.rootdom->address, info.rootdom->size);
}

bootimage_t::modinfo_t bootimage_t::find_module(const char* name)
{
    info_t info = find_entry(location, end, kind_module, name);
    if (!info.generic)
        return modinfo_t(0,0);
    return modinfo_t(location + info.module->address, info.module->size);
}

bootimage_t::modinfo_t bootimage_t::find_namespace(const char* name)
{
    info_t info = find_entry(location, end, kind_module, name);
    if (!info.generic)
        return modinfo_t(0,0);
    info.generic = reinterpret_cast<char*>(location + info.module->local_namespace_offset);
    return modinfo_t(location + info.module_namespace->address, info.module_namespace->size);
}

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
