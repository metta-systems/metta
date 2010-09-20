//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2010, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "elf_loader.h"
#include "default_console.h"

using namespace elf32;

elf_loader_t::elf_loader_t(address_t image_base)
    : elf_parser_t(image_base)
{
}

// TODO: use debugging info if present
cstring_t elf_loader_t::find_symbol(address_t addr, address_t* symbol_start)
{
//     section_header_t* debug_table = section_header(".debug_frame");

    address_t max = 0;
    symbol_t* fallback_symbol = 0;
    section_header_t* symbol_table = section_symbol_table();
    if (!symbol_table)
        return NULL;

    for (unsigned int i = 0; i < symbol_entries_count(); i++)
    {
        symbol_t* symbol = reinterpret_cast<symbol_t*>(symbol_table->addr + i * symbol_table->entsize);//FIXME: base addr missing?

        if ((addr >= symbol->value) && (addr < symbol->value + symbol->size))
        {
            const char* c = strtab_pointer(section_string_table(), symbol->name);

            if (symbol_start)
                *symbol_start = symbol->value;
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
        const char* c = strtab_pointer(section_string_table(), fallback_symbol->name);

        if (symbol_start)
            *symbol_start = fallback_symbol->value;
        return c;
    }

    if (symbol_start)
        *symbol_start = 0;
    return NULL;
}

// Find symbol str in symbol table and return its absolute address.
address_t elf_loader_t::find_symbol(cstring_t str)
{
    section_header_t* symbol_table = section_symbol_table();
    if (!symbol_table)
        return 0;

//     kconsole << symbol_entries_count() << " symbols to consider." << endl;
//     kconsole << "Symbol table @ " << start() + symbol_table->offset << endl;
//     kconsole << "BSS          @ " << start() + section_header(".bss")->offset << endl;

    for (unsigned int i = 0; i < symbol_entries_count(); i++)
    {
        symbol_t* symbol = reinterpret_cast<symbol_t*>(start() + symbol_table->offset + i * symbol_table->entsize);

        const char* c = strtab_pointer(section_string_table(), symbol->name);
//         kconsole << "Looking at symbol " << c << " @ " << symbol << endl;
        if (str == c)
        {
            if (ELF32_ST_TYPE(symbol->info) == STT_SECTION)
            {
                return section_header(symbol->shndx)->offset + start();
            }
            else
            {
                return section_header(symbol->shndx)->offset + symbol->value + start();
            }
        }
    }

    return 0;
}

//TODO:
// symbol_table_t
// iterator for searching the symbols by name
// non-linear lookups

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
