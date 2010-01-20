#include "dwarf_parser.h"
#include "dwarf_abbrev.h"
#include "dwarf_aranges.h"
#include "dwarf_info.h"
#include "datarepr.h"
#include "form_reader.h"
#include <stdio.h>

using namespace elf32; // FIXME: only elf32 is supported, will fail on x86-64

dwarf_parser_t::dwarf_parser_t(elf_parser_t& elf) : elf_parser(elf)
{
    section_header_t* h = elf_parser.section_header(".debug_aranges");
    section_header_t* b = elf_parser.section_header(".debug_abbrev");
    section_header_t* g = elf_parser.section_header(".debug_info");
    debug_str = elf_parser.section_header(".debug_str");
    if (h && g && b)
    {
        debug_abbrev = new dwarf_debug_abbrev_t(elf_parser.start() + b->offset, b->size);
        debug_aranges = new dwarf_debug_aranges_t(elf_parser.start() + h->offset, h->size);
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

                auto ref = dynamic_cast<ref4_form_reader_t*>(die.node_attributes[DW_AT_specification]);
                if (ref)
                {
                    die_t progdesc(*this);
                    size_t progoff = ref->data + cuh_offset;
                    progdesc.decode(debug_info->start, progoff);

                    auto name = dynamic_cast<strp_form_reader_t*>(progdesc.node_attributes[DW_AT_name]);
                    auto file = dynamic_cast<data1_form_reader_t*>(progdesc.node_attributes[DW_AT_decl_file]);
                    auto line = dynamic_cast<data1_form_reader_t*>(progdesc.node_attributes[DW_AT_decl_line]);
                    auto mang = dynamic_cast<strp_form_reader_t*>(progdesc.node_attributes[DW_AT_GNU_cpp_mangled_name]);

                    if (name && file && line && mang)
                    {
                        // print output data:
                        // [TID] 0xfault_addr func-name (source:line)
                        printf("0x%08x %s / %s (file %x, line %d)\n", addr, name->data, mang->data, file->data, line->data);
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
