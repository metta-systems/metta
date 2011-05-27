#include "block_device.h"
#include <uuid/uuid.h>
#include "superblock.h"
#include "memutils.h"
#include "fourcc.h"
#include "macros.h"

const int sectorsize = 4096;
const int nodesize = 4096;
const int leafsize = 4096;

int make_fs(block_device_t& dev, size_t num_bytes, const char* label)
{
    fs_superblock_t super;

    num_bytes = (num_bytes / sectorsize) * sectorsize;

    memutils::fill_memory(&super, 0, sizeof(super));

    super.version = 1;
//     uint8_t   checksum[CHECKSUM_SIZE];  // [  4] block data checksum
    uuid_generate(super.fsid);
    super.block_offset = 0;             // which block this node is supposed to live in
    super.flags = 0;                    // [ 60] not related to validity, but matches generic header format for different trees.
    super.magic = Magic64BE<'M','e','T','T','a','F','S','1'>::value;
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
    memutils::copy_string(super.label, label, sizeof(super.label));

    UNUSED(dev);

    return 1;
}

int main(int argc, char** argv)
{
    // mkfs deviceName <create 1 or 0> <byte size>
    if (argc != 4)
        return 111;

    const char* fname = argv[1];
    int create = atoi(argv[2]);
    size_t size = atoi(argv[3]);
    block_device_t dev(fname, create);

    make_fs(dev, size, "test_fs");
}
