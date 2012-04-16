//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2011, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
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
    // module_namespace_t get_namespace() const { return ns; }

private:
    address_t entry_point;
    // module_namespace_t ns;
};

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
