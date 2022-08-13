#ifndef __THREAD_SYNC_H
#define __THREAD_SYNC_H
#include "../lib/kernel/list.h"
#include "../lib/stdint.h"
#include "thread.h"

struct semaphore{
    uint8_t value;
    struct list waiters;
};

struct lock{
    struct task_struct* holder;
    struct semaphore semaphore;
    uint32_t holder_repeat_nr;
};

void lock_acquire(struct lock* plock);
void lock_release(struct lock* plock);
void lock_init(struct lock* plock);
void sema_init(struct semaphore* psema,uint8_t value);

#endif