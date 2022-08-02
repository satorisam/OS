#include "print.h"
#include "init.h"
#include "debug.h"
#include "interrupt.h"

int main(){
    put_str("I am kernel\n");
    init_all();
    intr_set_status(INTR_ON);
    ASSERT(1==2);


    while(1);
    return 0;
}