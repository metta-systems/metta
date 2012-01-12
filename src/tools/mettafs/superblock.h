//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2011, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
/*!
 * Mettafs simple prototype.
 *
 * necessary structures:
 * - superblock
 * - file tree (objectids)
 * - metadata (tags) tree
 *
 * file tree keeps object nodes, extents. simply oid -> extents mapping.
 * tag tree lists all tags and keeps corresponding objectids on a sub-tree. a mapping is from a tag name to list of all
 * oids that have this tag. this makes exact tag matching easier for the prototype, also adding or removing tag is
 * a simple btree(?) operation. oids list is radix sorted. union and intersection of several tags is also easy to do.
 *
 * All addressing is made using 64-bit byte offsets, so the FS is totally block-size agnostic.
 */

#pragma once

#include "types.h"
#include "macros.h"

typedef uint64_t fs_location_t; // All addressing is made using 64-bit byte offsets, so the FS is totally block-size agnostic.

/*!
 * Structure common to several block headers in FS, including superblock and node/leaf block headers.
 */
struct btree_header_common_t
{
    static const int CHECKSUM_SIZE = 32;
    static const int FS_UUID_SIZE = 16;
    static const int TREE_UUID_SIZE = 16;
    /*!
     * Data needed to verify the validity of the block.
     */
    uint32_t  version;                  // [  0] block format version (for fs live migration)
    uint8_t   checksum[CHECKSUM_SIZE];  // [  4] block data checksum
    uint8_t   fsid[FS_UUID_SIZE];       // [ 36] parent filesystem id
    fs_location_t  block_offset;        // [ 52] which block this node is supposed to live in

    uint64_t  flags;                    // [ 60] not related to validity, but matches generic header format for different trees.
} PACKED; // 68 bytes

/*!
 * every tree block (node or leaf) starts with this header.
 */
class btree_block_header_t : public btree_header_common_t
{
public:
    uint8_t level;          // [ 68] btree level this block is at
//     uint8_t align[3];
    /*!
     * Phantom/misplaced writes detection.
     */
    uint64_t generation;                     // [ 69]

    uint8_t chunk_tree_uuid[TREE_UUID_SIZE]; // [ 77]
    uint64_t owner;                          // [ 93]
    uint32_t numItems;                       // [101]
} PACKED; // 105 bytes

/*!
 * Filesystem superblock.
 */
class fs_superblock_t : public btree_header_common_t
{
public:
    uint64_t magic;             // [ 68] 'MeTTaFS1' in network (big-engian) order
    uint64_t generation;        // [ 76]
    fs_location_t root;         // [ 84] location of "root of roots" tree

    uint64_t total_bytes;       // [ 92]
    uint64_t bytes_used;        // [100]
    uint32_t sector_size;       // [108] size of hardware sector (minimal read/write unit)
    uint32_t node_size;         // [112] size of node block (e.g. 4kb in default btrfs)
    uint32_t leaf_size;         // [116] size of leaf block (4kb in def btrfs, depends on avg size of tag infos, may be bigger?)
    uint16_t checksum_type;     // [120] how block checksumming is performed
    uint8_t  root_level;        // [122] start level of root node
//     dev_item_t dev_item;        // [123]
    char label[256];            // [???] filesystem label
    // TODO: add root trees links? see "root" above.
} PACKED; // 379+ bytes

static const uint16_t CHECKSUM_TYPE_SHA1 = 1;

struct fs_ondisk_key_t {
	uint64_t objectid;
	uint8_t type;
	uint64_t offset;
} PACKED;

struct fs_key_t {
	uint64_t objectid;
	uint8_t type;
	uint64_t offset;
} PACKED;


/*
 * A leaf is full of items. offset and size tell us where to find
 * the item in the leaf (relative to the start of the data area)
 */
struct fs_item_t {
	fs_ondisk_key_t key;
	uint32_t offset;
	uint32_t size;
} PACKED;

/*
 * leaves have an item area and a data area:
 * [item0, item1....itemN] [free space] [dataN...data1, data0]
 *
 * The data is separate from the items to get the keys closer together
 * during searches.
 */
struct fs_leaf_t : public btree_block_header_t
{
    fs_item_t items[];
} PACKED;

/*
 * all non-leaf blocks are nodes, they hold only keys and pointers to
 * other blocks
 */
struct fs_key_ptr_t {
	fs_ondisk_key_t key;
	uint64_t blockptr;
	uint64_t generation;
} PACKED;

struct fs_node_t : public btree_block_header_t {
	fs_key_ptr_t ptrs[];
} PACKED;

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
