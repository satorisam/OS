#include "fs.h"
#include "stdint.h"
#include "global.h"
#include "../device/ide.h"
#include "inode.h"
#include "dir.h"
#include "super_block.h"
#include "stdio-kernel.h"
#include "string.h"
#include "debug.h"
#include "list.h"
#include "file.h"

struct partition* cur_part;	//默认操作分区

static void partition_format(struct disk* hd,struct partition* part){
    uint32_t boot_sector_sects = 1;		//引导块一个块
    uint32_t super_block_sects = 1;		//超级块一个块
    uint32_t inode_bitmap_sects = DIV_ROUND_UP(MAX_FILES_PER_PART,BITS_PER_SECTOR);  //inode位图占的块数
    										       //inode数组所占的块数
    uint32_t inode_table_sects = DIV_ROUND_UP((sizeof(struct inode) * MAX_FILES_PER_PART),SECTOR_SIZE);
    
    //注意这里的used_sects 肯定是不准确 差了那么一点点的 因为还没有包含block_bitmap_sects 但是为了简单处理 要先得到free_sects才能推  所以到后面block_bitmap_sects 要除两次
    uint32_t used_sects = boot_sector_sects + super_block_sects + inode_bitmap_sects + inode_table_sects;
    uint32_t free_sects = part->sec_cnt - used_sects;
    
    uint32_t block_bitmap_sects = DIV_ROUND_UP(free_sects,BITS_PER_SECTOR);	//一位一块
    uint32_t block_bitmap_bit_len = free_sects - block_bitmap_sects;	//再减去block_bitmap的
    block_bitmap_sects = DIV_ROUND_UP(block_bitmap_bit_len,BITS_PER_SECTOR);
    
    struct super_block sb;	        		//利用栈来初始化超级块 我们的栈此刻在
    sb.magic         = 0x20020419;			//魔数
    sb.sec_cnt       = part->sec_cnt; 		//该分区总扇区数
    sb.inode_cnt     = MAX_FILES_PER_PART;		//该分区总inode数
    sb.part_lba_base = part->start_lba;		//该分区lba起始扇区位置
    
    // 引导块 超级块 空闲块位图 inode位图 inode数组 根目录 空闲块区域
    //挨着挨着顺序赋值即可
    sb.block_bitmap_lba   = part->start_lba + 2;
    sb.block_bitmap_sects = block_bitmap_sects;
    
    sb.inode_bitmap_lba   = sb.block_bitmap_lba + sb.block_bitmap_sects;
    sb.inode_bitmap_sects = inode_bitmap_sects;
    
    sb.inode_table_lba    = sb.inode_bitmap_lba + sb.inode_table_sects;
    sb.inode_table_sects  = inode_table_sects;
    
    sb.data_start_lba     = sb.inode_table_lba + sb.inode_table_sects;
    sb.root_inode_no	   = 0;			//根目录inode起始编号 0 
    sb.dir_entry_size     = sizeof(struct dir_entry); //目录项大小
    
    printk("%s  info:\n",part->name);
    printk("    magic:0x%x\n    part_lba_base:0x%x\n    all_sectors:0x%x\n    \
inode_cnt:0x%x\n    block_bitmap_lba:0x%x\n    block_bitmap_sectors:0x%x\n    \
inode_bitmap_lba:0x%x\n    inode_bitmap_sectors:0x%x\n    \
inode_table_lba:0x%x\n    inode_table_sectors:0x%x\n    \
data_start_lba:0x%x\n", \
    sb.magic,sb.part_lba_base,sb.sec_cnt,sb.inode_cnt,sb.block_bitmap_lba,sb.block_bitmap_sects,\
    sb.inode_bitmap_lba,sb.inode_bitmap_sects,sb.inode_table_lba,\
    sb.inode_table_sects,sb.data_start_lba);   
    
    //把元信息挨个挨个写进硬盘
    
    ide_write(hd,part->start_lba + 1,&sb,1);
    printk("    super_block_lba:0x%x\n",part->start_lba + 1);
    
    //找一个最大的数据缓冲区 我们的栈已经不足以满足我们的各种信息的储存了 之后还要把元信息给腾到硬盘中
    uint32_t buf_size = (sb.block_bitmap_sects >= sb.inode_bitmap_sects) ? sb.block_bitmap_sects : sb.inode_bitmap_sects;
    buf_size = ((buf_size >= inode_table_sects) ? buf_size : sb.inode_table_sects) * SECTOR_SIZE;
    //申请缓冲空间 给元信息腾空间 设置成uint8_t* 原因是 先弄块位图的初始化
    uint8_t* buf = (uint8_t*)sys_malloc(buf_size);
    
    /* 初始化块位图了 */
    buf[0] |= 0x1;
    uint32_t block_bitmap_last_byte = block_bitmap_bit_len / 8; //先算算占用多少字节
    uint8_t block_bitmap_last_bit  = block_bitmap_bit_len % 8; //最后还有剩余多少位
    uint32_t last_size = SECTOR_SIZE - (block_bitmap_last_byte % SECTOR_SIZE); //先除余数 算出来多少字节空的
    
    //处理字节 把可能多的一字节全部置成1 这几步处理的很细节阿
    memset(&buf[block_bitmap_last_byte],0xff,last_size);	 //全部置1 保证不会被使用
    
    //处理最后的位 有效位变成0 用~来处理 真的很妙
    uint8_t bit_idx = 0;
    while(bit_idx < block_bitmap_last_bit){  //修改过
        buf[block_bitmap_last_byte] &= ~(1 << (bit_idx++));	//有效位
    }
    
    //把位图元信息给写到硬盘中 块位图的部分就结束了 还有inode位图 inode数组等着我们
    ide_write(hd,sb.block_bitmap_lba,buf,sb.block_bitmap_sects);
    
    /*初始化inode位图了*/
    memset(buf,0,buf_size);
    buf[0] |= 0x1;	                                          //第一个inode用于存根目录
    ide_write(hd,sb.inode_bitmap_lba,buf,sb.inode_bitmap_sects); //第一个inode初始化在后面
    
    /*初始化inode数组了*/
    memset(buf,0,buf_size);
    struct inode* i = (struct inode*)buf;			//先初始化第一个inode 根目录所在的
    i->i_size = sb.dir_entry_size * 2;			//. 和 .. 
    i->i_no   = 0;
    i->i_sectors[0]  = sb.data_start_lba;			//根目录所在扇区就是最开始的第一个扇区
    
    ide_write(hd,sb.inode_table_lba,buf,sb.inode_table_sects);
    
    /*写根目录文件进入 第一个扇区了*/
    memset(buf,0,buf_size);
    struct dir_entry* p_de = (struct dir_entry*)buf;
    
    memcpy(p_de->filename,".",1);				//名称
    p_de->i_no = 0;						//根目录. inode仍然是自己
    p_de->f_type = FT_DIRECTORY;
    p_de++;							//移动到下一条目录项
    
    memcpy(p_de->filename,"..",2);
    p_de->i_no = 0;						//根目录的父目录仍然是自己 因为自己是固定好的 根基
    p_de->f_type = FT_DIRECTORY;
    
    ide_write(hd,sb.data_start_lba,buf,1);			//把根目录文件写到第一个扇区中
    
    printk("    root_dir_lba:0x%x\n",sb.data_start_lba);
    printk("%s format done\n",part->name);
    sys_free(buf);						//临时借用的 现在得还回去了 
}

