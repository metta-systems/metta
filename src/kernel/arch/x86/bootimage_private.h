//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2011, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "fourcc.h"

//======================================================================================================================
// bootimage internal structures
//======================================================================================================================

namespace bootimage_n
{

enum kind_e
{
    kind_root_domain,
    kind_glue_code,
    kind_module,
    kind_namespace
};

struct header_t
{
    uint32_t magic;        //!< contains header magic value 'BIMG'
    uint32_t version;      //!< contains initfs format version, currently 1

    header_t()
        : magic(four_cc<'B','I','M','G'>::value)
        , version(1)
    {}
};

struct rec_t
{
    uint32_t tag;    // entry tag
    uint32_t length; // length of the whole entry
};

struct glue_code_t : public rec_t
{
    address_t text, data, bss;
    size_t text_size, data_size, bss_size;
};

struct namespace_t : public rec_t
{
    address_t address; // file offset! (actually from start of bootimage)
    size_t size;
    const char* name;
};

struct module_t : public namespace_t
{
    address_t local_namespace_offset;
};

#define SIZEOF_ONDISK_MODULE 24

struct root_domain_t : public module_t
{
    uintptr_t entry_point;
};

#define SIZEOF_ONDISK_ROOT_DOMAIN 28

struct namespace_entry_t
{
    int tag; // type discriminator
    union {
        const char* name;
        address_t name_off;
    };
    union {
        void* value;
        uintptr_t value_int;
    };
};

#define SIZEOF_ONDISK_NAMESPACE_ENTRY 12

union info_t
{
    rec_t*         rec;
    root_domain_t* rootdom;
    glue_code_t*   glue;
    module_t*      module;
    namespace_t*   module_namespace;
    char*          generic;
};

}

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
