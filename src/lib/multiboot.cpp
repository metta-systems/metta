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
    uint32_t total = sizeof(multiboot_t::header_t);
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
* @param target The destination for the new copy. Must have enough
*               space to store the copy. The space is calculated
*               via the size() method.
*/
void multiboot_t::copy(address_t target)
{
    // Put strings after the target mbi and after the modules.
//     char* strings = (char *)(L4_Word_t(target) + sizeof(mbi_t) + sizeof(mbi_module_t)*this->modcount);

    // Copy the structure.
//     memcopy(target, this, sizeof(mbi_t));
    // Copy the command line.
//     if (this->cmdline)
//     {
//         target->cmdline = strings;
//         strcpy(target->cmdline, this->cmdline);
//         strings = strings + 1 + strlen(this->cmdline);
        // TODO: align the strings pointer.
//     }

    // Put modules at end of the target mbi.  Assume this will get
    // proper aligment.
//     target->mods = (mbi_module_t *)(L4_Word_t(target) + sizeof(mbi_t));

//     for (L4_Word_t i = 0; i < this->modcount; i++)
//     {
        // Copy the structure.
//         memcopy(&target->mods[i], &this->mods[i], sizeof(mbi_module_t));
        // Copy the command line.
//         if (this->mods[i].cmdline)
        {
//             target->mods[i].cmdline = strings;
//             strcpy(target->mods[i].cmdline, this->mods[i].cmdline);
//             strings = strings + 1 + strlen(this->mods[i].cmdline);
            // TODO: align the strings pointer.
//         }
//     }
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
