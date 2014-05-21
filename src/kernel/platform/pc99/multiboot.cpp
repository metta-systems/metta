//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2012, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "multiboot.h"
#include "elf_parser.h"
#include "memutils.h"
#include "debugger.h"

void multiboot_t::set_header(multiboot_t::header_t* h)
{
    header = h;

    symtab = nullptr;
    strtab = nullptr;

    if (!header)
        return;

    // try and find the symtab/strtab
    if (!is_elf())
        return;

//     elf_parser_t elf(header->addr);
//     symtab = elf.section_header_by_type(SHT_SYMTAB);
//     strtab = elf.section_header(".strtab");

//     elf32::section_header* shstrtab = (elf32::section_header*)(header->addr + header->shndx * header->size);
    // loop through the section headers, try to find the symbol table.
/*    for(uint32_t i = 0; i < header->num; i++)
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
    }*/
}


multiboot_t::mmap_t* multiboot_t::memory_map() const
{
    if (!has_mmap_info())
        return 0;
    ASSERT(sizeof(mmap_entry_t)==24);
    return &header->mmap;
}


void multiboot_t::mmap_t::dump()
{
    debugger_t::dump_memory((address_t)addr, length);
}


multiboot_t::mmap_entry_t* multiboot_t::mmap_t::first_entry()
{
    return addr;
}


multiboot_t::mmap_entry_t* multiboot_t::mmap_t::next_entry(multiboot_t::mmap_entry_t* prev)
{
    if (!prev)
        return 0;

    multiboot_t::mmap_entry_t* end  = reinterpret_cast<multiboot_t::mmap_entry_t*>((char*)addr + length);
    multiboot_t::mmap_entry_t* next = reinterpret_cast<multiboot_t::mmap_entry_t*>((char*)prev + prev->entry_size + 4);

    if (next < end)
        return next;

    return 0;
}
