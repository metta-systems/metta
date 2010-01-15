//
// Copyright 2007 - 2009, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "elf_parser.h"
// #include "default_console.h"
#include "memutils.h"
// #include "memory.h"
// #include "minmax.h"
// #include "config.h"

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

static inline address_t elf2loc(header_t* base, size_t offset)
{
    return reinterpret_cast<address_t>(base) + offset;
}

program_header_t* elf_parser_t::program_header(int index) const
{
    if (index < 0 || index > header->phnum)
        return 0;
    return reinterpret_cast<program_header_t*>(elf2loc(header, header->phoff + index * header->phentsize));
}

section_header_t* elf_parser_t::section_header(int index) const
{
    if (index < 0 || index > header->shnum)
        return 0;
    return reinterpret_cast<section_header_t*>(elf2loc(header, header->shoff + index * header->shentsize));
}

section_header_t* elf_parser_t::section_header_by_type(word_t type) const
{
    section_header_t* s;
    for (int i = 0; i < header->shnum; i++)
    {
        s = section_header(i);
        if (s->type == type)
            return s;
    }
    return 0;
}

section_header_t* elf_parser_t::section_string_table() const
{
    if (header->shstrndx == SHN_UNDEF)
        return 0;
    return section_header(header->shstrndx);
}

section_header_t* elf_parser_t::section_header(cstring_t name) const
{
    section_header_t* strtab = section_string_table();
    if (!strtab)
        return 0;

    section_header_t* s;
    for (int i = 0; i < header->shnum; i++)
    {
        s = section_header(i);
        if (cstring_t(reinterpret_cast<char*>(header) + strtab->offset + s->name) == name)
            return s;
    }
    return 0;
}

bool elf_parser_t::is_valid() const
{
#define ERROR_RETURN_ON(x) \
if (x) { \
    return false; \
}

    ERROR_RETURN_ON(!header);
    ERROR_RETURN_ON(header->magic != ELF_MAGIC);
    ERROR_RETURN_ON(header->elfclass != ELF_CLASS_32);
    ERROR_RETURN_ON(header->data != ELF_DATA_2LSB);
    ERROR_RETURN_ON(header->machine != EM_386);
    ERROR_RETURN_ON(header->version != EV_CURRENT);

    return true;
#undef ERROR_RETURN_ON
}

/*!
 * Parse ELF program image, finding corresponding sections.
 */
bool elf_parser_t::parse(address_t start)
{
    header = reinterpret_cast<header_t*>(start);

//     kconsole << RED << "elf file parsing @" << start << " failed: " << s <<endl;
#define ERROR_RETURN_ON(x, s) \
if (x) { \
    return false; \
}

    ERROR_RETURN_ON(header->magic != ELF_MAGIC, "bad magic")
    ERROR_RETURN_ON(header->elfclass != ELF_CLASS_32, "wrong class")
    ERROR_RETURN_ON(header->data != ELF_DATA_2LSB, "wrong endianness")
    ERROR_RETURN_ON(header->machine != EM_386, "wrong architecture")
    ERROR_RETURN_ON(header->version != EV_CURRENT, "wrong version")

    symbol_table = section_header_by_type(SHT_SYMTAB);
    string_table = section_header(".strtab");

    return true;
#undef ERROR_RETURN_ON
}

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