//除了挂载 还需要在内存中把超级块指针 块位图 i结点位图 i结点指针给初始化赋值了 方便使用

bool mount_partition(struct list_elem* pelem,int arg){
    char* part_name = (char*)arg;
    struct partition* part = elem2entry(struct partition,part_tag,pelem);//得到分区指针 partition*
    if(!strcmp(part->name,part_name)){					   //字符串相匹配
    	cur_part = part;						   //赋值指针
    	struct disk* hd = cur_part->my_disk;
    	
    	struct super_block* sb_buf = (struct super_block*)sys_malloc(SECTOR_SIZE);
        cur_part->sb = (struct super_block*)sys_malloc(sizeof(struct super_block));
    	if(cur_part->sb == NULL)
    	    PANIC("alloc memory failed!");
    	
    	memset(sb_buf,0,SECTOR_SIZE);
    	ide_read(hd,cur_part->start_lba + 1,sb_buf,1);
    	memcpy(cur_part->sb,sb_buf,sizeof(struct super_block));
    	
    	cur_part->block_bitmap.bits = (uint8_t*)sys_malloc(sb_buf->block_bitmap_sects * SECTOR_SIZE);
    	if(cur_part->block_bitmap.bits == NULL)
    	   PANIC("alloc memory failed!");
    	cur_part->block_bitmap.btmp_bytes_len = sb_buf->block_bitmap_sects * SECTOR_SIZE;
    	ide_read(hd,sb_buf->block_bitmap_lba,cur_part->block_bitmap.bits,sb_buf->block_bitmap_sects);
    	
        cur_part->inode_bitmap.bits = (uint8_t*)sys_malloc(sb_buf->inode_bitmap_sects * SECTOR_SIZE);
        if(cur_part->inode_bitmap.bits == NULL)
    	   PANIC("alloc memory failed!");
    	cur_part->inode_bitmap.btmp_bytes_len = sb_buf->inode_bitmap_sects * SECTOR_SIZE;
    	ide_read(hd,sb_buf->inode_bitmap_lba,cur_part->inode_bitmap.bits,sb_buf->inode_bitmap_sects);
    	
    	list_init(&cur_part->open_inodes);
    	printk("mount %s done!\n",part->name);
    	return true;	//停止循环
    	
    }
    return false;	//继续循环
}

