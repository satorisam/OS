#include "print.h"
#include "init.h"
#include "debug.h"
#include "interrupt.h"
#include "memory.h"
#include "../thread/thread.h"
#include "console.h"
#include "ioqueue.h"
#include "keyboard.h"
#include "../userprog/process.h"
#include "syscall-init.h"
#include "syscall.h"
#include "../lib/stdio.h"
#include "../fs/fs.h"

void k_thread_a(void* arg);
void k_thread_b(void* arg);
void u_prog_a(void);
void u_prog_b(void);
int prog_a_pid = 0,prog_b_pid = 0;

int main(){
    put_str("I am kernel\n");
    init_all();
    intr_enable();
    printk("/dir1/subdir1 create %s!\n",(sys_mkdir("/dir1/subdir1") == 0) ? "done" : "fail");
   printk("/dir1 create %s!\n",(sys_mkdir("/dir1") == 0) ? "done" : "fail");
   printk("/dir1/subdir1 create %s!\n",(sys_mkdir("/dir1/subdir1") == 0) ? "done" : "fail");
   int fd = sys_open("/dir1/subdir1/file2",O_CREAT | O_RDWR);
   if(fd != -1)
   {
        printk("/dir1/subdir1/file2 create done!\n");
        sys_write(fd,"Catch me if u can!\n",19);
        sys_lseek(fd,0,SEEK_SET);
        char buf[32] = {0};
        sys_read(fd,buf,19);
        printf("/dir1/subdir1/file2 says:\n%s",buf);
        sys_close(fd);
   }
    while(1);
}

void u_prog_a(void){
    void* addr1 = malloc(256);
	void* addr2 = malloc(255);
	void* addr3 = malloc(254);
	printf(" prog_a malloc addr:0x%x, 0x%x, 0x%x\n", (int)addr1,(int)addr2,(int)addr3);
	int cpu_delay = 100000;
	while(cpu_delay-->0);
	free(addr1);
	free(addr2);
	free(addr3);
    while(1);
}

void u_prog_b(void){
    void* addr1 = malloc(256);
	void* addr2 = malloc(255);
	void* addr3 = malloc(254);
	printf(" prog_a malloc addr:0x%x, 0x%x, 0x%x\n", (int)addr1,(int)addr2,(int)addr3);
	int cpu_delay = 100000;
	while(cpu_delay-->0);
	free(addr1);
	free(addr2);
	free(addr3);
    while(1);
}


void k_thread_a(void* arg){
    while(1){
        if(!ioq_empty(&kbd_buf)){
            console_put_str((char*)arg);
            char byte = ioq_getchar(&kbd_buf);
            console_put_char(byte);
        }
    }
}

void k_thread_b(void* arg){
    while(1){
        if(!ioq_empty(&kbd_buf)){
            console_put_str((char*)arg);
            char byte = ioq_getchar(&kbd_buf);
            console_put_char(byte);
        }
    }
}
