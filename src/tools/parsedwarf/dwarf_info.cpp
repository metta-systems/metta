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
    // find abbreviation for tag
    auto forms = parser.debug_info->find_abbrev(tag);
    if (forms)
    {
        printf("ABBREV %d TAG %08x %s\n", (uint32_t)tag, (uint32_t)forms->tag, tag2name(forms->tag));
        if (forms->tag == DW_TAG_subprogram)
            is_subprogram = true;
        for (unsigned i = 0; i < forms->attributes.size()-1; ++i)
        {
            uint32_t name = forms->attributes[i].name;
            uint32_t form = forms->attributes[i].form;
            node_attributes[name] = form_reader_t::create(parser, form);
            node_attributes[name]->decode(from, offset);

//   form: 6 DW_FORM_data4, name: DW_AT_ranges, value: <32>0x00000138
//   form: 6 DW_FORM_data4, name: DW_AT_stmt_list, value: <32>0x00000459

            printf("  form: %s, name: %x %s, value: ", form2name(form), (uint32_t)name, attr2name(name));
            node_attributes[name]->print();

            if (name == DW_AT_external)
            {
                flag_form_reader_t* f = dynamic_cast<flag_form_reader_t*>(node_attributes[name]);
                if (f && f->data)
                    is_subprogram = false; // skip externals
            }
            if (name == DW_AT_low_pc)
            {
                addr_form_reader_t* f = dynamic_cast<addr_form_reader_t*>(node_attributes[name]);
                if (f)
                    low_pc = f->data;
            }
            if (name == DW_AT_high_pc)
            {
                addr_form_reader_t* f = dynamic_cast<addr_form_reader_t*>(node_attributes[name]);
                if (f)
                    high_pc = f->data;
            }
        }
    }
}
