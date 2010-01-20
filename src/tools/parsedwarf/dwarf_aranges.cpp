#include "dwarf_aranges.h"
#include "local_panic.h"
#include <stdio.h>

void aranges_set_header_t::decode(address_t from, size_t& offset)
{
    unit_length = *reinterpret_cast<uint32_t*>(from + offset);
    if (unit_length == 0xffffffff)
        PANIC("DWARF64 is not supported!");
    offset += sizeof(uint32_t);
    version = *reinterpret_cast<uint16_t*>(from + offset);
    offset += sizeof(uint16_t);
    debug_info_offset = *reinterpret_cast<uint32_t*>(from + offset);
    offset += sizeof(uint32_t);
    address_size = *reinterpret_cast<uint8_t*>(from + offset);
    offset += sizeof(uint8_t);
    segment_size = *reinterpret_cast<uint8_t*>(from + offset);
    offset += sizeof(uint8_t);

    offset += 4; // This field is not described in the DWARF3 specification!!!
}

void arange_desc_t::decode(address_t from, size_t& offset)
{
    start = *reinterpret_cast<uint32_t*>(from + offset);
    offset += sizeof(uint32_t);
    length = *reinterpret_cast<uint32_t*>(from + offset);
    offset += sizeof(uint32_t);
}

bool dwarf_debug_aranges_t::lookup(address_t target_pc, size_t& info_offset)
{
    address_t from = start;
    size_t offset = 0;

    aranges_set_header_t sh;
    arange_desc_t ad;

    sh.decode(from, offset);
    printf("decoded set header: unit-length %d, version 0x%04x, debug-info-offset 0x%x\n", sh.unit_length, sh.version, sh.debug_info_offset);
    while (offset < size)
    {
        ad.decode(from, offset);
        if (ad.start == 0 && ad.length == 0 && offset < size)
        {
            sh.decode(from, offset);
            printf("decoded set header: unit-length %d, version 0x%04x, debug-info-offset 0x%x\n", sh.unit_length, sh.version, sh.debug_info_offset);
            continue;
        }
        printf("found range: 0x%x - 0x%x (%d bytes)\n", ad.start, ad.start + ad.length - 1, ad.length);
        if (ad.start <= target_pc && (ad.start + ad.length) > target_pc)
        {
            info_offset = sh.debug_info_offset;
            return true;
        }
    }

    return false;
}
