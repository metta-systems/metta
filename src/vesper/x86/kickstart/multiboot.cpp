//
// Copyright 2007 - 2009, Stanislav Karchebnyy <berkus+metta@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "multiboot.h"
#include "memutils.h"

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

multiboot_t::header_t::memmap_t* multiboot_t::memory_map() const
{
    if (!has_mmap_info())
        return 0;
    ASSERT(sizeof(mmapinfo_t)==24);
    return &header->mmap;
}

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
