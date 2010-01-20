//
// Copyright 2007 - 2009, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "elf.h"
#include "types.h"
#include "cstring.h"

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

    elf32::section_header_t* section_string_table() const;

    //! Returns the entry point of the executable.
    inline address_t get_entry_point() { return (address_t)header->entry; }

    inline size_t symbol_entries_count() const { return symbol_table->size / symbol_table->entsize; }
    inline size_t string_entries_count() const { return string_table->size / string_table->entsize; }

    //! Returns true if the parser loaded correctly.
    bool is_valid() const;

protected:
    elf32::header_t*         header;          //!< ELF file header.
    elf32::section_header_t* symbol_table;    //!< Pointer to ELF symbol table in section headers.
    elf32::section_header_t* string_table;    //!< Pointer to ELF string table in section headers.
    elf32::section_header_t* got_table;       //!< Global offset table.
    elf32::section_header_t* rel_table;       //!< Relocations table.
};

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
