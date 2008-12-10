//
// Copyright 2007 - 2008, Stanislav Karchebnyy <berkus+metta@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

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
};

struct initfs_entry
{
	uint32_t magic;
	uint32_t name_offset;
	uint32_t location;
	uint32_t size;
};

struct initfs_index
{
	uint32_t magic;
	uint32_t count;
};

// kate: indent-width 4; replace-tabs on;
// vi:set ts=4:set expandtab=on:
