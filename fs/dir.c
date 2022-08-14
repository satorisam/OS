#include "dir.h"
#include "ide.h"
#include "fs.h"
#include "inode.h"
#include "memory.h"
#include "string.h"
#include "stdint.h"
#include "stdio-kernel.h"
#include "debug.h"
#include "file.h"
#include "super_block.h"

struct dir root_dir;

//打开根目录
void open_root_dir(struct partition *part)
{
    root_dir.inode = inode_open(part, part->sb->root_inode_no);
    root_dir.dir_pos = 0;
}

//打开指定inode结点中的目录 返回目录指针
struct dir *dir_open(struct partition *part, uint32_t inode_no)
{
    struct dir *pdir = (struct dir *)sys_malloc(sizeof(struct dir));
    pdir->inode = inode_open(part, inode_no);
    pdir->dir_pos = 0;
    return pdir;
}

//在part分区找名字为name的文件或者目录
//找到后返回true 并且把目录项存放到dir_e中 以后可能解析目录的时候通过递归来实现
bool search_dir_entry(struct partition *part, struct dir *pdir, const char *name, struct dir_entry *dir_e)
{
    uint32_t block_cnt = 140; // inode中12个直接指 + 1个128间接 = 140 128 = 512/4
    uint32_t *all_blocks = (uint32_t *)sys_malloc(48 + 512);
    if (all_blocks == NULL)
    {
        printk("search_dir_entry: sys_malloc for all_blocks failed\n");
        return false;
    }

    uint32_t block_idx = 0;
    while (block_idx < 12)
    {
        all_blocks[block_idx] = pdir->inode->i_sectors[block_idx];
        ++block_idx;
    }

    block_idx = 0;

    //如果存在一级间接块表
    if (pdir->inode->i_sectors[12] != 0)
        ide_read(part->my_disk, pdir->inode->i_sectors[12], all_blocks + 12, 1);

    //每次只分配一个扇区大小 之后我们把目录项不会再出现放到两个扇区之间的情况
    uint8_t *buf = (uint8_t *)sys_malloc(SECTOR_SIZE);
    struct dir_entry *p_de = (struct dir_entry *)buf;

    uint32_t dir_entry_size = part->sb->dir_entry_size;
    uint32_t dir_entry_cnt = SECTOR_SIZE / dir_entry_size;

    while (block_idx < block_cnt)
    {
        //说明此处没有目录文件 继续遍历
        if (all_blocks[block_idx] == 0)
        {
            ++block_idx;
            continue;
        }
        //把其内容读出来
        ide_read(part->my_disk, all_blocks[block_idx], buf, 1);

        uint32_t dir_entry_idx = 0;
        while (dir_entry_idx < dir_entry_cnt)
        {
            if (!strcmp(p_de->filename, name))
            {
                memcpy(dir_e, p_de, dir_entry_size); //把目录项内容复制到dir_e指针区
                sys_free(buf);
                sys_free(all_blocks);
                return true;
            }
            ++p_de;
            ++dir_entry_idx;
        }
        ++block_idx;
        p_de = (struct dir_entry *)buf; //赋值还原回去
        memset(buf, 0, SECTOR_SIZE);    //全部设置为0 初始化buf
    }
    sys_free(buf);
    sys_free(all_blocks);
    return false;
}

//关闭目录
void dir_close(struct dir *dir)
{
    if (dir == &root_dir)
        return;
    inode_close(dir->inode);
    sys_free(dir);
}

//在内存中初始化目录项
void create_dir_entry(char *filename, uint32_t inode_no, uint8_t file_type, struct dir_entry *p_de)
{
    ASSERT(strlen(filename) <= MAX_FILE_NAME_LEN);
    memcpy(p_de->filename, filename, strlen(filename));
    p_de->i_no = inode_no;
    p_de->f_type = file_type;
}

