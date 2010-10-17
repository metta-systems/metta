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
 * All addressing is made using 64-bit byte offsets, so the FS is totally block-size agnostic. (?)
 */

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
    uint64_t  block_offset;             // [ 52] which block this node is supposed to live in

    uint64_t  flags;                    // [ 60] not related to validity, but matches generic header format for different trees.
}; // 68 bytes

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
}; // 105 bytes

/*!
 * Filesystem superblock.
 */
class fs_superblock_t : public btree_header_common_t
{
public:
    uint64_t magic;             // [ 68] 'MeTTaFS1' in network (big-engian) order
    uint64_t generation;        // [ 76]
    uint64_t root;              // [ 84] location of "root of roots" tree

    uint64_t total_bytes;       // [ 92]
    uint64_t bytes_used;        // [100]
    uint32_t sector_size;       // [108] size of hardware sector (minimal read/write unit)
    uint32_t node_size;         // [112] size of node block (e.g. 4kb in default btrfs)
    uint32_t leaf_size;         // [116] size of leaf block (4kb in def btrfs, depends on avg size of tag infos, may be bigger?)
    uint16_t checksum_type;     // [120] how block checksumming is performed
    uint8_t  root_level;        // [122] start level of root node
    dev_item_t dev_item;        // [123]
    char label[256];            // [???] filesystem label
    // TODO: add root trees links? see "root" above.
}; // 379+ bytes

/*!
 * A fixed sized tuple used to identify and sort items in a Btree. The key is broken up into 3 parts: objectid, type,
 * and position. The type field indicates how each of the other two fields should be used, and what to expect to find in
 * the item.
 * The key's fields are fairly open to interpretation and depend on type heavily. For different types meaning of
 * two other fields may be radically different. Ordering of fields defines ordering of keys and therefore
 * important.
 */
class btree_key
{
public:
    uint64_t objectId; // [  0] object id is unique fs key for the file.
    uint8_t type;      // [  8] Type of the file data this key describes (extents, meta etc) - a substream (resource fork) selector.
    uint64_t position; // [  9] The byte offset of start of the given type file data in the fs.
}; // 17 bytes

/*!
 * A variable sized structure stored in btree leaves. Items hold different types of data depending on key type.
 *
 * A leaf block is full of items. offset and size tell us where to find
 * the item in the leaf (relative to the start of the block)
 */
class btree_item : public btree_key
{
public:
    uint32_t offset; // [ 17]
    uint32_t size;   // [ 21]
}; // 25 bytes

/*!
 * A leaf block structure:
 * [block_header] [item0, item1...itemN] [free space] [dataN...data1, data0]
 */
struct btree_leaf
{
    btree_block_header_t header;
    btree_item items[]; // header.numItems items
}; // sb.leaf_size bytes

/*!
 * All non-leaf blocks are nodes, they hold only keys and pointers to other blocks.
 */
class btree_key_ptr : public btree_key
{
public:
    uint64_t blockptr;    // [ 17] this is byte offset of start of the block
    uint64_t generation;  // [ 25]
}; // 33 bytes

/*!
 * A node block structure:
 * [block header] [key_ptr0, key_ptr1...key_ptrN]
 */
struct btree_node
{
    btree_block_header_t header;
    btree_key_ptr ptrs[]; // header.numItems keys
}; // sb.node_size bytes

// extent tree holds data blocks for the file
struct file_extent_item
{
    uint64_t generation; // [  0]
    uint64_t ram_bytes;  // [  8] max uncompressed size of extent
    uint8_t compression; // [ 16]
    uint8_t encryption;  // [ 17]
    uint8_t type;        // [ 18] either real extent or inline data

    uint64_t disk_bytenr; // [ 19]
    uint64_t disk_bytes;  // [ 27]

    // Logical position and size of this extent into the file.
    uint64_t offset;      // [ 35]
    uint64_t num_bytes;   // [ 43]
}; // 51 bytes

// **FIXME** Stuff from btrfs:

struct fs_info
{
    fs_root* extents_root;
    fs_root* tags_root;
    fs_root* checksum_root;
};

struct fs_root
{
    btree_key root_key;
    fs_info* fs_info;
    fs_root_item root_item;
    uint32_t sectorsize; // data allocations are done in sectorsize units
    uint64_t objectid;
};

// Files have many-to-many relation with tags.

/*
 * fs_paths remember the path taken from the root down to the leaf.
 * level 0 is always the leaf, and nodes[1...BTRFS_MAX_LEVEL] will point
 * to any other levels that are present.
 *
 * The slots array records the index of the item or block pointer
 * used while walking the tree.
 *
 * The whole fs_paths structure is used to find, insert and delete nodes.
 */
static const int FS_MAX_LEVEL = 8;

class fs_path
{
public:
    buffer* nodes[FS_MAX_LEVEL];
    int slots[FS_MAX_LEVEL];
    // We ignore locking and other mumbo-jumbo atm.
};

// A lazy-load buffer for extent contents.
class extent_buffer
{
public:
    extent_buffer(uint64_t block_offset, size64_t size);
    operator []() { will load contents on demand, for example in PAGE_SIZE chunks etc. }
    bool access(uint64_t start, size = PAGE_SIZE);// actual loader
};


//======================================================================================================================

/*
 * look for key in the tree.  path is filled in with nodes along the way
 * if key is found, we return zero and you can find the item in the leaf
 * level of the path (level 0)
 *
 * If the key isn't found, the path points to the slot where it should
 * be inserted, and 1 is returned.  If there are other errors during the
 * search a negative error number is returned.
 *
 * if ins_len > 0, nodes and leaves will be split as we walk down the
 * tree.  if ins_len < 0, nodes will be merged as we walk down the tree (if
 * possible)
 */
int btrfs_search_slot(struct btrfs_trans_handle *trans, struct btrfs_root
*root, struct btrfs_key *key, struct btrfs_path *p, int
ins_len, int cow)

(ctree.c:1699)


block = root_block;
path->nodes[block->level] = block;
bin_search(block, key, slot);
path->slots[block->level] = slot;
