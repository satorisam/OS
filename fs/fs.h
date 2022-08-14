#ifndef __FS_FS_H
#define __FS_FS_H
#include "stdint.h"

#define MAX_FILES_PER_PART 4096

#define BITS_PER_SECTOR 4096
#define SECTOR_SIZE 512
#define BLOCK_SIZE SECTOR_SIZE
#define MAX_PATH_LEN 512

enum file_types{
    FT_UNKNOWN,
    FT_REGULAR,
    FT_DIRECTORY
};

enum oflags
{
    O_RDONLY,		//只读属性
    O_WRONLY,		//只写属性
    O_RDWR,		//可读写
    O_CREAT = 4	//创建
};

struct path_search_record
{
    char searched_path[MAX_PATH_LEN]; //父路径
    struct dir* parent_dir;	       //文件直接的父路径 或者说上一级的路径
    enum file_types file_type;        //普通文件 目录 或者未知类型
};

extern struct partition* cur_part;

void filesys_init(void);
int32_t sys_open(const char* pathname,uint8_t flags);
int32_t path_depth_cnt(char* pathname);

#endif