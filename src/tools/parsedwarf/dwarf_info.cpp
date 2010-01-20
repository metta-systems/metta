#include "dwarf_info.h"
#include "datarepr.h"
#include "form_reader.h"
#include "local_panic.h"
#include <stdio.h>

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

void die_t::decode(address_t from, size_t& offset)
{
    tag.decode(from, offset);
    if (tag == 0)
    {
        printf("Last sibling node\n");
        return;
    }
    printf("TAG %08x %s\n", (uint32_t)tag, tag2name(tag));
    // find abbreviation for tag
    auto forms = parser.debug_info->find_abbrev(tag);
    if (forms)
    {
        for (unsigned i = 0; i < forms->attributes.size()-1; ++i)
        {
            node_attributes[forms->attributes[i].name] = form_reader_t::create(parser, forms->attributes[i].form);
            node_attributes[forms->attributes[i].name]->decode(from, offset);

            printf("  name: %s, value: ", attr2name(forms->attributes[i].name));
            node_attributes[forms->attributes[i].name]->print();

            if (forms->attributes[i].name == DW_AT_low_pc)
            {
                addr_form_reader_t* f = dynamic_cast<addr_form_reader_t*>(node_attributes[forms->attributes[i].name]);
                if (f)
                    low_pc = f->data;
            }
            if (forms->attributes[i].name == DW_AT_high_pc)
            {
                addr_form_reader_t* f = dynamic_cast<addr_form_reader_t*>(node_attributes[forms->attributes[i].name]);
                if (f)
                    high_pc = f->data;
            }
        }
    }
}
