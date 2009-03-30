//
// Copyright 2007 - 2009, Stanislav Karchebnyy <berkus+metta@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "elf_parser.h"

elf_parser::elf_parser()
{
	header              = NULL;
	section_headers     = NULL;
	symbol_table        = NULL;
	string_table        = NULL;
	got_table           = NULL;
	filename            = NULL;
}

// elf_parser::~elf_parser()
// {
// 	delete header;
// 	delete section_headers;
// 	delete [] filename;
// }

void elf_parser::load_image(address_t start, size_t size)
{
    (void)start;
    (void)size;
}

// @todo use debugging info if present
char* elf_parser::find_symbol(address_t addr, address_t *symbol_start)
{
	address_t max = 0;
	elf32::symbol *fallback_symbol = 0;
	for (unsigned int i = 0; i < symbol_table->size /
         symbol_table->entsize; i++)
	{
		elf32::symbol *symbol = (elf32::symbol *)(symbol_table->addr
                                + i * symbol_table->entsize);

		if ((addr >= symbol->value) &&
			(addr <  symbol->value + symbol->size) )
		{
			char *c = (char *)(symbol->name) + string_table->addr;

			if (symbol_start)
			{
				*symbol_start = symbol->value;
			}
			return c;
		}

		if (symbol->value > max && symbol->value <= addr)
		{
			max = symbol->value;
			fallback_symbol = symbol;
		}
	}

	// Search for symbol with size failed, now take a wild guess.
	// Use a biggest symbol value less than addr (if found).
	if (fallback_symbol)
	{
		char *c = (char *)(fallback_symbol->name) + string_table->addr;

		if (symbol_start)
		{
			*symbol_start = fallback_symbol->value;
		}
		return c;
	}

	if (symbol_start)
		*symbol_start = 0;
	return NULL;
}

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
