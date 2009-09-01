#pragma once

#include "multiboot.h"

/*!
* Provide OO access to boot info page structures.
*/
class bootinfo_t
{
public:
    bootinfo_t(address_t bootinfo_page) : boot_info(bootinfo_page) {}
    size_t size();
    multiboot_t::header_t* multiboot_header();
private:
    address_t boot_info;
};

inline size_t bootinfo_t::size()
{
    return *reinterpret_cast<uint32_t*>(boot_info);
}

inline multiboot_t::header_t* bootinfo_t::multiboot_header()
{
    return reinterpret_cast<multiboot_t::header_t*>(boot_info + 4);
}
