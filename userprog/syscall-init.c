#include "syscall-init.h"
#include "print.h"
#include "../thread/thread.h"
#include "syscall.h"
#include "../device/console.h"
#include "../lib/string.h"

#define syscall_nr 32
typedef void* syscall;
syscall syscall_table[syscall_nr];

uint32_t sys_getpid(void){
    return running_thread()->pid;
}

uint32_t sys_write(char* str){
    console_put_str(str);
    return strlen(str);
}

void syscall_init(void){
    put_str("syscall_init start\n");
    syscall_table[SYS_GETPID] = sys_getpid;
    syscall_table[SYS_WRITE] = sys_write;
    put_str("syscall_init done\n");
}
