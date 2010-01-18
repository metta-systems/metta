#include "form_reader.h"
#include "datarepr.h"
#include <stdio.h>

form_reader_t* form_reader_t::create(uint32_t form)
{
    printf("form_reader_t::create(%u)\n", form);
    switch (form)
    {
        case DW_FORM_addr:
            return new addr_form_reader_t;
        case DW_FORM_block2:
            return new block2_form_reader_t;
        case DW_FORM_block4:
            return new block4_form_reader_t;
        case DW_FORM_data2:
            return new data2_form_reader_t;
        case DW_FORM_data4:
            return new data4_form_reader_t;
        case DW_FORM_data8:
            return new data8_form_reader_t;
        case DW_FORM_string:
            return new string_form_reader_t;
        case DW_FORM_block:
            return new block_form_reader_t;
        case DW_FORM_block1:
            return new block1_form_reader_t;
        case DW_FORM_data1:
            return new data1_form_reader_t;
        case DW_FORM_flag:
            return new flag_form_reader_t;
        case DW_FORM_sdata:
            return new sdata_form_reader_t;
        case DW_FORM_strp:
            return new strp_form_reader_t;
        case DW_FORM_udata:
            return new udata_form_reader_t;
        case DW_FORM_ref_addr:
            return new ref_addr_form_reader_t;
        case DW_FORM_ref1:
            return new ref1_form_reader_t;
        case DW_FORM_ref2:
            return new ref2_form_reader_t;
        case DW_FORM_ref4:
            return new ref4_form_reader_t;
        case DW_FORM_ref8:
            return new ref8_form_reader_t;
        case DW_FORM_ref_udata:
            return new ref_udata_form_reader_t;
        case DW_FORM_indirect:
            return new indirect_form_reader_t;
    }
    return 0;
}

bool addr_form_reader_t::decode(address_t from, size_t& offset)
{
    data = *reinterpret_cast<uint32_t*>(from + offset);
    offset += sizeof(uint32_t);
    return true;
}

bool block_form_reader_t::decode(address_t from, size_t& offset)
{
    length.decode(from, offset);
    data = reinterpret_cast<char*>(from + offset);
    offset += length;
    return true;
}

bool block1_form_reader_t::decode(address_t from, size_t& offset)
{
    length = *reinterpret_cast<uint8_t*>(from + offset);
    offset += sizeof(uint8_t);
    data = reinterpret_cast<char*>(from + offset);
    offset += length;
    return true;
}

bool block2_form_reader_t::decode(address_t from, size_t& offset)
{
    length = *reinterpret_cast<uint16_t*>(from + offset);
    offset += sizeof(uint16_t);
    data = reinterpret_cast<char*>(from + offset);
    offset += length;
    return true;
}

bool block4_form_reader_t::decode(address_t from, size_t& offset)
{
    length = *reinterpret_cast<uint32_t*>(from + offset);
    offset += sizeof(uint32_t);
    data = reinterpret_cast<char*>(from + offset);
    offset += length;
    return true;
}

bool sdata_form_reader_t::decode(address_t from, size_t& offset)
{
    data.decode(from, offset);
    return true;
}

bool udata_form_reader_t::decode(address_t from, size_t& offset)
{
    data.decode(from, offset);
    return true;
}

bool data1_form_reader_t::decode(address_t from, size_t& offset)
{
    data = *reinterpret_cast<uint8_t*>(from + offset);
    offset += sizeof(uint8_t);
    return true;
}

bool data2_form_reader_t::decode(address_t from, size_t& offset)
{
    data = *reinterpret_cast<uint16_t*>(from + offset);
    offset += sizeof(uint16_t);
    return true;
}

bool data4_form_reader_t::decode(address_t from, size_t& offset)
{
    data = *reinterpret_cast<uint32_t*>(from + offset);
    offset += sizeof(uint32_t);
    return true;
}

bool data8_form_reader_t::decode(address_t from, size_t& offset)
{
    data = *reinterpret_cast<uint64_t*>(from + offset);
    offset += sizeof(uint64_t);
    return true;
}

bool flag_form_reader_t::decode(address_t from, size_t& offset)
{
    data = *reinterpret_cast<uint8_t*>(from + offset);
    offset += sizeof(uint8_t);
    return true;
}

bool string_form_reader_t::decode(address_t from, size_t& offset)
{
    data = reinterpret_cast<char*>(from + offset);
    char* d = reinterpret_cast<char*>(from + offset);
    while (*d++)
        ++offset;
    ++offset;
    return true;
}

bool strp_form_reader_t::decode(address_t from, size_t& offset)
{
    uint32_t stroff = *reinterpret_cast<uint32_t*>(from + offset);
    offset += sizeof(uint32_t);
    (void)stroff;
//     data = debug_str_sect.offset(stroff);
    return true;
}

bool ref1_form_reader_t::decode(address_t from, size_t& offset)
{
    data = *reinterpret_cast<uint8_t*>(from + offset);
    offset += sizeof(uint8_t);
    return true;
}

bool ref2_form_reader_t::decode(address_t from, size_t& offset)
{
    data = *reinterpret_cast<uint16_t*>(from + offset);
    offset += sizeof(uint16_t);
    return true;
}

bool ref4_form_reader_t::decode(address_t from, size_t& offset)
{
    data = *reinterpret_cast<uint32_t*>(from + offset);
    offset += sizeof(uint32_t);
    return true;
}

bool ref8_form_reader_t::decode(address_t from, size_t& offset)
{
    data = *reinterpret_cast<uint64_t*>(from + offset);
    offset += sizeof(uint64_t);
    return true;
}

bool ref_udata_form_reader_t::decode(address_t from, size_t& offset)
{
    data.decode(from, offset);
    return true;
}

bool ref_addr_form_reader_t::decode(address_t from, size_t& offset)
{
    data = *reinterpret_cast<uint32_t*>(from + offset);
    offset += sizeof(uint32_t);
    return true;
}

bool indirect_form_reader_t::decode(address_t from, size_t& offset)
{
    form.decode(from, offset);
    data = create(form);
    if (data)
    {
        data->decode(from, offset);
        return true;
    }
    return false;
}
