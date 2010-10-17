#pragma once

#include "types.h"

struct fs_superblock_t
{
    uint64_t root; // byte offset of root tree
    uint64_t tag_root; // byte offset of tags root tree
};
