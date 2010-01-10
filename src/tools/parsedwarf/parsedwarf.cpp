/*!
 * Example code to parse DWARF2/3 debug info from an ELF format file.
 */
#include <stdlib.h>
#include "../mkinitfs/raiifile.h"
#include "elf_parser.h"
#include "leb128.h"
#include "datarepr.h"

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

class dwarf_debug_t
{
    address_t start;
    size_t    size;

public:
    dwarf_debug_t(address_t st, size_t sz)
        : start(st)
        , size(sz)
    {
    }
};

// Use .debug_aranges to find functions by address.
// Use .debug_frame or .eh_frame to unwind stack. <- not needed for skype

// Entries in a .debug_frame section are aligned on an addressing unit boundary and come in two forms: A Common Information Entry (CIE) and a Frame Description Entry (FDE).


/* Compilation Unit Header */
class cuh_t
{
    uint32_t length;
    uint16_t version;
    uint32_t debug_abbrev_offset;
    uint8_t  address_size;
};

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
};

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

//     address_t addr = strtoul(argv[2], NULL, 0);
    file f(argv[1], fstream::in);
    filebinio fio(f);
    size_t fsize = f.size();
    char* buffer = new char [fsize];
    f.read(buffer, fsize);
    address_t start = reinterpret_cast<address_t>(buffer);
    elf_parser_t elf(start);
    section_header_t* h = elf.section_header(".debug_info");
    if (h)
    {
        dwarf_debug_t debug_info(start + h->offset, h->size);
        // find addr in debug info and figure function name and source file line
//         if (debug_info.find_address(addr, info))
        {
            // print output data:
            // [TID] 0xfault_addr func-name (source:line)
        }
    }

    return 0;
}

