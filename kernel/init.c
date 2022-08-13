#include "init.h"
#include "print.h"
#include "interrupt.h"
#include "timer.h"
#include "memory.h"
#include "../thread/thread.h"
#include "console.h"
#include "keyboard.h"
#include "../userprog/tss.h"
#include "../userprog/syscall-init.h"
#include "../device/ide.h"

void init_all(){
    put_str("init_all\n");
    idt_init();
    timer_init();
    mem_init();
    thread_init();
    console_init();
    keyboard_init();
    tss_init();
    syscall_init();
    ide_init();
}