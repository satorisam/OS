#ifndef __FS_INODE_H
#define __FS_INODE_H
#include "stdint.h"
#include "stdbool.h"

struct inode{
    uint32_t i_no;
    uint32_t i_size;

    uint32_t i_open_cnts;
    bool write_deny;

    uint32_t i_sectors[13];
    struct list_elem inode_tag;
};

#endif