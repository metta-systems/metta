#include "Multiboot.h"
#include "Kernel.h"
#include "ELF.h"

Multiboot::Multiboot(MultibootHeader *h)
{
	header = h;

	symtab = NULL;
	strtab = NULL;

	// try and find the symtab/strtab
	if (isElf())
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
				if (Kernel::strEquals(c, ".strtab"))
				{
					strtab = sh;
				}
			}
		}
	}
}

Multiboot::~Multiboot()
{
}
