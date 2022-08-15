#ifndef __LIB_USER_SYSCALL_H
#define __LIB_USER_SYSCALL_H
#include "../lib/stdint.h"
enum SYSCALL_NR{
    SYS_GETPID,
    SYS_WRITE,
    SYS_MALLOC,
    SYS_FREE,
    SYS_FORK,
    SYS_READ,
    SYS_PUTCHAR,
    SYS_CLEAR
};

uint32_t getpid(void);
void* malloc(uint32_t size);
void free(void* ptr);
uint32_t write(int fd,const void* buf,int count);

#endif