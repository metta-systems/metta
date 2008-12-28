//
// Copyright 2007 - 2008, Stanislav Karchebnyy <berkus+metta@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "multiboot.h"
#include "kernel.h"
#include "string.h"
#include "elf.h"
#include "default_console.h" // for print_mmap_info() method below

namespace metta {
namespace kernel {

multiboot::multiboot(multiboot::header *h)
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
            if (!strcmp(c, ".strtab"))
            {
                strtab = sh;
            }
        }
    }
}

void multiboot::print_mmap_info()
{
    if (!has_mmap_info())
        return;
    kconsole.print("MMAP is provided @ %p:\n", header_->mmap_addr);

    ASSERT(sizeof(mmapinfo)==24);

    mmapinfo* mmi = (mmapinfo*)(header_->mmap_addr);
    mmapinfo* end = (mmapinfo*)(header_->mmap_addr + header_->mmap_length);
    while (mmi < end)
    {
        kconsole.print("[entry @ %p, %d bytes]  %llx, %llu bytes (type %u = %s)\n", mmi, mmi->size, mmi->base_addr, mmi->length, mmi->type, mmi->type == 1 ? "Free" : "Occupied");
        mmi = (mmapinfo*)(((char *)mmi) + mmi->size + 4);
    }
}

}
}

// kate: indent-width 4; replace-tabs on;
// vi:set ts=4:set expandtab=on:
