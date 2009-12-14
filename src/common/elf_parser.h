//
// Copyright 2007 - 2009, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "elf.h"
#include "types.h"

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

    program_header_t* program_header(int index) const;
    inline size_t program_header_count() const { return header->phnum; }
    section_header_t* section_header(int index) const;
    section_header_t* section_header(cstring_t name) const;
    inline size_t section_header_count() const { return header->shnum; }

    //! Returns the entry point of the executable.
    inline address_t get_entry_point() { return (address_t)header->entry; }

    inline size_t symbol_entries_count() const { return symbol_table->size / symbol_table->entsize; }
    inline size_t string_entries_count() const { return string_table->size / string_table->entsize; }


    //! Returns the symbol name for an address.
    /*!
     *  Also returns the start address of that symbol in symbol_start if symbol_start is non-NULL.
     */
//     cstring_t find_symbol(address_t addr, address_t* symbol_start = NULL);

    //! Returns the address of a symbol with name str.
//     address_t find_symbol(cstring_t str);

    //! Returns the address of the symbol with offset o in the relocation symbol table.
//     address_t find_dynamic_symbol_location(address_t o);

    //! Returns a NULL terminated name of the symbol at given offset in the relocation symbol table.
//     cstring_t find_dynamic_symbol_name(address_t o);

    //! Gets the address of the global offset table.
//     address_t get_global_offset_table();

    //! Returns last location occupied by the elf image in memory.
//     address_t get_alloc_end();

    //! Returns true if the parser loaded correctly.
//     bool is_valid()
//     {
//         return (header != 0);
//     }

private:
    elf32::header_t*         header;          //!< ELF file header.
    elf32::section_header_t* symbol_table;    //!< Pointer to ELF symbol table in section headers.
    elf32::section_header_t* string_table;    //!< Pointer to ELF string table in section headers.
    elf32::section_header_t* got_table;       //!< Global offset table.
    elf32::section_header_t* rel_table;       //!< Relocations table.
};

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