static char* path_parse(char* pathname,char* name_store){
    if(pathname[0] == '/')
        while(*(++pathname) == '/');	//直到pathname位置不是

    while(*pathname != '/' && *pathname != '\0')
    	*(name_store++) = *(pathname++);
    	
    if(pathname[0] == 0)	return NULL;
    return pathname;
}

//返回路径的层数 
int32_t path_depth_cnt(char* pathname){
    ASSERT(pathname != NULL);
    char* p = pathname;
    char name[MAX_FILE_NAME_LEN];
    
    uint32_t depth = 0;
    
    p = path_parse(p,name);
    while(name[0]){
    	++depth;
    	memset(name,0,MAX_FILE_NAME_LEN);
    	if(p)
    	    p = path_parse(p,name);
    }
    return depth;
}

static int search_file(const char* pathname,struct path_search_record* searched_record)
{
    //如果是根目录 则直接判定返回即可 下面的工作就不需要做了
    if(!strcmp(pathname,"/") || !strcmp(pathname,"/.") || !strcmp(pathname,"/.."))
    {
    	searched_record->parent_dir = &root_dir;
    	searched_record->file_type  = FT_DIRECTORY;
    	searched_record->searched_path[0] = 0;		//置空
    	return 0;	//根目录inode编号为0
    }
    
    uint32_t path_len = strlen(pathname);
    
    ASSERT(pathname[0] == '/' && path_len > 1 && path_len < MAX_PATH_LEN);		
    char* sub_path = (char*)pathname;
    struct dir* parent_dir = &root_dir;			//每个刚开始都是从根目录开始
    struct dir_entry dir_e;					//存放目录项的临时变量
    
    char name[MAX_FILE_NAME_LEN] = {0};
    
    searched_record->parent_dir = parent_dir;
    searched_record->file_type  = FT_UNKNOWN;
    uint32_t parent_inode_no = 0;				//父目录的inode号
    
    sub_path = path_parse(sub_path,name);			//解析目录
    while(name[0])
    {
    	ASSERT(strlen(searched_record->searched_path) < 512);
    	strcat(searched_record->searched_path,"/");
    	strcat(searched_record->searched_path,name);
    	
    	if(search_dir_entry(cur_part,parent_dir,name,&dir_e))
    	{
    	    memset(name,0,MAX_FILE_NAME_LEN);
    	    if(sub_path)    sub_path = path_parse(sub_path,name);   
    	    
    	    if(FT_DIRECTORY == dir_e.f_type)	//打开的是目录继续解析即可
    	    {
    	    	
    	    	parent_inode_no = parent_dir->inode->i_no;
    	    	dir_close(parent_dir);
    	    	parent_dir = dir_open(cur_part,dir_e.i_no);
    	    	searched_record->parent_dir = parent_dir;
                continue;
    	    }
    	    else if(FT_REGULAR == dir_e.f_type)
    	    {
    	    	searched_record->file_type = FT_REGULAR;
    	    	return dir_e.i_no;
    	    }
    	}
    	else 	return -1;
    }
    
    dir_close(searched_record->parent_dir);
    searched_record->parent_dir = dir_open(cur_part,parent_inode_no);
    searched_record->file_type = FT_DIRECTORY;
    return dir_e.i_no;
}

