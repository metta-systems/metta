//
// Part of Metta OS. Check http://atta-metta.net for latest version.
//
// Copyright 2007 - 2012, Stanislav Karchebnyy <berkus@atta-metta.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "bootimage.h"
#include "bootimage_private.h"
#include "default_console.h"
#include "memutils.h"
#include "panic.h"
#include "config.h"

using namespace bootimage_n;

void bootimage_t::namespace_t::set(address_t b, void* loc)
{
    ASSERT(loc);
    base = b;
    n_entries = *static_cast<uint32_t*>(loc);
    entries = reinterpret_cast<namespace_entry_t*>(static_cast<char*>(loc)+sizeof(uint32_t));
}

// find an entry in the namespace with key key and return it's int value
bool bootimage_t::namespace_t::get_int(cstring_t key, int& value)
{
    for (size_t i = 0; i < n_entries; ++i)
    {
        if (key == (entries[i].name+base) && entries[i].tag == namespace_entry_t::integer)
        {
            value = entries[i].value_int;
            return true;
        }
    }
    return false;
}

bool bootimage_t::namespace_t::get_string(cstring_t key, cstring_t& value)
{
    for (size_t i = 0; i < n_entries; ++i)
    {
        if (key == (entries[i].name+base) && entries[i].tag == namespace_entry_t::string)
        {
            value = static_cast<const char*>(entries[i].value) + base;
            return true;
        }
    }
    return false;
}

bool bootimage_t::namespace_t::get_symbol(cstring_t key, void*& value)
{
    for (size_t i = 0; i < n_entries; ++i)
    {
        if (key == (entries[i].name+base) && entries[i].tag == namespace_entry_t::symbol)
        {
            value = entries[i].value;
            return true;
        }
    }
    return false;
}

void bootimage_t::namespace_t::dump_all_keys()
{
    kconsole << "Total " << n_entries << " keys in namespace." << endl;
    for (size_t i = 0; i < n_entries; ++i)
    {
        kconsole << "namespace key (at " << uint32_t(entries[i].name) << "): " << (entries[i].name+base) << endl;
    }
}


//======================================================================================================================
// bootimage_t - TODO: use iterators
//======================================================================================================================

/**
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
static void dump_ns(namespace_t* ns, address_t location)
{
    kconsole << "base address : " << location << endl
             << "tag          : " << ns->tag << endl
             << "length       : " << ns->length << endl
             << "address      : " << ns->address << endl
             << "size         : " << ns->size << endl
             << "name         : " << ns->name + location << endl;
}

static void dump_module(module_t* mod, address_t location)
{
    dump_ns(mod, location);
    kconsole << "local ns ofs : " << mod->local_namespace_offset << ", abs addr:" << mod->local_namespace_offset + location << endl;
}

static void dump_rootdom(root_domain_t* dom, address_t location)
{
    dump_module(dom, location);
    kconsole << "entry point  : " << dom->entry_point << endl;
}
#endif

static info_t find_entry(address_t location, address_t end, kind_e kind, const char* name)
{
    ASSERT(sizeof(header_t)==8);

    info_t info;
    info.generic = reinterpret_cast<char*>(location + sizeof(header_t));
    while (info.generic < (char*)end)
    {
#if BOOTIMAGE_DEBUG
        kconsole << "location " << (address_t)info.generic << " moving by " << info.rec->length << endl;
#endif
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
        info.generic += info.rec->length;
    }
    info.generic = 0;
    return info;
}

bootimage_t::modinfo_t bootimage_t::find_root_domain(namespace_t* namesp)
{
    info_t info = find_entry(location, end, kind_root_domain, 0);
    if (!info.generic)
        return modinfo_t(0,0);
    if (namesp)
        namesp->set(location, reinterpret_cast<char*>(location + info.rootdom->local_namespace_offset));

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

// bootimage_t::namespace_t bootimage_t::find_namespace(const char* name)
// {
//     info_t info = find_entry(location, end, kind_module, name);
//     if (!info.generic)
//         info = find_entry(location, end, kind_root_domain, 0);
//     if (!info.generic)
//         return namespace_t(0);
// #if BOOTIMAGE_DEBUG
//     dump_ns(info.module_namespace, location);
// #endif
//     return namespace_t(reinterpret_cast<char*>(location + info.module->local_namespace_offset));
// }
