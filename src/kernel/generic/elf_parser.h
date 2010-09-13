//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2010, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "elf.h"
#include "types.h"
#include "cstring.h"

// elf.findAllSymbols().withPrefix("exported_").withSuffix("_rootdom")

/*!
 * Provides interface to find loadable sections of the elf file
 * (and all other informative sections and lookup aids).
 */
class elf_parser_t
{
public:
    elf_parser_t();
    elf_parser_t(address_t image_base);

    bool parse(address_t image_base);
    address_t start() { return reinterpret_cast<address_t>(header); }

    elf32::program_header_t* program_header(int index) const;
    inline size_t program_header_count() const { return header->phnum; }
    elf32::section_header_t* section_header(int index) const;
    elf32::section_header_t* section_header(cstring_t name) const;
    elf32::section_header_t* section_header_by_type(elf32::word_t type) const;
    inline size_t section_header_count() const { return header->shnum; }

    elf32::section_header_t* section_shstring_table() const;
    elf32::section_header_t* section_string_table() const;
    elf32::section_header_t* section_symbol_table() const;

    //! Returns the entry point of the executable.
    inline address_t get_entry_point() { return (address_t)header->entry; }

    size_t symbol_entries_count() const;
    size_t string_entries_count() const;

    //! Returns true if the parser loaded correctly.
    bool is_valid() const;

    //! Returns true if elf file has relocations.
    bool is_relocatable() const;

    bool relocate_to(address_t load_address);

protected:
    const char* strtab_pointer(elf32::section_header_t* strtab, elf32::word_t name_offset) const;

    elf32::header_t*         header;          //!< ELF file header.
};

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
