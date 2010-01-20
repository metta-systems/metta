#pragma once

#include "elf_parser.h"

class dwarf_debug_info_t;
class dwarf_debug_aranges_t;
class dwarf_debug_abbrev_t;

// Overall wrapper and handler for parsing ELF's DWARF debug information.
class dwarf_parser_t
{
public:
    elf_parser_t& elf_parser;
    dwarf_debug_info_t* debug_info;
    dwarf_debug_aranges_t* debug_aranges;
    dwarf_debug_abbrev_t* debug_abbrev;
    elf32::section_header_t* debug_str;

    dwarf_parser_t(elf_parser_t& elf);
    ~dwarf_parser_t();

    bool lookup(address_t addr);
};
