//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2010, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "bootimage.h"
#include "default_console.h"
#include "fourcc.h"

enum kind_e
{
    kind_root_domain,
    kind_glue_code,
    kind_module
};

struct bootimage_header_t
{
    uint32_t magic;        //!< contains header magic value 'BIMG'
    uint32_t version;      //!< contains initfs format version, currently 1

    header_t()
        : magic(FourCC<'B','I','M','G'>::value)
        , version(1)
    {}
};

struct bootimage_rec_t
{
    uint16_t tag;
    uint16_t size;
};

struct bootimage_root_domain_t : public bootimage_rec_t
{
};

struct bootimage_glue_code_t : public bootimage_rec_t
{
};

struct bootimage_module_t : public bootimage_rec_t
{
};

union bootimage_info_t
{
    bootimage_rec_t*         rec;
    bootimage_root_domain_t* rootdom;
    bootimage_glue_code_t*   glue;
    bootimage_module_t*      module;
    char*                    generic;
};

bool bootimage_t::valid()
{
    return false;//start->magic == FourCC<'B','I','M','G'>::value and start->version == 1;
}

address_t bootimage_t::get_file(uint32_t num)
{
    if (num >= count())
        return 0;

    return 0;//(address_t)start + entries[num].location;
}

const char* bootimage_t::get_file_name(uint32_t num)
{
    if (num >= count())
        return 0;

    return 0;//(const char*)start + entries[num].name_offset;
}

uint32_t bootimage_t::get_file_size(uint32_t num)
{
    if (num >= count())
        return 0;

    return 0;//entries[num].size;
}

uint32_t bootimage_t::count()
{
    return 0;//start->count;
}

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
