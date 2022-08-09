#include "print.h"
#include "init.h"
#include "debug.h"
#include "interrupt.h"
#include "memory.h"
#include "../thread/thread.h"
#include "console.h"

void k_thread_a(void* arg);
void k_thread_b(void* arg);

int main(){
    put_str("I am kernel\n");
    init_all();
    thread_start("666",31,k_thread_a,"argA ");
    thread_start("777",8,k_thread_b,"argB ");

    intr_enable();
    while(1){
        console_put_str("Main ");
    }


    while(1);
    return 0;
}

void k_thread_a(void* arg){
    char* para = arg;
    while(1){
        console_put_str(para);
    }
}

void k_thread_b(void* arg){
    char* para = arg;
    while(1){
        console_put_str(para);
    }
}