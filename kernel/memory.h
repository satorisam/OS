#ifndef __KERNEL_MEMORY_H
#define __KERNEL_MEMORY_H
#include "../lib/stdint.h"
#include "list.h"
#include "bitmap.h"

struct virtual_addr{
    struct bitmap vaddr_bitmap;
    uint32_t vaddr_start;
};

enum pool_flags{
    PF_KERNEL = 1,
    PF_USER = 2
};

struct mem_block{
    struct list_elem free_elem;
};

struct mem_block_desc{
    uint32_t block_size;
    uint32_t blocks_per_arena;
    struct list free_list;
};

#define DESC_CNT 7

#define PG_P_1 0b1
#define PG_P_0 0b0
#define PG_RW_R 0b00
#define PG_RW_W 0b10
#define PG_US_S 0b000
#define PG_US_U 0b100

extern struct pool kernel_pool, user_pool;
void mem_init(void);
void block_desc_init(struct mem_block_desc* desc_array);
void* sys_malloc(uint32_t size);
void* get_kernel_pages(uint32_t pg_cnt);
void* get_user_pages(uint32_t pg_cnt);
void* get_a_page(enum pool_flags pf,uint32_t vaddr);
uint32_t addr_v2p(uint32_t vaddr);
#endif