//将目录项p_de 写入父目录parent_dir io_buf由主调函数提供
bool sync_dir_entry(struct dir *parent_dir, struct dir_entry *p_de, void *io_buf)
{
    struct inode *dir_inode = parent_dir->inode;
    uint32_t dir_size = dir_inode->i_size;
    uint32_t dir_entry_size = cur_part->sb->dir_entry_size;

    ASSERT(dir_size % dir_entry_size == 0);

    uint32_t dir_entrys_per_sec = (512 / dir_entry_size); //得到每扇区最多多少个目录项
    int32_t block_lba = -1;
    uint8_t block_idx = 0;
    uint32_t all_blocks[140] = {0}; //局部变量 所有块

    //先看直接块 后面再看简介块
    while (block_idx < 12)
    {
        all_blocks[block_idx] = dir_inode->i_sectors[block_idx]; //直接块读取
        block_idx++;
    }

    struct dir_entry *dir_e = (struct dir_entry *)io_buf; //遍历目录项的临时指针
    int32_t block_bitmap_idx = -1;                        //位图idx

    block_idx = 0;
    while (block_idx < 140)
    {
        block_bitmap_idx = -1;
        if (all_blocks[block_idx] == 0) //此块还没有分配
        {
            block_lba = block_bitmap_alloc(cur_part);
            if (block_lba == -1)
            {
                printk("alloc block bitmap for sync_dir_entry failed\n");
                return false;
            }

            block_bitmap_idx = block_lba - cur_part->sb->data_start_lba; //由分配扇区号 - 起始扇区号 = 位图的相对偏移
            ASSERT(block_bitmap_idx != -1);
            bitmap_sync(cur_part, block_bitmap_idx, BLOCK_BITMAP);       //同步到硬盘

            block_bitmap_idx = -1;
            if (block_idx < 12) //直接块
                dir_inode->i_sectors[block_idx] = all_blocks[block_idx] = block_lba;
            else if (block_idx == 12) //一级间接表 间接表
            {
                dir_inode->i_sectors[block_idx] = block_lba;
                block_lba = -1;
                block_lba = block_bitmap_alloc(cur_part); //再给到一个块 给简介块第0个赋值
                if (block_lba == -1)                      //分配失败则需要回退之前的分配
                {
                    block_bitmap_idx = dir_inode->i_sectors[12] - cur_part->sb->data_start_lba;
                    bitmap_set(&cur_part->block_bitmap, block_bitmap_idx, 0);
                    dir_inode->i_sectors[12] = 0;
                    printk("alloc block bitmap for sync_dir_entry failed\n");
                    return false;
                }
                block_bitmap_idx = block_lba - cur_part->sb->data_start_lba;
                ASSERT(block_bitmap_idx != -1);
                bitmap_sync(cur_part, block_bitmap_idx, BLOCK_BITMAP);
                all_blocks[12] = block_lba;
                ide_write(cur_part->my_disk, dir_inode->i_sectors[12], all_blocks + 12, 1); //一级间接块复制到硬盘中
            }
            else //已经有一级间接表了
            {
                all_blocks[block_idx] = block_lba;
                ide_write(cur_part->my_disk, dir_inode->i_sectors[12], all_blocks + 12, 1); //一级间接表复制到硬盘中
            }

            memset(io_buf, 0, 512);
            memcpy(io_buf, p_de, dir_entry_size);
            ide_write(cur_part->my_disk, all_blocks[block_idx], io_buf, 1); //把目录项放到新分配的区块
            dir_inode->i_size += dir_entry_size;
            return true;
        }

        //此块已经被分配 寻找空余空间
        ide_read(cur_part->my_disk, all_blocks[block_idx], io_buf, 1);
        uint8_t dir_entry_idx = 0;
        while (dir_entry_idx < dir_entrys_per_sec) //由于我们存放目录项不再出现存放于两个扇区间的情况 即<dir_entrys_per_sec即可
        {
            if ((dir_e + dir_entry_idx)->f_type == FT_UNKNOWN) //这个位置还没有存放目录项
            {
                memcpy(dir_e + dir_entry_idx, p_de, dir_entry_size);
                ide_write(cur_part->my_disk, all_blocks[block_idx], io_buf, 1); //写回去
                dir_inode->i_size += dir_entry_size;                            //文件大小增加
                return true;
            }
            ++dir_entry_idx;
        }
        ++block_idx; //遍历完140个块
    }
    printk("directory is full!\n");
    return false;
}