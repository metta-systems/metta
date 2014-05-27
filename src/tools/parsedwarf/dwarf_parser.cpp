//
// Part of Metta OS. Check http://atta-metta.net for latest version.
//
// Copyright 2010 - 2012, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "dwarf_parser.h"
#include "dwarf_abbrev.h"
#include "dwarf_aranges.h"
#include "dwarf_lines.h"
#include "dwarf_info.h"
#include "datarepr.h"
#include "form_reader.h"
#include "dwarf_debug.h"

using namespace elf32; // FIXME: only elf32 is supported, will fail on x86-64

dwarf_parser_t::dwarf_parser_t(elf_parser_t& elf) : elf_parser(elf), root(0)
{
    section_header_t* h = elf_parser.section_header(".debug_aranges");
    section_header_t* b = elf_parser.section_header(".debug_abbrev");
    section_header_t* l = elf_parser.section_header(".debug_line");
    section_header_t* g = elf_parser.section_header(".debug_info");
    debug_str = elf_parser.section_header(".debug_str");
    if (h && g && b /*&& l*/)
    {
        debug_abbrev = new dwarf_debug_abbrev_t(elf_parser.start() + b->offset, b->size);
        debug_aranges = new dwarf_debug_aranges_t(elf_parser.start() + h->offset, h->size);
        debug_lines = new dwarf_debug_lines_t(elf_parser.start() + l->offset, l->size);
        debug_info = new dwarf_debug_info_t(elf_parser.start() + g->offset, g->size, *debug_abbrev);
    }
#if DWARF_DEBUG
    else
        printf("No required DWARF debug sections found!\n");
#endif
}

dwarf_parser_t::~dwarf_parser_t()
{
    delete debug_info;
    delete debug_aranges;
    delete debug_abbrev;
    delete debug_lines;
}

dwarf_parser_t& dwarf_parser_t::operator=(const dwarf_parser_t& d)
{
    if (this != &d)
    { // FIXME: will double-free!
        elf_parser = d.elf_parser;
        debug_info = d.debug_info;
        debug_aranges = d.debug_aranges;
        debug_abbrev = d.debug_abbrev;
        debug_lines = d.debug_lines;
        debug_str = d.debug_str;
        root = d.root;
    }
    return *this;
}

die_t* dwarf_parser_t::build_tree(size_t& offset)
{
    die_t* rootnode = new die_t(*this);
    rootnode->decode(debug_info->start, offset);

    if (rootnode->is_last()) // degenerate tree
    {
        delete rootnode;
        return 0;
    }

    if (rootnode->has_children)
        build_tree_recurse(rootnode, offset);

    return rootnode;
}

void dwarf_parser_t::build_tree_recurse(die_t* thisnode, size_t& offset)
{
    while (1)
    {
        die_t* child = new die_t(*this);
        child->decode(debug_info->start, offset);

        if (child->is_last())
        {
            delete child;
            return;
        }

        child->parent = thisnode;
        if (thisnode)
            thisnode->children.push_back(child);

        if (child->has_children)
            build_tree_recurse(child, offset); // append children to DIE
    }
}

bool dwarf_parser_t::lookup(address_t addr)
{
    size_t cuh_offset;
    size_t offset = 0;
    address_t low_pc, high_pc;
    // find addr in debug info and figure function name and source file line
    if (debug_aranges->lookup(addr, offset))
    {
//         printf("FOUND ADDRESS\n");
        cuh_offset = offset;
        cuh_t cuh;
        cuh = debug_info->get_cuh(offset);
#if DWARF_DEBUG
        cuh.dump();
#endif

        size_t abbr_offset = cuh.debug_abbrev_offset;
        debug_abbrev->load_abbrev_set(abbr_offset); // TODO: cache abbrevs (key: given offset)

        // Build DIE tree for a given compilation unit.
        root = build_tree(offset);

        die_t* node = 0;
        // Traverse tree to find the subroutine containing addr
        if (root)
            node = root->find_address(addr, low_pc, high_pc);

        if (node)
        {
            printf("0x%08x", addr);
            node->dump();

            die_t* named_node = find_named_node(node, cuh_offset);
            if (named_node)
            {
                named_node->dump();
                auto name = named_node->string_attr(DW_AT_name);
/*                auto file = dynamic_cast<data1_form_reader_t*>(named_node->node_attributes[DW_AT_decl_file]);
                auto line = dynamic_cast<data1_form_reader_t*>(named_node->node_attributes[DW_AT_decl_line]);*/
                auto mang = named_node->string_attr(DW_AT_GNU_cpp_mangled_name);

                if (name /*&& file && line*/)
                {
                    // print output data:
                    // [TID] 0xfault_addr func-name (source:line)
                    printf(" %s (%s)" /*(file %x, line %d)\n"*/, name, mang ? mang : "<no mangled name>"/*, file->data, line->data*/);
                }
            }

            die_t* cu_node = node->find_compile_unit();
            if (cu_node)
            {
                cu_node->dump();
                auto stmt = dynamic_cast<data4_form_reader_t*>(cu_node->node_attributes[DW_AT_stmt_list]);
                if (stmt)
                {
                    size_t ofs = stmt->data;
                    if (debug_lines->execute(ofs))
                    {
                        printf(" (%s:%d)", debug_lines->file_name(addr, low_pc, high_pc).c_str(), debug_lines->line_number(addr, low_pc, high_pc));
                    }
                }
            }
            printf("\n");
        }

        return true;
    }
    else
        printf("0x%08x <unresolved>\n", addr);

    return false;
}

die_t* dwarf_parser_t::find_named_node(die_t* node, size_t cuh_offset)
{
    auto name = dynamic_cast<strp_form_reader_t*>(node->node_attributes[DW_AT_name]);
    if (name)
        return node;

    auto ref = dynamic_cast<ref4_form_reader_t*>(node->node_attributes[DW_AT_specification]);
    if (!ref)
        ref = dynamic_cast<ref4_form_reader_t*>(node->node_attributes[DW_AT_abstract_origin]);
    if (ref)
    {
        return root->find_by_offset(ref->data + cuh_offset);
    }

    return 0;
}
