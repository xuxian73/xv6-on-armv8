#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "arm.h"
#include "spinlock.h"
#include "proc.h"
#include "syscall.h"
#include "defs.h"

// Fetch the uint64 at addr from the current process.
int
fetchaddr(uint64 addr, uint64 *ip)
{
  struct proc *p = myproc();
  if(addr >= p->sz || addr+sizeof(uint64) > p->sz)
    return -1;
  if(copyin(p->pagetable, (char *)ip, addr, sizeof(*ip)) != 0)
    return -1;
  return 0;
}

// Fetch the nul-terminated string at addr from the current process.
// Returns length of string, not including nul, or -1 for error.
int fetchstr(uint64 addr, char *buf, int max)
{
    struct proc *p = myproc();
    int err = copyinstr(p->pagetable, buf, addr, max);
    if (err < 0)
        return err;
    return strlen(buf);
}

static uint64
argraw(int n)
{
    struct proc *p = myproc();
    switch (n)
    {
    case 0:
        return p->trapframe->x0;
    case 1:
        return p->trapframe->x1;
    case 2:
        return p->trapframe->x2;
    case 3:
        return p->trapframe->x3;
    case 4:
        return p->trapframe->x4;
    case 5:
        return p->trapframe->x5;
    case 6:
        return p->trapframe->x6;
    default:
        panic("argraw");
    }
    return -1;
}

int argint(int n, int *ip)
{
    *ip = argraw(n);
    return 0;
}

int argaddr(int n, uint64 *ip)
{
    *ip = argraw(n);
    return 0;
}

int argstr(int n, char *buf, int max)
{
    uint64 addr;
    if (argaddr(n, &addr) < 0)
        return -1;
    return fetchstr(addr, buf, max);
}

extern uint64 sys_chdir(void);
extern uint64 sys_close(void);
extern uint64 sys_dup(void);
extern uint64 sys_exec(void);
extern uint64 sys_exit(void);
extern uint64 sys_fork(void);
extern uint64 sys_fstat(void);
extern uint64 sys_getpid(void);
extern uint64 sys_kill(void);
extern uint64 sys_link(void);
extern uint64 sys_mkdir(void);
extern uint64 sys_mknod(void);
extern uint64 sys_open(void);
extern uint64 sys_pipe(void);
extern uint64 sys_read(void);
extern uint64 sys_sbrk(void);
extern uint64 sys_sleep(void);
extern uint64 sys_unlink(void);
extern uint64 sys_wait(void);
extern uint64 sys_write(void);
extern uint64 sys_uptime(void);

static uint64 (*syscalls[])(void) = {
    [SYS_fork] sys_fork,
    [SYS_exit] sys_exit,
    [SYS_wait] sys_wait,
    [SYS_pipe] sys_pipe,
    [SYS_read] sys_read,
    [SYS_kill] sys_kill,
    [SYS_exec] sys_exec,
    [SYS_fstat] sys_fstat,
    [SYS_chdir] sys_chdir,
    [SYS_dup] sys_dup,
    [SYS_getpid] sys_getpid,
    [SYS_sbrk] sys_sbrk,
    [SYS_sleep] sys_sleep,
    [SYS_uptime] sys_uptime,
    [SYS_open] sys_open,
    [SYS_write] sys_write,
    [SYS_mknod] sys_mknod,
    [SYS_unlink] sys_unlink,
    [SYS_link] sys_link,
    [SYS_mkdir] sys_mkdir,
    [SYS_close] sys_close,
};

void syscall(void)
{
    int num;
    int ret;
    struct proc *p = myproc();
    num = p->trapframe->x7;

    if ((num > 0) && (num <= NELEM(syscalls)) && syscalls[num])
    {
        ret = syscalls[num]();
        p->trapframe->x0 = ret;
    }
    else
    {
        p->trapframe->x0 = -1;
    }
}