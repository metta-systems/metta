#pragma once

#include "elf_parser.h"
#include "dwarf_info.h"

class dwarf_debug_info_t;
class dwarf_debug_aranges_t;
class dwarf_debug_abbrev_t;
class dwarf_debug_lines_t;

// Overall wrapper and handler for parsing ELF's DWARF debug information.
class dwarf_parser_t
{
public:
    elf_parser_t& elf_parser;
    dwarf_debug_info_t* debug_info;
    dwarf_debug_aranges_t* debug_aranges;
    dwarf_debug_abbrev_t* debug_abbrev;
    dwarf_debug_lines_t* debug_lines;
    elf32::section_header_t* debug_str;
    die_t* root; // root of DIE tree

    dwarf_parser_t(elf_parser_t& elf);
    ~dwarf_parser_t();
    dwarf_parser_t& operator=(const dwarf_parser_t&);

    bool lookup(address_t addr);

    die_t* build_tree(size_t& offset);
    die_t* find_named_node(die_t* node, size_t cuh_offset);

private:
    void build_tree_recurse(die_t* thisnode, size_t& offset);
};
