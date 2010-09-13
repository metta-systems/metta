//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2010, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "elf_parser.h"

//! Parse an ELF file, generate symbolic information and load code/data segments.
class elf_loader_t : public elf_parser_t
{
public:
    //! Creates a blank ELF loader, prepared for a call to load_image.
    elf_loader_t();
    elf_loader_t(address_t image_base);

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

private:
};

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