int32_t sys_open(const char* pathname,uint8_t flags)
{
    //最后一位是'/'则无法辨析 这里是打开文件
    if(pathname[strlen(pathname) - 1] == '/')
    {
    	printk("cant open a directory %s\n",pathname);
    	return -1;
    }
    ASSERT(flags <= 7);
    int32_t fd = -1;
    
    struct path_search_record searched_record;	     //记录访问记录
    memset(&searched_record,0,sizeof(struct path_search_record));
    
    uint32_t pathname_depth = path_depth_cnt((char*)pathname);
    
    int inode_no = search_file(pathname,&searched_record); //搜索文件
    bool found = (inode_no == -1) ? false : true;
    
    if(searched_record.file_type == FT_DIRECTORY)	     //如果是目录文件类型
    {
    	printk("cant open a directory with open(),use opendir() to instead\n");
    	dir_close(searched_record.parent_dir);
    	return -1;
    }
    
    uint32_t path_searched_depth = path_depth_cnt(searched_record.searched_path);
    
    if(pathname_depth != path_searched_depth)
    {
    	printk("cannot access %s:Not a directory,subpath %s isnt exist\n",pathname,searched_record.searched_path);
    	dir_close(searched_record.parent_dir);
    	return -1;	
    }
    
    if(!found && !(flags & O_CREAT))
    {
    	printk("in path %s: Not a directory, subpath %s isnt exist\n",pathname,searched_record.searched_path);
    	dir_close(searched_record.parent_dir);
    	return -1;
    }
    else if(found && (flags & O_CREAT))
    {
    	printk("%s has already exist!\n",pathname);
    	dir_close(searched_record.parent_dir);
    	return -1;
    }
    
    switch(flags & O_CREAT)
    {
    	case O_CREAT:
    	    printk("creating file\n");
    	    fd = file_create(searched_record.parent_dir,strrchr(pathname,'/') + 1,flags);
    	    dir_close(searched_record.parent_dir);
			break;
    }
    
    return fd;
}

//文件系统初始化 磁盘上搜索 如果没有则格式化分区 并创建文件系统
void filesys_init(void){
    uint8_t channel_no = 0,dev_no,part_idx = 0;
    struct super_block* sb_buf = (struct super_block*)sys_malloc(SECTOR_SIZE);
    
    if(sb_buf == NULL)	PANIC("alloc memory failed!");
    printk("searching filesysteam......\n");
    while(channel_no < channel_cnt)
    {
    	dev_no = 1;
    	while(dev_no < 2)
    	{
    	    if(dev_no == 0)	//跳过hd60M.img主盘
    	    {
    	    	++dev_no;	
    	    	continue;
    	    }
    	    struct disk* hd = &channels[channel_no].devices[dev_no];		//得到硬盘指针
    	    struct partition* part = hd->prim_parts;				//先为主区创建文件系统
    	    while(part_idx < 12)		//4个主区 + 8个逻辑分区
    	    {
    	    	if(part_idx == 4)
    	    	    part = hd->logic_parts;	
    	    	if(part->sec_cnt != 0)		//分区存在 如果没有初始化 即所有成员都为0
    	    	{
    	    	    memset(sb_buf,0,SECTOR_SIZE);
    	    	    ide_read(hd,part->start_lba +1,sb_buf,1);	//读取超级块的扇区
    	    	    
    	    	    if(sb_buf->magic != 0x20020419)			//还没有创建文件系统
    	    	    {
    	    	    	printk("formatting %s's partition %s......\n",\
    	    	    	hd->name,part->name);
    	    	    	partition_format(hd,part);
    	    	    }
    	    	    else
    	    	    	printk("%s has filesystem\n",part->name);
    	    	}
    	    	++part_idx;
    	    	++part;	//到下一个分区看
    	    }
    	    ++dev_no;		//切换盘号
    	}
    	++channel_no;		//增加ide通道号
    }
    sys_free(sb_buf);
    char default_part[8] = "sbb1";	//参数为int 4字节字符串指针传的进去
    list_traversal(&partition_list,mount_partition,(int)default_part);
    open_root_dir(cur_part);
    
    uint32_t fd_idx = 0;
    while(fd_idx < MAX_FILE_OPEN)
    	file_table[fd_idx++].fd_inode = NULL;
}
