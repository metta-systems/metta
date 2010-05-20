//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2010, Stanislav Karchebnyy <berkus@exquance.com>
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
{
}

elf_parser_t::elf_parser_t(address_t image_base)
    : header(NULL)
{
    parse(image_base);
}

static inline address_t elf2loc(header_t* base, size_t offset)
{
    return reinterpret_cast<address_t>(base) + offset;
}

program_header_t* elf_parser_t::program_header(int index) const
{
    if (index < 0 || index >= header->phnum)
        return 0;
    return reinterpret_cast<program_header_t*>(elf2loc(header, header->phoff + index * header->phentsize));
}

section_header_t* elf_parser_t::section_header(int index) const
{
    if (index < 0 || index >= header->shnum)
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

section_header_t* elf_parser_t::section_shstring_table() const
{
    if (header->shstrndx == SHN_UNDEF)
        return 0;
    return section_header(header->shstrndx);
}

section_header_t* elf_parser_t::section_string_table() const
{
    return section_header(".strtab");
}

size_t elf_parser_t::string_entries_count() const
{
    section_header_t* strtab = section_string_table();
    if (!strtab)
        return 0;
    return strtab->size / strtab->entsize;
}

section_header_t* elf_parser_t::section_symbol_table() const
{
    return section_header_by_type(SHT_SYMTAB);
}

size_t elf_parser_t::symbol_entries_count() const
{
    section_header_t* symtab = section_symbol_table();
    if (!symtab)
        return 0;
    return symtab->size / symtab->entsize;
}

section_header_t* elf_parser_t::section_header(cstring_t name) const
{
    section_header_t* shstrtab = section_shstring_table();
    if (!shstrtab)
        return 0;

    section_header_t* s;
    for (int i = 0; i < header->shnum; i++)
    {
        s = section_header(i);
        if (cstring_t(strtab_pointer(shstrtab, s->name)) == name)
            return s;
    }
    return 0;
}

const char* elf_parser_t::strtab_pointer(section_header_t* strtab, elf32::word_t name_offset) const
{
    if (!strtab)
        return 0;
    return reinterpret_cast<char*>(header) + strtab->offset + name_offset;
}

bool elf_parser_t::is_valid() const
{
//     kconsole << RED << "elf file parsing @" << start << " failed: " << s <<endl;
#define ERROR_RETURN_ON(x, s) \
if (x) { \
    return false; \
}

    ERROR_RETURN_ON(!header, "no header");
    ERROR_RETURN_ON(header->magic != ELF_MAGIC, "bad magic")
    ERROR_RETURN_ON(header->elfclass != ELF_CLASS_32, "wrong class")
    ERROR_RETURN_ON(header->data != ELF_DATA_2LSB, "wrong endianness")
    ERROR_RETURN_ON(header->machine != EM_386, "wrong architecture")
    ERROR_RETURN_ON(header->version != EV_CURRENT, "wrong version")

    return true;
#undef ERROR_RETURN_ON
}

/*!
 * Parse ELF program image, finding corresponding sections.
 */
bool elf_parser_t::parse(address_t start)
{
    header = reinterpret_cast<header_t*>(start);

    if (!is_valid())
        return false;

    return true;
}

bool elf_parser_t::is_relocatable() const
{
    return header && header->type == ET_REL;
}

bool elf_parser_t::relocateTo(address_t load_address)
{
    section_header_t* shstrtab = section_shstring_table();
    if (!shstrtab)
        return false;

    // Traverse all sections, find relocation sections and apply them.
    section_header_t* rel_section;
    for (int i = 0; i < header->shnum; i++)
    {
        rel_section = section_header(i);
        if (rel_section->type == SHT_REL)
        {
//             kconsole << "Found rel section " << strtab_pointer(shstrtab, rel_section->name) << " @" << rel_section->offset << endl;
            elf32::rel_t* rels = reinterpret_cast<elf32::rel_t*>(elf2loc(header, rel_section->offset));
            size_t nrels = rel_section->size / sizeof(rels[0]);
            section_header_t* symtab = section_header(rel_section->link);
            symbol_t* symbols = symtab ? reinterpret_cast<symbol_t*>(elf2loc(header, symtab->offset)) : 0;
            section_header_t* target_sect = section_header(rel_section->info);
            if (symbols)
            {
                for (size_t j = 0; j < nrels; j++)
                {
                    elf32::rel_t& rel = rels[j];
                    symbol_t& sym = symbols[ELF32_R_SYM(rel.info)];

                    uint32_t result;
                    address_t P = target_sect->offset + rel.offset + load_address;
                    uint32_t  A = *reinterpret_cast<uint32_t*>(P);
                    address_t S = 0;

                    if (ELF32_ST_TYPE(sym.info) == STT_SECTION)
                    {
                        S = section_header(sym.shndx)->offset + load_address;
//                         kconsole << "S is section '" << strtab_pointer(shstrtab, section_header(sym.shndx)->name) << "'" << endl;
                    }
                    else
                    {
                        S = section_header(sym.shndx)->offset + sym.value + load_address;
//                         kconsole << "S is symbol '" << strtab_pointer(section_string_table(), sym.name) << "' for section " << strtab_pointer(shstrtab, section_header(sym.shndx)->name) << endl;
                    }

                    switch (ELF32_R_TYPE(rel.info))
                    {
                        case R_386_32:
                            result = S + A;
//                             kconsole << "R_386_32: S " << S << " + A " << A << " = " << result << endl;
                            break;
                        case R_386_PC32:
                            result = S + A - P;
//                             kconsole << "R_386_PC32: S " << S << " + A " << A << " - P " << P << " = " << result << endl;
                            break;
                        default:
//                             kconsole << "Unknown relocation type found, skipped, expect crashes!" << endl;
                            break;
                    }
                    *reinterpret_cast<uint32_t*>(P) = result;
                }
            }
        }
    }

    // Zero-out BSS
    section_header_t* bss = section_header(".bss");
    memutils::fill_memory(reinterpret_cast<void*>(load_address + bss->offset), 0, bss->size);

    return true;
}

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
