#ifndef __KERNEL_MEMORY_H
#define __KERNEL_MEMORY_H
#include "stdint.h"
#include "bitmap.h"

struct virtual_addr{
    struct bitmap vaddr_bitmap;
    uint32_t vaddr_start;
};

enum pool_flags{
    PF_KERNEL = 1,
    PF_USER = 2
};

#define PG_P_1 0b1
#define PG_P_0 0b0
#define PG_RW_R 0b00
#define PG_RW_W 0b10
#define PG_US_S 0b000
#define PG_US_U 0b100

extern struct pool kernel_pool, user_pool;
void mem_init(void);
void* get_kernel_pages(uint32_t pg_cnt);
#endif
