#include "BlockDevice.h"
#include <uuid/uuid.h>

int make_fs(BlockDevice& dev, const char* label)
{
    superblock_t super;

    num_bytes = (num_bytes / sectorsize) * sectorsize;

    memset(&super, 0, sizeof(super));

    super.version = 1;
//     uint8_t   checksum[CHECKSUM_SIZE];  // [  4] block data checksum
    uuid_generate(super.fsid);
    super.block_offset = 0;             // which block this node is supposed to live in
    super.flags = 0;                    // [ 60] not related to validity, but matches generic header format for different trees.
    super.magic = MAGIC64BE('M','e','T','T','a','F','S','1');
    super.generation = 1;
//     uint64_t root;              // [ 84] location of "root of roots" tree
    super.total_bytes = num_bytes;
    super.bytes_used = 0;
    super.sector_size = sectorsize;
    super.node_size = nodesize;
    super.leaf_size = leafsize;
    super.checksum_type = CHECKSUM_TYPE_SHA1;
    super.root_level = 1;
//     dev_item_t dev_item;        // [123]
    strncpy(super.label, label, sizeof(super.label));
}

int main(int argc, char** argv)
{
    // mkfs deviceName <create 1 or 0>
    if (argc != 3)
        return 111;

    const char* fname = argv[1];
    int create = atoi(argv[2]);
    BlockDevice dev(fname, create);

    make_fs(dev, "test_fs");
}
