#include "bootimage.h"
#include "root_domain.h"
#include "panic.h"
#include "default_console.h"

root_domain_t::root_domain_t(bootimage_t& img)
{
    start = img.find_root_domain(&size);
    elf.parse(start);
    elf32::section_header_t* text = elf.section_header(".text");
    address_t entry = elf.get_entry_point();
    ptrdiff_t offset = start - text->addr + text->offset;
    if (!elf.is_relocatable() && offset != 0)
        PANIC("non-relocatable root domain, cannot proceed.");
    kconsole << "Root domain relocated by 0x" << (unsigned)offset << endl;
    elf.relocate_to(start);
    entry_point = entry + offset;
}

address_t root_domain_t::entry()
{
    return entry_point;
}
