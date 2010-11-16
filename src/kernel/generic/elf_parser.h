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
#include "iterator"

// elf.find_all_symbols().with_prefix("exported_").with_suffix("_rootdom")

/*!
 * Provides interface to find loadable sections of the elf file
 * (and all other informative sections and lookup aids).
 */
class elf_parser_t
{
public:
    /* Iterator for going over available program headers. */
    class program_iterator : public std::iterator<std::forward_iterator_tag, elf32::program_header_t>
    {
        elf32::program_header_t* ptr;
        elf32::program_header_t* end;
        size_t entry_size;

    public:
        program_iterator(elf32::program_header_t* entry, elf32::program_header_t* end, size_t entry_size);
        elf32::program_header_t operator *();
        void operator ++();
        inline bool operator != (const program_iterator& other) { return ptr != other.ptr; }
    };

    /* Iterator for going over available section headers. */
    class section_iterator : public std::iterator<std::forward_iterator_tag, elf32::section_header_t>
    {
        elf32::section_header_t* ptr;
        elf32::section_header_t* end;
        size_t entry_size;

    public:
        section_iterator(elf32::section_header_t* entry, elf32::section_header_t* end, size_t entry_size);
        elf32::section_header_t operator *();
        void operator ++();
        inline bool operator != (const section_iterator& other) { return ptr != other.ptr; }
    };

    elf_parser_t();
    elf_parser_t(address_t image_base);

    bool parse(address_t image_base);
    address_t start() { return reinterpret_cast<address_t>(header); }

    // Iterator based interface.
    program_iterator program_headers_begin();
    program_iterator program_headers_end();

    // Old school interface.
    elf32::program_header_t* program_header(int index) const;
    inline size_t program_header_count() const { return header->phnum; }

    // Iterator based interface.
    section_iterator section_headers_begin();
    section_iterator section_headers_end();

    // Old school interface.
    elf32::section_header_t* section_header(int index) const;
    elf32::section_header_t* section_header(cstring_t name) const;
    elf32::section_header_t* section_header_by_type(elf32::word_t type) const;
    inline size_t section_header_count() const { return header->shnum; }

    elf32::section_header_t* section_shstring_table() const;
    inline const char* shstring_table() const { return strtab_pointer(section_shstring_table(), 0); }
    elf32::section_header_t* section_string_table() const;
    inline const char* string_table() const { return strtab_pointer(section_string_table(), 0); }
    elf32::section_header_t* section_symbol_table() const;

    //! Returns the entry point of the executable.
    inline address_t get_entry_point() { return (address_t)header->entry; }

    size_t symbol_entries_count() const;
    size_t string_entries_count() const;

    //! Returns true if the parser loaded correctly.
    bool is_valid() const;

    //! Returns true if elf file has relocations.
    bool is_relocatable() const;

    bool relocate_to(address_t load_address, offset_t base_offs);

    bool apply_relocation(elf32::rel_t& rel, elf32::symbol_t& sym, elf32::section_header_t* target_sect, address_t load_address);

    const char* strtab_pointer(elf32::section_header_t* strtab, elf32::word_t name_offset) const;

    // ELF Loader part
    //! Parse an ELF file, generate symbolic information and load code/data segments.

    //! Loads the image file from specified memory location.
    /*!
     * 
     */
    bool load_image(address_t start, size_t size);

    //! Returns the symbol name for an address.
    /*!
     *  Also returns the start address of that symbol in symbol_start if symbol_start is non-NULL.
     */
    cstring_t find_symbol(address_t addr, address_t* symbol_start = NULL);

    //! Returns the address of a symbol with name str.
    address_t find_symbol(cstring_t str);

    //! Returns the address of the symbol with offset o in the relocation symbol table.
    address_t find_dynamic_symbol_location(address_t o);

    //! Returns a NULL terminated name of the symbol at given offset in the relocation symbol table.
    cstring_t find_dynamic_symbol_name(address_t o);

    //! Gets the address of the global offset table.
    address_t get_global_offset_table();

    //! Returns last location occupied by the elf image in memory.
    address_t get_alloc_end();

protected:

    elf32::header_t*         header;          //!< ELF file header.
};

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
