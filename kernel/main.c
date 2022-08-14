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
    
    //process_execute(u_prog_a,"user_prog_a");
    //process_execute(u_prog_b,"user_prog_b");
    //thread_start("k_thread_b",31,k_thread_b,"argB ");
    //thread_start("k_thread_a",31,k_thread_a,"argA ");
	
	char buf[64] = {0};
   uint32_t fd = sys_open("/file1",O_CREAT);
   sys_close(fd);
   
   fd = sys_open("/file1",O_RDWR);
   printk("open /file1,fd:%d\n",fd);
   //sys_write(fd,"hello,world\n",12); //新硬盘可以先存数据用 要是之前存过删掉即可
   //sys_write(fd,"hello,world\n",12);
   sys_close(fd);
   
   fd = sys_open("/file1",O_RDONLY);
   int read_bytes = sys_read(fd,buf,18);
   printk("1_ read %d bytes:\n%s\n",read_bytes,buf);
   
   memset(buf,0,sizeof(buf));
   read_bytes = sys_read(fd,buf,7);
   printk("2_ read %d bytes:\n%s",read_bytes,buf);
   
   memset(buf,0,sizeof(buf));
   read_bytes = sys_read(fd,buf,2);
   printk("3_ read %d bytes:\n%s",read_bytes,buf);
   
   printk("________ SEEK_SET 0 ________\n");
   
   sys_lseek(fd,0,SEEK_SET);
   memset(buf,0,sizeof(buf));
   read_bytes = sys_read(fd,buf,25);
   printk("4_ read %d bytes:\n%s",read_bytes,buf);
   
   printk("________ SEEK_SET 6 ________\n");
   sys_lseek(fd,6,SEEK_SET);
   memset(buf,0,sizeof(buf));
   read_bytes = sys_read(fd,buf,5);
   printk("5_ read %d bytes:\n%s\n",read_bytes,buf);
   
   printk("________ SEEK_CUR 1________\n");
   sys_lseek(fd,1,SEEK_CUR);
   memset(buf,0,sizeof(buf));
   read_bytes = sys_read(fd,buf,5);
   printk("6_ read %d bytes:\n%s",read_bytes,buf);
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
