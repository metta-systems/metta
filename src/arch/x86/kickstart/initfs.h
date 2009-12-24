//
// Copyright 2007 - 2009, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "types.h"

// FIXME: __BYTE_ORDER needs <endian.h> on Linux
#if __BYTE_ORDER == __LITTLE_ENDIAN
#define FOURCC_MAGIC(w,o,r,d) (((unsigned long)d << 24) | ((unsigned long)r << 16) | \
((unsigned long)o << 8)  |  (unsigned long)w)
#else
#define FOURCC_MAGIC(w,o,r,d) (((unsigned long)w << 24) | ((unsigned long)o << 16) | \
((unsigned long)r << 8)  |  (unsigned long)d)
#endif

/*!

Dictionary (plist) storage format: shared stringtable, entry struct: (ofs,len), tags table: array of (type,entry) structs -
type describes the type of tag, associated entry stores tag metadata.
plist: array of (tag,entry) - where tag is "key"- index into tags table, entry is value, data at ofs of len bytes.

*/
namespace proplists {
class entry_t
{
    address_t offset;
    size_t    size;
};
class tag_t
{
    uint32_t type;
    entry_t  meta;
};
class property_t
{
    tag_t*   tag;
    entry_t  value;
};

class proplist_t
{
public:
    proplist_t(address_t start);

private:
    address_t   base;
    tag_t*      tags;
    property_t* contents;
};
}

//in initfs
enum tags_types { INITFS_MODULE, INITFS_BLOB, INITFS_DEPS };

/*!

Initfs contains modules, with dependency info, used to boot up the system.
Index information in initfs should allow quick and easy dependency calculation and module instantiation.
Modules can be ELF executables or data blobs loaded and mapped at specified address in memory.

deps are lists of items from common stringtable. (ofs,len) pairs for ndeps count.


initfs nexus is a dictionary of initfs contents. initfs header contains a pointer to the nexus.


<data blob>
address
size
name
<module>
address
size
name
dependencies list
upcall record (PCB) location


--------
example initfs:
module interrupts, depends:cpu,memory, upcall@6789xx
module cpu, depends:none, upcall@1234xx
module memory, depends:mmu, upcall@3456xx
module mmu, depends:none, upcall@4567xx
module keyboard, depends:keymaps, upcall@1278xx
blob keymaps, @3333, size 2222
tags table:
tag 1: MODULE, meta: ?
tag 2: BLOB, meta: ?
--------


* initfs file consists of four areas: the header, data area, names area and the index.
 *
 * the header contains information about loadable areas - data, names and index.
// Initfs file layout:
// version 1: uses separate index structure, not in production.
// version 2: previous format.
//            * initfs::header
//            * files data
//            * aligned: names area
//            * aligned: index of initfs::entry * count
// version 3: current.
              * initfs::header
              * aligned: modules data
              * aligned: nexus
 */
class initfs_t
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
        uint32_t version;      //!< contains initfs format version, currently 2
        uint32_t index_offset; //!< offset of index entries
        uint32_t names_offset; //!< offset of names area
        uint32_t names_size;   //!< size of names area (unaligned)
        uint32_t count;        //!< count of index entries

        header_t()
            : magic(FOURCC_MAGIC('I','i','f','S'))
            , version(2)
            , index_offset(0)
            , names_offset(0)
            , names_size(0)
            , count(0)
        {}
    };

    struct entry_t
    {
        uint32_t magic;
        uint32_t name_offset;
        uint32_t location;
        uint32_t size;

        entry_t(uint32_t name_offset_ = 0, uint32_t location_ = 0, uint32_t size_ = 0)
            : magic(FOURCC_MAGIC('F','E','n','t'))
            , name_offset(name_offset_)
            , location(location_)
            , size(size_)
        {}
    };

private:
    header_t* start;
    entry_t*  entries;
};

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
