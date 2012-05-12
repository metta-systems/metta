//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2010 - 2012, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "dwarf_aranges.h"
#include "local_panic.h"
#include "dwarf_debug.h"

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

    unknown_data = *reinterpret_cast<uint32_t*>(from + offset);
    offset += 4;
}

void aranges_set_header_t::dump()
{
    DPRINT("aranges set header: unit-length %d bytes, version 0x%04x, debug-info-offset 0x%x, address size %d, segment size %d, unknown data %08x, entries count %d\n", unit_length, version, debug_info_offset, address_size, segment_size, unknown_data, (unit_length - 12)/8);
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
#if DWARF_DEBUG
    sh.dump();
#endif
//     ASSERT(sh.address_size == 4 && sh.segment_size == 0);
    while (offset < size)
    {
        ad.decode(from, offset);
        DPRINT("found range: 0x%08x - 0x%08x (%d bytes)\n", ad.start, ad.start + ad.length - 1, ad.length);
        if (ad.is_last())
        {
            if (offset < size)
            {
                sh.decode(from, offset);
#if DWARF_DEBUG
                sh.dump();
#endif
//                 ASSERT(sh.address_size == 4 && sh.segment_size == 0);
            }
            continue;
        }
        if ((ad.start <= target_pc) && (ad.start + ad.length > target_pc))
        {
            DPRINT(" range contains requested 0x%x\n", target_pc);
            info_offset = sh.debug_info_offset;
            return true;
        }
    }

    DPRINT(" requested 0x%x not contained in any range\n", target_pc);
    return false;
}
