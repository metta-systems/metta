#pragma once

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

    bootimage_header_t()
        : magic(FourCC<'B','I','M','G'>::value)
        , version(1)
    {}
};

struct bootimage_rec_t
{
    uint32_t tag;
    uint32_t size;
};

struct bootimage_root_domain_t : public bootimage_rec_t
{
    uintptr_t entry_point;
    uintptr_t namespace_data;
};

struct bootimage_glue_code_t : public bootimage_rec_t
{
    address_t text, data, bss;
    size_t text_size, data_size, bss_size;
};

struct bootimage_module_t : public bootimage_rec_t
{
    address_t address;
    size_t size;
};

union bootimage_info_t
{
    bootimage_rec_t*         rec;
    bootimage_root_domain_t* rootdom;
    bootimage_glue_code_t*   glue;
    bootimage_module_t*      module;
    char*                    generic;
};
