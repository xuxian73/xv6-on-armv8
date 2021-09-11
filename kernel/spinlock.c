#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "arm.h"
#include "proc.h"
#include "defs.h"

extern struct cpu* cpu;
void 
initlock(struct spinlock* lk, char* name)
{
    lk->name = name;
    lk->locked = 0;
    lk->cpu = 0;
}

//Acquire the lock
//Loops until the lock is acquired.
void
acquire(struct spinlock* lk)
{
    push_off();
    if(holding(lk))
        panic("acquire");
    while(__sync_lock_test_and_set(&lk->locked, 1) != 0)
        ;

    __sync_synchronize();

    lk->cpu = mycpu();
}

//Release the lock.
void
release(struct spinlock* lk)
{
    if(!holding(lk))
        panic("release");
    lk->cpu = 0;

    __sync_synchronize();

    __sync_lock_release(&lk->locked);

    pop_off();
}

void 
push_off(void)
{
    int old = intr_get();

    cli();
    if (mycpu()->noff == 0)
        mycpu()->intena = old;
    mycpu()->noff += 1;
}

void 
pop_off(void)
{
    struct cpu *c = mycpu();
    if(intr_get())
        panic("pop_off - interruptible");
    if(c->noff < 1)
        panic("pop_off");
    c->noff -= 1;
    if((c->noff == 0) && c->intena)
        sti();
}

// Check whether this cpu is holding the lock.
// Interrupts must be off.
int
holding(struct spinlock *lk)
{
  int r;
  r = (lk->locked && lk->cpu == mycpu());
  return r;
}

// interrupt enabled?
int
intr_get(void)
{
    uint64 x;
    asm("mrs %0, DAIF": "=r"(x) : :);
    return !(x & DAIF_I);
}
