#include "print.h"
#include "init.h"
#include "debug.h"
#include "interrupt.h"
#include "memory.h"
#include "../thread/thread.h"

void k_thread_a(void* arg);

int main(){
    put_str("I am kernel\n");
    init_all();
    thread_start("666",31,k_thread_a,"argA");


    while(1);
    return 0;
}

void k_thread_a(void* arg){
    char* para = arg;
    while(1){
        put_str(para);
    }
}