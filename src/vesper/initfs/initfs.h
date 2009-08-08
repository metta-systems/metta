//
// Copyright 2007 - 2009, Stanislav Karchebnyy <berkus+metta@madfire.net>
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

// Initfs file layout:
// version 1: uses separate index structure, not in production.
// version 2: current.
// initfs::header
// files data
// aligned: names area
// aligned: initfs::entry * count

/*!
initfs file consists of four areas: the header, data area, names area and the index.

the header contains information about loadable areas - data, names and index.
*/
class initfs
{
public:
    explicit initfs(address_t start);
//     address_t get_file(string spec);

    /*! Silly iterator interface
     * TODO: replace with normal iterator.
     */
    address_t get_file(uint32_t num);
    const char* get_file_name(uint32_t num);
    uint32_t count();

    bool valid();

    struct header
    {
        uint32_t magic;        //!< contains header magic value 'IifS'
        uint32_t version;      //!< contains initfs format version, currently 2
        uint32_t index_offset; //!< offset of index entries
        uint32_t names_offset; //!< offset of names area
        uint32_t names_size;   //!< size of names area (unaligned)
        uint32_t count;        //!< count of index entries

        header()
            : magic(FOURCC_MAGIC('I','i','f','S'))
            , version(2)
            , index_offset(0)
            , names_offset(0)
            , names_size(0)
            , count(0)
        {}
    };

    struct entry
    {
        uint32_t magic;
        uint32_t name_offset;
        uint32_t location;
        uint32_t size;

        entry(uint32_t name_offset_ = 0, uint32_t location_ = 0, uint32_t size_ = 0)
            : magic(FOURCC_MAGIC('F','E','n','t'))
            , name_offset(name_offset_)
            , location(location_)
            , size(size_)
        {}
    };

private:
    header* start;
    entry*  entries;
};

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
