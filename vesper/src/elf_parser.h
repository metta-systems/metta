//
// Copyright 2007 - 2008, Stanislav Karchebnyy <berkus+metta@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "elf.h"
#include "types.h"

namespace metta {
namespace kernel {

/**
 * Parses an ELF file, generating symbolic information and loading code/data
 * segments.
 */
class elf_parser
{
public:
    /**
     * Creates a blank ELF parser, preparing for a call to loadKernel.
     */
    elf_parser();

    /**
     * Creates the ELF parser based on a file.
     */
    elf_parser(const char *fname);
    ~elf_parser();

    /**
     * Duplicates (this), performing a deep copy.
     */
    elf_parser *clone();

    /**
     * Writes all section information to the virtual memory image.
     */
    void write_all_sections();

    /**
     * Returns the address of the last byte to be loaded in.
     */
    address_t get_last_address();

    /**
     * Loads the symbol table for the kernel from the specified location.
     */
    void load_kernel(elf32_section_header* symtab,
                     elf32_section_header* strtab);

    /**
     * Returns the symbol name for an address. Also returns the start address
     * of that symbol in startAddr if startAddr != NULL.
     */
    char* find_symbol(address_t addr, address_t *symbol_start = NULL);

    /**
     * Returns the address of a symbol with name str.
     * NOTE: This is much slower than it should be. This should be implemented
     * using the hashtable sections in ELF.
     */
    address_t find_symbol(char* str);

    /**
     * Returns the address of the symbol with offset o in the
     * relocation symbol table.
     */
    address_t find_dynamic_symbol_location(address_t o);

    /**
     * Returns a NULL terminated string specifying the name of
     * the symbol at offset o in the relocation symbol table.
     */
    char *find_dynamic_symbol_name(address_t o);

    /**
     * Gets the address of the global offset table.
     */
    address_t get_global_offset_table();

    /**
     * Returns the entry point of the executable.
     */
    address_t get_entry_point()
    {
        return (address_t)header->e_entry;
    }

    /**
     * Returns true if the parser loaded correctly.
     */
    bool is_valid()
    {
        return (filename != 0);
    }

private:
    elf32_header*         header;
    elf32_section_header* symbol_table;
    elf32_section_header* string_table;
    elf32_section_header* got_table; // Global offset table.
    elf32_section_header* rel_table;
    elf32_section_header* section_headers;
    const char*           filename;
};

}
}

// kate: indent-width 4; replace-tabs on;
// vi:set ts=4:set expandtab=on:
