//
// Copyright 2007 - 2009, Stanislav Karchebnyy <berkus+metta@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "multiboot.h"
#include "kernel.h"
#include "string.h"
#include "elf.h"
#include "memutils.h"

using metta::common::memutils;

namespace metta {
namespace kernel {

multiboot multiboot::instance;

void multiboot::set_header(multiboot::header *h)
{
    header_ = h;

    symtab = NULL;
    strtab = NULL;

    // try and find the symtab/strtab
    if (!is_elf())
        return;

    elf32::section_header* shstrtab = (elf32::section_header*)(header_->addr +
                                        header_->shndx * header_->size);
    // loop through the section headers, try to find the symbol table.
    for(uint32_t i = 0; i < header_->num; i++)
    {
        elf32::section_header* sh = (elf32::section_header*)(header_->addr + i *
                                    header_->size);

        if (sh->sh_type == SHT_SYMTAB)
        {
            symtab = sh;
        }
        else if (sh->sh_type == SHT_STRTAB)
        {
            char *c = (char *)shstrtab->sh_addr + sh->sh_name;
            if (!memutils::strcmp(c, ".strtab"))//FIXME: replace with string() method
            {
                strtab = sh;
            }
        }
    }
}

multiboot::header::memmap* multiboot::memory_map() const
{
    if (!has_mmap_info())
        return 0;
    ASSERT(sizeof(mmapinfo)==24);
    return &header_->mmap;
}

}
}

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
