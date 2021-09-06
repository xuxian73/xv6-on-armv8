#include "types.h"
#include "arm.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64
sys_exit(void)
{
    int n;
    if(argint(0, &n) < 0)
        return -1;
    exit(n);
    return 0; //not reached
}

uint64
sys_fork(void)
{
    return fork();
}

uint64
sys_getpid(void)
{
    return myproc()->pid;
}

uint64
sys_wait(void) 
{
    uint64 p;
    if(argaddr(0, &p) < 0)
        return -1;
    return wait(p);
}

uint64
sys_kill(void)
{
    int pid;
    if(argint(0, &pid) < 0)
        return -1;
    return kill(pid);
}

uint64 
sys_sbrk(void)
{
    int addr, n;
    if(argint(0, &n) < 0) {
        return -1;
    }
    addr = myproc()->sz;
    if(growproc(n) < 0)
        return -1;
    return addr;
}

uint64
sys_uptime(void)
{
    uint64 xticks;
    acquire(&tickslock);
    xticks = ticks;
    release(&tickslock);
    return xticks;
}

uint64
sys_sleep(void)
{
    return 0;
}