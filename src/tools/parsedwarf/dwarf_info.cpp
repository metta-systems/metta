//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2010, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "dwarf_info.h"
#include "datarepr.h"
#include "form_reader.h"
#include "local_panic.h"
#include "dwarf_debug.h"

void cuh_t::decode(address_t from, size_t& offset)
{
    unit_length = *reinterpret_cast<uint32_t*>(from + offset);
    if (unit_length == 0xffffffff)
        PANIC("DWARF64 is not supported!");
    offset += sizeof(uint32_t);
    version = *reinterpret_cast<uint16_t*>(from + offset);
    offset += sizeof(uint16_t);
    debug_abbrev_offset = *reinterpret_cast<uint32_t*>(from + offset);
    offset += sizeof(uint32_t);
    address_size = *reinterpret_cast<uint8_t*>(from + offset);
    offset += sizeof(uint8_t);
}

void cuh_t::dump()
{
    DPRINT("compilation unit header: unit-length %d bytes, version %04x, debug-abbrev-offset 0x%x, address_size %d\n", unit_length, version, debug_abbrev_offset, address_size);
}

die_t& die_t::operator=(const die_t& d)
{
    if (this != &d)
    {
        parser = d.parser;
        abbrev_code = d.abbrev_code;
        node_attributes = d.node_attributes;
        tag = d.tag;
        has_children = d.has_children;
        parent = d.parent;
        children = d.children;
    }
    return *this;
}

bool die_t::decode(address_t from, size_t& offset)
{
    offs = offset;
    abbrev_code = uleb128_t::decode(from, offset, -1);
    if (abbrev_code == 0)
    {
        DPRINT("Last sibling node\n");
        return false;
    }
    // find abbreviation
    auto abbrev = parser.debug_info->find_abbrev(abbrev_code);
    if (abbrev)
    {
        tag = abbrev->tag;
        has_children = abbrev->has_children;
        for (size_t i = 0; i < abbrev->attributes.size()-1; ++i)
        {
            uint32_t name = abbrev->attributes[i].name;
            node_attributes[name] = form_reader_t::create(parser, abbrev->attributes[i].form);
            node_attributes[name]->decode(from, offset);
        }
#if DWARF_DEBUG
        dump();
#endif
        return true;
    }
    return false;
}

die_t* die_t::find_address(address_t addr, address_t& low_pc, address_t& high_pc)
{
    low_pc = -1;
    high_pc = 0;

    addr_form_reader_t* f1 = dynamic_cast<addr_form_reader_t*>(node_attributes[DW_AT_low_pc]);
    if (f1)
        low_pc = f1->data;

    addr_form_reader_t* f2 = dynamic_cast<addr_form_reader_t*>(node_attributes[DW_AT_high_pc]);
    if (f2)
        high_pc = f2->data;

    if (is_subprogram() && low_pc <= addr && high_pc >= addr)
    {
        DPRINT("FOUND TARGET SUBROUTINE FROM %x to %x\n", low_pc, high_pc);
        return this;
    }

    for (size_t i = 0; i < children.size(); ++i)
    {
        die_t* c = children[i]->find_address(addr, low_pc, high_pc);
        if (c)
            return c;
    }

    return 0;
}

die_t* die_t::find_by_offset(size_t offset)
{
    if (offs == offset)
        return this;

    for (size_t i = 0; i < children.size(); ++i)
    {
        die_t* c = children[i]->find_by_offset(offset);
        if (c)
            return c;
    }

    return 0;
}

die_t* die_t::find_compile_unit()
{
    die_t* node = this;
    while (node)
    {
        if (node->tag == DW_TAG_compile_unit)
            return node;
        node = node->parent;
    }
    return 0;
}

const char* die_t::string_attr(uint32_t attr)
{
    auto strp = dynamic_cast<strp_form_reader_t*>(node_attributes[attr]);
    if (strp)
        return strp->data;
    auto str = dynamic_cast<string_form_reader_t*>(node_attributes[attr]);
    if (str)
        return str->data;
    return 0;
}

void die_t::dump()
{
#if DWARF_DEBUG
    printf("*DIE: abbrev %d tag %08x %s (has_children %d)\n", abbrev_code, tag, tag2name(tag), has_children);
    auto abbrev = parser.debug_info->find_abbrev(abbrev_code);
    if (abbrev)
    {
        for (size_t i = 0; i < abbrev->attributes.size()-1; ++i)
        {
            uint32_t name = abbrev->attributes[i].name;
            uint32_t form = abbrev->attributes[i].form;
            printf("  form: %s, name: %x %s, value: ", form2name(form), name, attr2name(name));
            node_attributes[name]->print();
        }
    }
#endif
}
