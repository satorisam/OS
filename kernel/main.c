#include "print.h"
#include "init.h"
#include "debug.h"
#include "interrupt.h"
#include "memory.h"

int main(){
    put_str("I am kernel\n");
    init_all();
    void* addr = get_kernel_pages(3);
    put_char('\n');
    put_int((uint32_t)addr);


    while(1);
    return 0;
}