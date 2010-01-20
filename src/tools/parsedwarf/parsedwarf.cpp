/*!
 * Example code to parse DWARF2/3 debug info from an ELF format file.
 */
#include <stdlib.h>
#include "raiifile.h"
#include "elf_parser.h"
#include "leb128.h"
#include "dwarf_parser.h"
#include "dwarf_aranges.h"
#include "dwarf_abbrev.h"
#include "dwarf_info.h"

void panic(const char* s)
{
    printf("%s\n", s);
    exit(-1);
}

using namespace raii_wrapper;
using namespace elf32; // FIXME: only elf32 is supported, will fail on x86-64

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
    dwarf_parser_t dwarf(elf);

    dwarf.lookup(addr);

    return 0;
}

