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

namespace kickstart_n {
    class memory_allocator_t;
}

/*!
* Provide access to boot info page structures.
*
* boot info page layout
* -------------------- START of page
* 4 bytes info page size
* 4 bytes flags field - bit 0 means memmgr pointer is valid.
* x bytes multiboot header
* y bytes multiboot mmap entries
* (free space)
* optional fields:
* 4 bytes initial memmgr pointer - used to reconstruct paging directories.
* -------------------- END of page
*/
class bootinfo_t
{
public:
    bootinfo_t(address_t bootinfo_page) : boot_info(bootinfo_page) { flags() = 0; }
    /*!
    * Get size of the boot info page, stored in the first word of the page.
    * You can typically store more info at the end of the page and then
    * call increase_size() to indicate size of the added block.
    */
    size_t size();
    uint32_t& flags();
    uint32_t optional_fields_size();
    void increase_size(size_t addend);
    void decrease_size(size_t addend);
    multiboot_t::header_t* multiboot_header();
    bool append_mmap_entry(multiboot_t::mmap_entry_t* entry);
    kickstart_n::memory_allocator_t* memmgr();
    bool set_memmgr(kickstart_n::memory_allocator_t*);
private:
    address_t boot_info;
};

inline size_t bootinfo_t::size()
{
    return *reinterpret_cast<size_t*>(boot_info);
}

inline uint32_t& bootinfo_t::flags()
{
    return *reinterpret_cast<uint32_t*>(boot_info + sizeof(size_t));
}

inline uint32_t bootinfo_t::optional_fields_size()
{
    return 0 + (flags() & 0x1 ? sizeof(address_t) : 0);
}

inline void bootinfo_t::increase_size(size_t addend)
{
    *reinterpret_cast<size_t*>(boot_info) += addend;
}

inline void bootinfo_t::decrease_size(size_t addend)
{
    *reinterpret_cast<size_t*>(boot_info) -= addend;
}

inline multiboot_t::header_t* bootinfo_t::multiboot_header()
{
    return reinterpret_cast<multiboot_t::header_t*>(boot_info + sizeof(size_t) + sizeof(uint32_t));
}

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
