#pragma once

#include "elf_parser.h"

class bootinfo_t;

/*!
 * Loads components from initramfs to predefined memory area and relocates them.
 * Constructor receives addresses of last_available_address field from
 * inside bootinfo page.
 * @internal This class shall be used only internally during bootup.
 */
class module_loader_t
{
public:
    module_loader_t(bootinfo_t* _parent, address_t* _last_available_address)
        : parent(_parent)
        , last_available_address(_last_available_address)
    {}

    /*!
     * @returns pointer to given closure.
     */
    void* load_module(const char* name, elf_parser_t& module, const char* closure_name);

private:
    bootinfo_t* parent;
    address_t*  last_available_address;
};
