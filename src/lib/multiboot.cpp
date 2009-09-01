//
// Copyright 2007 - 2009, Stanislav Karchebnyy <berkus+metta@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "multiboot.h"
#include "memutils.h"
#include "debugger.h"

void multiboot_t::set_header(multiboot_t::header_t* h)
{
    header = h;

    symtab = NULL;
    strtab = NULL;

    if (!header)
        return;

    // try and find the symtab/strtab
    if (!is_elf())
        return;

    elf32::section_header* shstrtab = (elf32::section_header*)(header->addr + header->shndx * header->size);
    // loop through the section headers, try to find the symbol table.
    for(uint32_t i = 0; i < header->num; i++)
    {
        elf32::section_header* sh = (elf32::section_header*)(header->addr + i * header->size);

        switch (sh->type)
        {
            case SHT_SYMTAB:
                symtab = sh;
                break;

            case SHT_STRTAB:
                char *c = (char *)shstrtab->addr + sh->name;
                if (memutils::strequal(c, ".strtab"))//FIXME: replace with const_string method
                {
                    strtab = sh;
                }
                break;
        }
    }
}


multiboot_t::mmap_t* multiboot_t::memory_map() const
{
    if (!has_mmap_info())
        return 0;
    ASSERT(sizeof(mmap_entry_t)==24);
    return &header->mmap;
}


/*!
* Returns the total size of the multiboot info. Includes the size of module
* definitions, strings, and any space necessary to maintain aligment.
*
* @returns The total size of the multiboot info.
*/
uint32_t multiboot_t::size()
{
    uint32_t total = sizeof(uint32_t) + sizeof(multiboot_t::header_t);
    uint32_t alignment_space, cmdline_len;

    cmdline_len = 1 + memutils::strlen(header->cmdline);
    alignment_space = sizeof(uint32_t) - cmdline_len % sizeof(uint32_t);
    total += cmdline_len + alignment_space;

    for (uint32_t i = 0; i < module_count(); i++)
    {
        total += sizeof(modinfo_t);
        cmdline_len = 1 + memutils::strlen(module(i)->str);
        alignment_space = sizeof(uint32_t) - cmdline_len % sizeof(uint32_t);
        total += cmdline_len + alignment_space;
    }

    total += memory_map()->size();

    return total;
}


/*!
* Copies the current multiboot info into a new location.
*
* Boot info page gathers some data from the bootloader and passes it unto nucleus for memory regions and
* pagetables initialization. Multiboot info is at the start of boot info page.
*
* Layout of boot info page:
* uint32_t first_free_byte; // address of first free byte within this page (after all infos), size of useful data.
* multiboot_t::header_t mb_header;
* char cmdline[]; // aligned
* multiboot_t::modinfo_t  mod_infos[num_infos];
* char mod_cmdlines[]; // aligned
* mmap_entry_t mmap[];
*
* use elf loader to unpack modules and update markings in modinfos (need to extend modinfos to include mappping info?)
* add fake mmap entry to cover newly allocated blocks and mark them as bootloader_specific memory type
*
* @param target The destination for the new copy. Must have enough
*               space to store the copy. The space is calculated
*               via the size() method.
*/
void multiboot_t::copy(address_t target)
{
    uint32_t* size_ptr = reinterpret_cast<uint32_t*>(target);
    target += sizeof(uint32_t);

    header_t* target_header = reinterpret_cast<header_t*>(target);

    // Copy the multiboot header structure.
    memutils::copy_memory(reinterpret_cast<void*>(target), header, sizeof(header_t));

    target += sizeof(header_t);

    // Copy kickstart command line.
    uint32_t alignment_space, cmdline_len;

    cmdline_len = 1 + memutils::strlen(header->cmdline);
    alignment_space = sizeof(uint32_t) - cmdline_len % sizeof(uint32_t);

    memutils::copy_memory(reinterpret_cast<void*>(target), header->cmdline, cmdline_len);

    // Point to the local copy.
    target_header->cmdline = reinterpret_cast<char*>(target);

    // This is where we store modules info.
    target += cmdline_len + alignment_space;
    // This is where we store modules command lines.
    char* strings = reinterpret_cast<char*>(target + module_count() * sizeof(modinfo_t));

    target_header->modules = reinterpret_cast<modinfo_t*>(target);

    for (uint32_t i = 0; i < module_count(); i++)
    {
        memutils::copy_memory(reinterpret_cast<void*>(target), module(i), sizeof(modinfo_t));
        target += sizeof(modinfo_t);

        if (module(i)->str)
        {
            target_header->modules[i].str = strings;

            cmdline_len = 1 + memutils::strlen(module(i)->str);
            alignment_space = sizeof(uint32_t) - cmdline_len % sizeof(uint32_t);

            memutils::copy_memory(strings, module(i)->str, cmdline_len);

            strings += cmdline_len + alignment_space;
        }
    }

    // This is where we put our memmap.
    target = reinterpret_cast<address_t>(strings);
    // We update header anyway, because we will be adding our fake mmap entry for allocated blocks.
    target_header->mmap.set_addr(target);

    mmap_t* memmap = memory_map();
    if (memmap)
    {
        uint32_t mmap_length = 0;
        multiboot_t::mmap_entry_t* mmi = memmap->first_entry();
        while (mmi)
        {
            memutils::copy_memory(reinterpret_cast<void*>(target), mmi, sizeof(mmap_entry_t));
            reinterpret_cast<mmap_entry_t*>(target)->set_entry_size(sizeof(mmap_entry_t)); //  ignore any extra fields
            mmap_length += sizeof(mmap_entry_t);
            target += sizeof(mmap_entry_t);
            mmi = memmap->next_entry(mmi);
        }
        target_header->mmap.set_length(mmap_length);
    }

    *size_ptr = target - reinterpret_cast<address_t>(size_ptr); // black muti magic
}


uint32_t multiboot_t::mmap_t::size()
{
    return length;
}


void multiboot_t::mmap_t::dump()
{
    debugger_t::dump_memory((address_t)addr, length);
}


multiboot_t::mmap_entry_t* multiboot_t::mmap_t::first_entry()
{
    return reinterpret_cast<multiboot_t::mmap_entry_t*>(addr);
}


multiboot_t::mmap_entry_t* multiboot_t::mmap_t::next_entry(multiboot_t::mmap_entry_t* prev)
{
    if (!prev)
        return 0;

    multiboot_t::mmap_entry_t* end  = reinterpret_cast<multiboot_t::mmap_entry_t*>(addr + length);
    multiboot_t::mmap_entry_t* next = reinterpret_cast<multiboot_t::mmap_entry_t*>((char*)prev + prev->entry_size + 4);

    if (next < end)
        return next;

    return 0;
}

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
