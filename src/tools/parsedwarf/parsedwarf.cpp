/*!
 * Example code to parse DWARF2/3 debug info from an ELF format file.
 */
#include <stdlib.h>
#include "../mkinitfs/raiifile.h"
#include "elf_parser.h"
#include "leb128.h"
#include "datarepr.h"

#define PANIC(s) panic(s)

void panic(const char* s)
{
    printf("%s\n", s);
    exit(-1);
}

using namespace raii_wrapper;
using namespace elf32; // FIXME: only elf32 is supported, will fail on x86-64

class abbrev_attr_t
{
    uleb128_t name;
    uleb128_t form; //DW_FORM_*
};

// DWARF 
class die_t //abbreviation_entry only?
{
    uleb128_t abbreviation_entry;
    uleb128_t tag;         //DW_TAG_*
    uint8_t   has_children;//DW_CHILDREN_*
    abbrev_attr_t* attributes;
};

class subprogram_die_t : public die_t
{
};

/* Compilation Unit Header */
/* Resides in: .debug_info */
/* Referenced from: .debug_aranges */
class cuh_t
{
public:
    uint32_t unit_length;
    uint16_t version;
    uint32_t debug_abbrev_offset;
    uint8_t  address_size;

    void decode(address_t from, size_t& offset)
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
};

class dwarf_debug_info_t
{
    address_t start;
    size_t    size;

public:
    dwarf_debug_info_t(address_t st, size_t sz)
        : start(st)
        , size(sz)
    {
    }

    cuh_t get_cuh(size_t& offset)
    {
        cuh_t cuh;
        cuh.decode(start + offset, offset);
        return cuh;
    }
};

class aranges_set_header_t
{
public:
    uint32_t unit_length;
    uint16_t version;
    uint32_t debug_info_offset;
    uint8_t  address_size;
    uint8_t  segment_size;

    void decode(address_t from, size_t& offset)
    {
        unit_length = *reinterpret_cast<uint32_t*>(from + offset);
        offset += sizeof(uint32_t);
        version = *reinterpret_cast<uint16_t*>(from + offset);
        offset += sizeof(uint16_t);
        debug_info_offset = *reinterpret_cast<uint32_t*>(from + offset);
        offset += sizeof(uint32_t);
        address_size = *reinterpret_cast<uint8_t*>(from + offset);
        offset += sizeof(uint8_t);
        segment_size = *reinterpret_cast<uint8_t*>(from + offset);
        offset += sizeof(uint8_t);
    }
};

class arange_desc_t
{
public:
    uint32_t start;
    uint32_t length;
//     uleb128_t start;
//     uleb128_t length;

    void decode(address_t from, size_t& offset)
    {
        start = *reinterpret_cast<uint32_t*>(from + offset);
        offset += sizeof(uint32_t);
        length = *reinterpret_cast<uint32_t*>(from + offset);
        offset += sizeof(uint32_t);
//         start.decode(from, offset);
//         length.decode(from, offset);
    }
};

class dwarf_debug_aranges_t
{
    address_t start;
    size_t    size;

public:
    dwarf_debug_aranges_t(address_t st, size_t sz)
        : start(st)
        , size(sz)
    {
    }

    bool lookup(address_t target_pc, size_t& info_offset)
    {
        address_t from = start;
        size_t offset = 0;

        aranges_set_header_t sh;
        arange_desc_t ad;

        sh.decode(from, offset);
        printf("decoded set header: unit-length %d, version %04x, debug-info-offset %d\n", sh.unit_length, sh.version, sh.debug_info_offset);
        while (offset < size)
        {
            ad.decode(from, offset);
            printf("found range: 0x%x - 0x%x (%d bytes)\n", ad.start, ad.start + ad.length - 1, ad.length);
            if (ad.start == 0 && ad.length == 0 && offset < size)
            {
                sh.decode(from, offset);
                printf("decoded set header: unit-length %d, version %04x\n", sh.unit_length, sh.version);
                continue;
            }
            if (ad.start <= target_pc && (ad.start + ad.length) > target_pc)
            {
                info_offset = sh.debug_info_offset;
                return true;
            }
        }

        return false;
    }
};

// Use .debug_aranges to find functions by address.
// Use .debug_frame or .eh_frame to unwind stack. <- not needed for skype

// Entries in a .debug_frame section are aligned on an addressing unit boundary and come in two forms: A Common Information Entry (CIE) and a Frame Description Entry (FDE).

/* Resides in: .debug_frame, .eh_frame */
class cie_t
{
    uint32_t length;
    uint32_t CIE_id;
    uint8_t  version;
    char*    augmentation;
    uleb128_t code_alignment_factor;
    sleb128_t data_alignment_factor;
    uleb128_t return_address_register;
    uint8_t*  initial_instructions;

    void decode(address_t from, size_t& offset);
};

/* Resides in: .debug_frame, .eh_frame */
class fde_t
{
    uint32_t length;
    uint32_t CIE_pointer;
    uint32_t initial_location;
    uint32_t address_range;
    uint8_t* instructions;
};

/*!
 * Usage: parsedwarf elf_with_dwarf_debug_info 0xaddress
 */
int main(int argc, char** argv)
{
    if (argc < 3)
        return 111;

    address_t addr = strtoul(argv[2], NULL, 0);
    file f(argv[1], fstream::in);
    size_t fsize = f.size();
    char* buffer = new char [fsize];
    f.read(buffer, fsize);
    address_t start = reinterpret_cast<address_t>(buffer);
    elf_parser_t elf(start);
    section_header_t* h = elf.section_header(".debug_aranges");
    section_header_t* g = elf.section_header(".debug_info");
    if (h && g)
    {
        dwarf_debug_info_t    debug_info(start + g->offset, g->size);
        size_t offset = 0;
        dwarf_debug_aranges_t debug_aranges(start + h->offset, h->size);
        // find addr in debug info and figure function name and source file line
        if (debug_aranges.lookup(addr, offset))
        {
            printf("FOUND ADDRESS\n");
            cuh_t cuh;
            cuh = debug_info.get_cuh(offset);
            printf("decoded cuh: unit-length %d, version %04x\n", cuh.unit_length, cuh.version);
            // print output data:
            // [TID] 0xfault_addr func-name (source:line)
        }
        else
            printf("NOT FOUND\n");
    }
    else
        printf("No .debug_aranges\n");

    return 0;
}

