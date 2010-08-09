#pragma once

#include "fourcc.h"

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
        : magic(FourCC<'B','I','M','G'>::value)
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

struct root_domain_t : public module_t
{
    uintptr_t entry_point;
};

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
