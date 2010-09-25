/*!
 * tagfs prototype.
 *
 * necessary structures:
 * - content storage
 * - metadata storage
 * - journal w/ transactions
 * - cow
 */

simplest possible:

superblock
+--file tables
+--tag tables

file table
+--hashtype
+--hash
+--taglinks
+--contents


use btrfs/reiserfs node allocation style: md at one side, content at other side, growing towards each other?

file node:
+--fs root this node belongs to (subvolume)
+--node key
+--file extents tree (content storage blocks allocation)
+--


generic tree structure similar to btrfs:

/*!
 * every tree block (node or leaf) starts with this header.
 */
class btree_block_header
{
    /*!
     * Data needed to verify the validity of the block.
     */
    uint8_t checksum[32];  // block data checksum
    uint8_t fsid[16];      // parent filesystem id
    blockno_t blockNumber; // which block this node is supposed to live in
    uint8_t level;         // btree level this block is at
    /*!
     * Phantom/misplaced writes detection.
     */
    uint64_t generation;
};

/*!
 *  A fixed sized tuple used to identify and sort items in a Btree. The key is broken up into 3 parts: objectid, type,
 *  and offset. The type field indicates how each of the other two fields should be used, and what to expect to find in
 *  the item
 */
class btree_key
{
    uint64_t objectId; //! object id is unique fs key for the file.
    uint8_t type;    //! Type of the file data this key describes (extents, meta etc) - a substream (resource fork) selector.
    uint64_t offset; //! The byte offset of start of the given type file data in the fs.
};

/*!
 * A variable sized structure stored in btree leaves. Items hold different types of data depending on key type.
 *
 * A leaf is full of items. offset and size tell us where to find
 * the item in the leaf (relative to the start of the data area)
 */
class btree_item : public btree_key
{
    uint32_t offset;
    uint32_t size;
};

/*!
 * All non-leaf blocks are nodes, they hold only keys and pointers to other blocks.
 */
class btree_key_ptr : public btree_key
{
    uint64_t blockptr;
    uint64_t generation;
};

A leaf block structure:
[block_header] [item0, item1....itemN] [free space] [dataN...data1, data0]

A node block structure:
[block header] [key_ptr0, key_ptr1...key_ptrN]

// extent tree holds data blocks for the file
struct file_extent_item
{
    uint64_t generation;
    uint64_t ram_bytes; // max uncompressed size of extent
    uint8_t compression;
    uint8_t encryption;
    uint8_t type; // either real extent or inline data

    uint64_t disk_bytenr;
    uint64_t disk_bytes;

    // Logical position and size of this extent into the file.
    uint64_t offset;
    uint64_t num_bytes;
};


struct fs_info
{
    fs_root* extents_root;
    fs_root* tags_root;
    fs_root* checksum_root;
};

struct fs_root
{
    fs_root_item root_item;
    btree_key root_key;
    fs_info* fs_info;
    uint64_t objectid;
    uint32_t sectorsize; // data allocations are done in sectorsize units
};
