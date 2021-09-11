#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "proc.h"
#include "arm.h"
#include "defs.h"

struct spinlock tickslock;
uint ticks;

void 
bad_handler(uint64 type)
{
    struct proc* p;
    p = myproc();
    printf("bad_handler: interrupt type ");
    printf("%d", (uint)type);
    printf(" not implemented.\n");
    p->killed = 1;
}

void 
irq_handler(struct trapframe* tf, uint64 el, uint64 esr)
{
    struct proc* p = myproc();
    if(p)
        p->trapframe = tf;
    int whichdev = gic_handler();
    if (p && p->killed)
        exit(-1);
    if (whichdev == PIC_TIMER && p && p->state == RUNNING)
        yield();
}

void
dabort_handler(struct trapframe tf, uint64 el)
{
    struct proc *p;
    p = myproc();
    printf("dabort_handler: not implemented.\n");
    p->killed = 1;
}

void
iabort_handler(struct trapframe tf, uint64 el)
{
    struct proc *p;
    p = myproc();
    printf("iabort_handler: not implemented.\n");
    p->killed = 1;
}

//software interrupt
//svc: generate a system call
void
swi_handler(struct trapframe* tf, uint64 el, uint64 esr)
{
    struct proc* p = myproc();
    p->trapframe = tf;
    syscall();
}