#pragma once

#include "elf_loader.h"

/*!
 * Loads components from initramfs to predefined memory area and relocates them.
 */
class module_loader_t
{
public:
    module_loader_t();
    void *load_module(const char* name, elf_loader_t& module, const char* closure_name);

private:
    address_t modules_start;
    address_t last_available_address;
};
