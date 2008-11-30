//
// Copyright 2007 - 2008, Stanislav Karchebnyy <berkus+metta@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "Kernel.h"
#include "ElfParser.h"
#include "DefaultConsole.h"

namespace metta {
namespace kernel {

elf_parser::elf_parser()
{
	header              = NULL;
	section_headers     = NULL;
	symbol_table        = NULL;
	string_table        = NULL;
	got_table           = NULL;
	filename            = NULL;
}

elf_parser::~elf_parser()
{
	delete header;
	delete section_headers;
	delete [] filename;
}

void elf_parser::load_kernel(elf32_section_header* symtab,
                             elf32_section_header* strtab)
{
	symbol_table = symtab;
	string_table = strtab;
}

char* elf_parser::find_symbol(address_t addr, address_t *symbol_start)
{
	address_t max = 0;
	elf32_symbol *fallback_symbol = 0;
	for (unsigned int i = 0; i < symbol_table->sh_size /
         symbol_table->sh_entsize; i++)
	{
		elf32_symbol *symbol = (elf32_symbol *)(symbol_table->sh_addr
                                + i * symbol_table->sh_entsize);

		if ((addr >= symbol->st_value) &&
			(addr <  symbol->st_value + symbol->st_size) )
		{
			char *c = (char *)(symbol->st_name) + string_table->sh_addr;

			if (symbol_start)
			{
				*symbol_start = symbol->st_value;
			}
			return c;
		}

		if (symbol->st_value > max && symbol->st_value <= addr)
		{
			max = symbol->st_value;
			fallback_symbol = symbol;
		}
	}

	// Search for symbol with size failed, now take a wild guess.
	// Use a biggest symbol value less than addr (if found).
	if (fallback_symbol)
	{
		char *c = (char *)(fallback_symbol->st_name) + string_table->sh_addr;

		if (symbol_start)
		{
			*symbol_start = fallback_symbol->st_value;
		}
		return c;
	}

	if (symbol_start)
		*symbol_start = 0;
	return NULL;
}

}
}

// kate: indent-width 4; replace-tabs on;
// vi:set ts=4:set expandtab=on:
