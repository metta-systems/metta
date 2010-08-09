#include "bootimage.h"
#include "root_domain.h"
#include "panic.h"
#include "default_console.h"

root_domain_t::root_domain_t(bootimage_t& img)
    : ns(0, 0)
{
    bootimage_t::modinfo_t mi = img.find_root_domain(&ns);
    kconsole << "Root domain @ " << (unsigned)mi.start << ", size " << mi.size << " bytes." << endl;
    elf.parse(mi.start);
    if (!elf.is_valid())
        PANIC("invalid root_domain elf image");
    elf32::section_header_t* text = elf.section_header(".text");
    address_t entry = elf.get_entry_point();
    ptrdiff_t offset = mi.start - text->addr + text->offset;
    if (!elf.is_relocatable() && offset != 0)
        PANIC("non-relocatable root domain");
    kconsole << "Root domain relocated by 0x" << (unsigned)offset << endl;
    elf.relocate_to(mi.start);
    entry_point = entry + offset;
}

address_t root_domain_t::entry()
{
    return entry_point;
}
