//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2010, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "types.h"
#include "fourcc.h"

/*!
Bootimage is similar to Nemesis' nexus - it contains information, modules, dependencies, namespaces etc.
required for successful bootstrap.

Index information in bootimage should allow quick and easy dependency calculation and module instantiation.
Modules can be ELF executables or data blobs loaded and mapped at specified address in memory.

Deps are lists of items from common stringtable. (ofs,len) pairs for ndeps count.

Root entry in bootimage is main startup code, the system privileged domain or root domain.

<data blob>
address
size
name ofs
<module>
address
size
name ofs
upcall record (PCB) location
dependencies list (ndeps * name ofs entries)

*/

/*!

// version 3: current.
              * initfs::header
              * aligned: modules data
              * aligned: nexus with metadata index in tagged format.
 */

class bootimage_t
{
public:
    explicit initfs_t(address_t start);
//     address_t get_file(string spec);

    /*! Silly iterator interface
     * TODO: replace with normal iterator.
     */
    address_t   get_file(uint32_t num);
    const char* get_file_name(uint32_t num);
    uint32_t    get_file_size(uint32_t num);
    uint32_t    count();

    bool valid();

    struct header_t
    {
        uint32_t magic;        //!< contains header magic value 'IifS'
        uint32_t version;      //!< contains initfs format version, currently 3
        uint32_t index_offset; //!< offset of index entries
        uint32_t names_offset; //!< offset of names area
        uint32_t names_size;   //!< size of names area (unaligned)
        uint32_t count;        //!< count of index entries

        header_t()
            : magic(FOURCC_MAGIC('I','i','f','S'))
            , version(3)
            , index_offset(0)
            , names_offset(0)
            , names_size(0)
            , count(0)
        {}
    };

    enum tags_e { MODULE = 1, BLOB = 2 };

    struct entry_t
    {
        uint32_t magic;
        uint32_t tag;
        uint32_t entry_size;
        uint32_t name_offset;
        uint32_t location;
        uint32_t size;
        // here can be extra fields

        entry_t(uint32_t name_offset_ = 0, uint32_t location_ = 0, uint32_t size_ = 0)
            : magic(FOURCC_MAGIC('F','E','n','t'))
            , name_offset(name_offset_)
            , location(location_)
            , size(size_)
        {}
    };

    struct module_entry_t : public entry_t
    {
        uint32_t pcb_offset;
        uint32_t ndeps;
    };

private:
    header_t* start;
    entry_t*  entries;
};

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
