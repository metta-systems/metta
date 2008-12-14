//
// Copyright 2007 - 2008, Stanislav Karchebnyy <berkus+metta@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#if __BYTE_ORDER == __LITTLE_ENDIAN
#define FOURCC_MAGIC(w,o,r,d) (((unsigned long)d << 24) | ((unsigned long)r << 16) | \
((unsigned long)o << 8)  |  (unsigned long)w)
#else
#define FOURCC_MAGIC(w,o,r,d) (((unsigned long)w << 24) | ((unsigned long)o << 16) | \
((unsigned long)r << 8)  |  (unsigned long)d)
#endif

// Initfs file layout:
// initfs_header
// files data
// page aligned
// names area
// page aligned
// initfs_index
// initfs_entry * count

struct initfs_header
{
    uint32_t magic;
    uint32_t index_offset;

    initfs_header() : magic(FOURCC_MAGIC('I','i','f','S')), index_offset(0) {}
};

struct initfs_entry
{
    uint32_t magic;
    uint32_t name_offset;
    uint32_t location;
    uint32_t size;

    initfs_entry() : magic(FOURCC_MAGIC('F','E','n','t')), name_offset(0), location(0), size(0) {}
};

struct initfs_index
{
    uint32_t magic;
    uint32_t count;

    initfs_index() : magic(FOURCC_MAGIC('f','I','d','X')), count(0) {}
};

// kate: indent-width 4; replace-tabs on;
// vi:set ts=4:set expandtab=on:
