//
// Copyright 2007 - 2009, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "elf_parser.h"
#include "default_console.h"
#include "memutils.h"
#include "memory.h"
#include "minmax.h"
#include "config.h"

using namespace elf32;

elf_parser_t::elf_parser_t()
    : header(NULL)
    , symbol_table(NULL)
    , string_table(NULL)
    , got_table(NULL)
    , rel_table(NULL)
{
}

elf_parser_t::elf_parser_t(address_t image_base)
    : header(NULL)
    , symbol_table(NULL)
    , string_table(NULL)
    , got_table(NULL)
    , rel_table(NULL)
{
    parse(image_base);
}

program_header_t* elf_parser_t::program_header(int index) const
{
    if (index < 0 || index > header->phnum)
        return 0;
    return reinterpret_cast<program_header_t*>(start + h->phoff + index * h->phentsize);
}

section_header_t* elf_parser_t::section_header(int index) const
{
    if (index < 0 || index > header->shnum)
        return 0;
    return reinterpret_cast<section_header_t*>(start + h->shoff + index * h->shentsize);
}

section_header_t* elf_parser_t::section_header(cstring_t name) const
{
}

/*!
 * Parse ELF program image, finding corresponding sections.
 */
bool elf_parser_t::parse(address_t start)
{
    header_t* h = reinterpret_cast<header_t*>(start);

#define ERROR_RETURN_ON(x, s) \
if (x) { \
    kconsole << RED << "elf file parsing @" << start << " failed: " << s <<endl; \
    return false; \
}

    ERROR_RETURN_ON(h->magic != ELF_MAGIC, "bad magic")
    ERROR_RETURN_ON(h->elfclass != ELF_CLASS_32, "wrong class")
    ERROR_RETURN_ON(h->data != ELF_DATA_2LSB, "wrong endianness")
    ERROR_RETURN_ON(h->machine != EM_386, "wrong architecture")
    ERROR_RETURN_ON(h->version != EV_CURRENT, "wrong version")

    header = h;
    symbol_table = section_header_by_type(SHT_SYMTAB);
    string_table = section_header(".strtab");
    debug_table = section_header(".debug_frame");

    return true;
}

// TODO: use debugging info if present
// char* elf_parser::find_symbol(address_t addr, address_t* symbol_start)
// {
//     address_t max = 0;
//     elf32::symbol* fallback_symbol = 0;
// 
//     for (unsigned int i = 0; i < symbol_entries_count(); i++)
//     {
//         elf32::symbol* symbol = reinterpret_cast<elf32::symbol*>(symbol_table->addr + i * symbol_table->entsize);
// 
//         if ((addr >= symbol->value) && (addr <  symbol->value + symbol->size))
//         {
//             char* c = reinterpret_cast<char*>(symbol->name) + string_table->addr;
// 
//             if (symbol_start)
//                 *symbol_start = symbol->value;
//             return c;
//         }
// 
//         if (symbol->value > max && symbol->value <= addr)
//         {
//             max = symbol->value;
//             fallback_symbol = symbol;
//         }
//     }
// 
//     // Search for symbol with size failed, now take a wild guess.
//     // Use a biggest symbol value less than addr (if found).
//     if (fallback_symbol)
//     {
//         char* c = reinterpret_cast<char*>(fallback_symbol->name) + string_table->addr;
// 
//         if (symbol_start)
//             *symbol_start = fallback_symbol->value;
//         return c;
//     }
// 
//     if (symbol_start)
//         *symbol_start = 0;
//     return NULL;
// }

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
