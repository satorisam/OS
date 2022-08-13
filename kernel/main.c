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
#include "../userprog/tss.h"

void k_thread_a(void* arg);
void k_thread_b(void* arg);
void u_prog_a(void);
void u_prog_b(void);
int test_var_a = 0, test_var_b = 0;

int main(){
    put_str("I am kernel\n");
    init_all();
    
    thread_start("666",31,k_thread_a,"A_");
    thread_start("777",31,k_thread_b,"B_");
    process_execute(u_prog_a,"user_prog_a");
    process_execute(u_prog_b,"user_prog_b");
    
    intr_enable();

    /*
    while(1){
        console_put_str("Main ");
    }*/


    while(1);
    return 0;
}

void k_thread_a(void* arg){
    char* para = arg;
    while(1){
        console_put_str("v_a:0x");
        console_put_int(test_var_a);
        console_put_char(' ');
    }
}

void k_thread_b(void* arg){
    char* para = arg;
    while(1){
        console_put_str("v_b:0x");
        console_put_int(test_var_b);
        console_put_char(' ');
    }
}

void u_prog_a(void){
    while(1){
        test_var_a++;
    }
}

void u_prog_b(void){
    while(1){
        test_var_b++;
    }
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