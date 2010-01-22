#include "dwarf_parser.h"
#include "dwarf_abbrev.h"
#include "dwarf_aranges.h"
#include "dwarf_lines.h"
#include "dwarf_info.h"
#include "datarepr.h"
#include "form_reader.h"
#include <stdio.h>

using namespace elf32; // FIXME: only elf32 is supported, will fail on x86-64

dwarf_parser_t::dwarf_parser_t(elf_parser_t& elf) : elf_parser(elf)
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
    else
        printf("No required DWARF debug sections found!\n");
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
    }
    return *this;
}

bool dwarf_parser_t::lookup(address_t addr)
{
    size_t cuh_offset;
    size_t offset = 0;
    // find addr in debug info and figure function name and source file line
    if (debug_aranges->lookup(addr, offset))
    {
        printf("FOUND ADDRESS\n");
        cuh_offset = offset;
        cuh_t cuh;
        cuh = debug_info->get_cuh(offset);
        printf("decoded cuh: unit-length %d, version %04x, debug-abbrev-offset %d\n", cuh.unit_length, cuh.version, cuh.debug_abbrev_offset);

        size_t abbr_offset = cuh.debug_abbrev_offset;
        debug_abbrev->load_abbrev_set(abbr_offset); // TODO: cache abbrevs (key: given offset)

        // continue parsing the cuh using abbrev data with given code
        while (1)
        {
            die_t die(*this);
            die.decode(debug_info->start, offset);

            if (die.is_subprogram && die.low_pc <= addr && die.high_pc >= addr)
            {
                printf("FOUND TARGET SUBROUTINE FROM %x to %x\n", die.low_pc, die.high_pc);

                die = resolve_refs(cuh_offset, die);

                auto name = dynamic_cast<strp_form_reader_t*>(die.node_attributes[DW_AT_name]);
                auto file = dynamic_cast<data1_form_reader_t*>(die.node_attributes[DW_AT_decl_file]);
                auto line = dynamic_cast<data1_form_reader_t*>(die.node_attributes[DW_AT_decl_line]);
                auto mang = dynamic_cast<strp_form_reader_t*>(die.node_attributes[DW_AT_GNU_cpp_mangled_name]);
                auto stmt = dynamic_cast<data4_form_reader_t*>(die.node_attributes[DW_AT_stmt_list]);

                if (name && file && line)
                {
                    // print output data:
                    // [TID] 0xfault_addr func-name (source:line)
                    printf("0x%08x %s (%s) (file %x, line %d)\n", addr, name->data, mang ? mang->data : "<no mangled name>", file->data, line->data);

                    if (stmt)
                    {
                        size_t ofs = stmt->data;
                        if (debug_lines->execute(ofs))
                            printf("line program PARSED OK\n");
                    }
                }

                break;
            }

            if (die.is_last() && (offset >= debug_info->size))
                break;
        }

        return true;
    }
    else
        printf("NOT FOUND\n");

    return false;
}

die_t dwarf_parser_t::resolve_refs(address_t cuh_offset, die_t desc)
{
    auto name = dynamic_cast<strp_form_reader_t*>(desc.node_attributes[DW_AT_name]);
    if (name)
        return desc;
    auto ref = dynamic_cast<ref4_form_reader_t*>(desc.node_attributes[DW_AT_specification]);
    if (!ref)
        ref = dynamic_cast<ref4_form_reader_t*>(desc.node_attributes[DW_AT_abstract_origin]);
    if (ref)
    {
        die_t desc2(*this);
        size_t progoff = ref->data + cuh_offset;
        desc2.decode(debug_info->start, progoff);
        return resolve_refs(cuh_offset, desc2);
    }
    else
        return die_t(*this);
}
