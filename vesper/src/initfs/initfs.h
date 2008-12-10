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
