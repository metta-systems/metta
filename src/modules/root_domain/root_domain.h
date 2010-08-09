#pragma once

#include "elf_parser.h"
#include "bootimage.h"

class component_t
{
};

class root_domain_t : public component_t
{
public:
    root_domain_t(bootimage_t& img);
    address_t entry();
    module_namespace_t get_namespace() const { return ns; }

private:
    address_t start;
    size_t size;
    elf_parser_t elf;
    address_t entry_point;
    module_namespace_t ns;
};
