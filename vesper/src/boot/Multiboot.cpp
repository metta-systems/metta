//
// Copyright 2007 - 2008, Stanislav Karchebnyy <berkus+metta@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "Multiboot.h"
#include "Kernel.h"
#include "ELF.h"

multiboot_t::multiboot_t(multiboot_header_t *h)
{
	header = h;

	symtab = NULL;
	strtab = NULL;

	// try and find the symtab/strtab
	if (is_elf())
	{
		Elf32SectionHeader *shstrtab = (Elf32SectionHeader *)(header->addr + header->shndx * header->size);
		// loop through the section headers, try to find the symbol table.
		for(uint32_t i = 0; i < header->num; i++)
		{
			Elf32SectionHeader *sh = (Elf32SectionHeader *)(header->addr + i * header->size);

			if (sh->sh_type == SHT_SYMTAB)
			{
				symtab = sh;
			}
			else if (sh->sh_type == SHT_STRTAB)
			{
				char *c = (char *)shstrtab->sh_addr + sh->sh_name;
				if (kernel::str_equals(c, ".strtab"))
				{
					strtab = sh;
				}
			}
		}
	}
}

// kate: indent-width 4; replace-tabs on;
// vi:set ts=4:set expandtab=on:
