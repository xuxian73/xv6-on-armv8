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
    _puts("bad_handler: interrupt type ");
    _puts_int((uint)type);
    _puts(" not implemented.\n");
    p->killed = 1;
}

void 
irq_handler(struct trapframe* tf, uint64 el, uint64 esr)
{
    struct proc* p = myproc();
    p->trapframe = tf;
    gic_handler();
}

void
dabort_handler(struct trapframe tf, uint64 el)
{
    struct proc *p;
    cli();
    p = myproc();
    _puts("dabort_handler: not implemented.\n");
    p->killed = 1;
    sti();
}

void
iabort_handler(struct trapframe tf, uint64 el)
{
    struct proc *p;
    p = myproc();
    _puts("iabort_handler: not implemented.\n");
    p->killed = 1;
    sti();
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