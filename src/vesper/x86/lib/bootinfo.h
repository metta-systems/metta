//
// Copyright 2007 - 2009, Stanislav Karchebnyy <berkus+metta@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "multiboot.h"
#include "memory.h"
#include "memutils.h"

/*!
* Provide access to boot info page structures.
*/
class bootinfo_t
{
public:
    bootinfo_t(address_t bootinfo_page) : boot_info(bootinfo_page) {}
    /*!
    * Get size of the boot info page, stored in the first word of the page.
    * You can typically store more info at the end of the page and then
    * call increase_size() to indicate size of the added block.
    */
    size_t size();
    void increase_size(size_t addend);
    multiboot_t::header_t* multiboot_header();
    bool append_mmap_entry(multiboot_t::mmap_entry_t* entry);
private:
    address_t boot_info;
};

inline size_t bootinfo_t::size()
{
    return *reinterpret_cast<size_t*>(boot_info);
}

inline void bootinfo_t::increase_size(size_t addend)
{
    (*reinterpret_cast<size_t*>(boot_info)) += addend;
}

inline multiboot_t::header_t* bootinfo_t::multiboot_header()
{
    return reinterpret_cast<multiboot_t::header_t*>(boot_info + sizeof(size_t));
}

//TODO: move to bootinfo.cpp
inline bool bootinfo_t::append_mmap_entry(multiboot_t::mmap_entry_t* entry)
{
    size_t entry_size = sizeof(multiboot_t::mmap_entry_t);
    if (size() + entry_size > PAGE_SIZE)
        return false;

    multiboot_t::header_t* header = multiboot_header();
    uint32_t end = boot_info + size();

    memutils::copy_memory(reinterpret_cast<void*>(end), entry, entry_size);
    reinterpret_cast<multiboot_t::mmap_entry_t*>(end)->set_entry_size(entry_size); //  ignore any extra fields
    header->mmap.set_size(header->mmap.size() + entry_size);
    increase_size(entry_size);

    return true;
}

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
