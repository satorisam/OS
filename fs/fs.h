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

enum oflags{
    O_RDONLY,		//只读属性
    O_WRONLY,		//只写属性
    O_RDWR,		//可读写
    O_CREAT = 4	//创建
};

enum whence{
    SEEK_SET = 1,
    SEEK_CUR,
    SEEK_END
};

struct path_search_record{
    char searched_path[MAX_PATH_LEN]; //父路径
    struct dir* parent_dir;	       //文件直接的父路径 或者说上一级的路径
    enum file_types file_type;        //普通文件 目录 或者未知类型
};

struct stat{
    uint32_t st_ino;		       //inode编号
    uint32_t st_size;		       //尺寸
    enum file_types st_filetype;      //文件类型
};



extern struct partition* cur_part;

void filesys_init(void);
int32_t sys_open(const char* pathname,uint8_t flags);
int32_t path_depth_cnt(char* pathname);
int32_t sys_close(int32_t fd);
int32_t sys_write(int32_t fd,const void* buf,uint32_t count);
int32_t sys_read(int32_t fd,void* buf,uint32_t count);
int32_t sys_lseek(int32_t fd,int32_t offset,uint8_t whence);
int32_t sys_unlink(const char* pathname);
int32_t sys_mkdir(const char* pathname);
struct dir* sys_opendir(const char* name);
struct dir_entry* sys_readdir(struct dir* dir);
void sys_rewinddir(struct dir* dir);
int get_child_dir_name(uint32_t p_inode_nr,uint32_t c_inode_nr,char* path,void* io_buf);
int32_t sys_chdir(const char* path);
char* sys_getcwd(char* buf,uint32_t size);
int32_t sys_read(int32_t fd,void* buf,uint32_t count);
int32_t sys_stat(const char* path,struct stat* buf);
int32_t sys_rmdir(const char* pathname);
int32_t sys_closedir(struct dir* dir);
char* path_parse(char* pathname,char* name_store);

#endif