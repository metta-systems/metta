#include "multiboot.h"

multiboot_t mb;
address_t multiboot_info;
address_t multiboot_flags;

multiboot_t* multiboot_t::prepare()
{
    if (multiboot_flags == 0x2BADB002)
    {
        mb.set_header(reinterpret_cast<multiboot_t::header_t*>(multiboot_info));
        return &mb;
    }
    return 0;
}

