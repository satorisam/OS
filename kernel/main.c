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

void k_thread_a(void* arg);
void k_thread_b(void* arg);
void u_prog_a(void);
void u_prog_b(void);
int prog_a_pid = 0,prog_b_pid = 0;

int main(){
    put_str("I am kernel\n");
    init_all();
    
    //process_execute(u_prog_a,"user_prog_a");
    process_execute(u_prog_b,"user_prog_b");
    thread_start("k_thread_b",31,k_thread_b,"argB ");
    intr_enable();
    
    thread_start("k_thread_a",31,k_thread_a,"argA ");
    /*
    while(1){
        console_put_str("Main ");
    }*/


    while(1);
    return 0;
}

void k_thread_a(void* arg){
    char* para = arg;
    void* addr = sys_malloc(33);
    console_put_str(" thread_a:sys_malloc(33)");
    console_put_int((int)addr);
    console_put_char('\n');
    while(1);
}

void k_thread_b(void* arg){
    char* para = arg;
    void* addr = sys_malloc(63);
    console_put_str(" thread_b:sys_malloc(63)");
    console_put_int((int)addr);
    console_put_char('\n');
    while(1);
}

void u_prog_a(void){
    char* name = "prog_a";
    printf(" I am %s, my pid:%d%c",name,getpid(),'\n');
    while(1);
}

void u_prog_b(void){
    char* name = "prog_b";
    printf(" I am %s, my pid:%d%c",name,getpid(),'\n');
    while(1);
}

/*
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
*/