#include "types.h"
#include "spinlock.h"
#include "proc.h"
#include "arm.h"
#include "defs.h"
void 
bad_handler(uint64 type)
{
    cli();
    _puts("bad_handler: interrupt type ");
    _puts_int((uint)type);
    _puts(" not implemented.\n");
}

void 
irq_handler(struct trapframe tf, uint64 el, uint64 esr)
{
    cli();
    _puts("irq_handler: not implemented.\n");
}

void
dabort_handler(struct trapframe tf, uint64 el)
{
    cli();
    _puts("dabort_handler: not implemented.\n");
}

void
iabort_handler(struct trapframe tf, uint64 el)
{
    cli();
    _puts("iabort_handler: not implemented.\n");
}

//software interrupt
//svc: generate a system call
void
swi_handler(struct trapframe tf, uint64 el, uint64 esr)
{
    cli();
    _puts("swi_handler: not implemented.\n");
}