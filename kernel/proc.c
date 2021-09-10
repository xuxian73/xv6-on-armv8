#include "types.h"
#include "param.h"
#include "defs.h"
#include "spinlock.h"
#include "proc.h"
#include "mmu.h"
#include "memlayout.h"
#include "arm.h"

struct cpu cpus[NCPU];
struct cpu *cpu;

struct proc procs[NPROC];
struct proc *initproc;
int nextpid = 1;
struct spinlock pid_lock;

static void freeproc(struct proc *p);
extern void trapret(void);
static void wakeup1(struct proc *chan);

struct cpu *
mycpu(void)
{
    int id = cpuid();
    struct cpu *c = &cpu[id];
    return c;
}

struct proc *
myproc(void)
{
    push_off();
    struct cpu *c = mycpu();
    struct proc *p = c->proc;
    pop_off();
    return p;
}

int allocpid()
{
    int pid;
    acquire(&pid_lock);
    pid = nextpid;
    nextpid += 1;
    release(&pid_lock);
    return pid;
}
void procinit(void)
{
    struct proc *p;
    initlock(&pid_lock, "nextpid");
    for (p = procs; p < &procs[NPROC]; ++p)
    {
        initlock(&p->lock, "proc");
        p->kstack = (uint64)kalloc();
        // kstack is va that ias already mapped by kpgtbl
    }
}

// Look in the process table for an UNUSED proc.
// If found, initialize state required to run in the kernel,
// and return with p->lock held.
// If there are no free procs, or a memory allocation fails, return 0.

static struct proc *
allocproc(void)
{
    struct proc *p;
    char *sp;
    for (p = procs; p < &procs[NPROC]; ++p)
    {
        acquire(&p->lock);
        if (p->state == UNUSED)
        {
            goto found;
        }
        else
        {
            release(&p->lock);
        }
    }
    return 0;

found:
    p->pid = allocpid();

    sp = (char *)p->kstack + KSTACKSIZE;

    sp -= sizeof(*p->trapframe);
    p->trapframe = (struct trapframe *)sp;
    memset(p->trapframe, 0, sizeof(*p->trapframe));

    sp -= 8;
    *(uint64 *)sp = (uint64)trapret;

    sp -= 8;
    *(uint64 *)sp = (uint64)p->kstack + PGSIZE;

    sp -= sizeof(*p->context);
    p->context = (struct context *)sp;

    p->pagetable = proc_pagetable(p);

    if (p->pagetable == 0)
    {
        freeproc(p);
        release(&p->lock);
        return 0;
    }

    memset(p->context, 0, sizeof(*(p->context)));

    p->context->lr = (uint64)forkret + 8;

    return p;
}

pagetable_t
proc_pagetable(struct proc *p)
{
    pagetable_t pagetable;
    pagetable = uvmcreate();
    if (pagetable == 0)
        return 0;
    return pagetable;
}

// free a proc structure and the data hanging from it,
// including user pages.
// p->lock must be held.
static void
freeproc(struct proc *p)
{
    p->trapframe = 0;
    if (p->pagetable)
    {
        uvmfree(p->pagetable, p->sz);
    }
    p->pagetable = 0;
    p->sz = 0;
    p->pid = 0;
    p->parent = 0;
    p->name[0] = 0;
    p->chan = 0;
    p->killed = 0;
    p->xstate = 0;
    p->state = UNUSED;
}

void proc_freepagetable(pagetable_t pagetable, uint64 sz)
{
    uvmfree(pagetable, sz);
}

uchar initcode[] = {
    0x80, 0x01, 0x00, 0x58, 0xa1, 0x01, 0x00, 0x58,
    0xe7, 0x00, 0x80, 0xd2, 0x01, 0x00, 0x00, 0xd4,
    0x47, 0x00, 0x80, 0xd2, 0x01, 0x00, 0x00, 0xd4,
    0xfe, 0xff, 0xff, 0x17, 0x2f, 0x69, 0x6e, 0x69,
    0x74, 0x00, 0x00, 0x00, 0x1c, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x1c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x24, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

void userinit(void)
{
    struct proc *p;
    p = allocproc();
    initproc = p;
    uvminit(p->pagetable, initcode, sizeof(initcode));
    p->sz = PGSIZE;
    p->trapframe->elr = 0;
    p->trapframe->spsr = 0x00;
    p->trapframe->x30 = 0;
    p->trapframe->sp = PGSIZE;
    safestrcpy(p->name, "initcode", sizeof(p->name));
    p->cwd = namei("/");
    p->state = RUNNABLE;
    release(&p->lock);
}

int growproc(int n)
{
    uint sz;
    struct proc *p = myproc();
    sz = p->sz;
    if (n > 0)
    {
        if ((sz = uvmalloc(p->pagetable, sz, sz + n)) == 0)
        {
            return -1;
        }
    }
    else if (n < 0)
    {
        sz = uvmdealloc(p->pagetable, sz, sz + n);
    }
    p->sz = sz;
    uvmswitch(p);
    return 0;
}

int fork(void)
{
    int pid, i;
    struct proc *np;
    struct proc *p = myproc();

    if ((np = allocproc()) == 0)
    {
        return -1;
    }

    if (uvmcopy(p->pagetable, np->pagetable, p->sz) < 0)
    {
        freeproc(np);
        release(&np->lock);
        return -1;
    }

    np->sz = p->sz;
    np->parent = p;

    *(np->trapframe) = *(p->trapframe);

    // Set x0 = 0 so the fork return 0 in the child
    np->trapframe->x0 = 0;

    // increment reference counts on open file descriptors.
    for(i = 0; i < NOFILE; i++)
        if(p->ofile[i])
            np->ofile[i] = filedup(p->ofile[i]);
    np->cwd = idup(p->cwd);

    pid = np->pid;
    np->state = RUNNABLE;
    safestrcpy(np->name, p->name, sizeof(p->name));
    release(&np->lock);
    return pid;
}

void reparent(struct proc *p)
{
    struct proc *pp;
    for (pp = procs; pp < &procs[NPROC]; ++pp)
    {
        // this code uses pp->parent without holding pp->lock.
        // acquiring the lock first could cause a deadlock
        // if pp or a child of pp were also in exit()
        // and about to try to lock p.
        if (pp->parent == p)
        {
            acquire(&pp->lock);
            pp->parent = initproc;
            release(&pp->lock);
        }
    }
}

void exit(int status)
{
    struct proc *p = myproc();
    int fd;
    struct file* f;
    if (p == initproc)
        panic("init exiting");

    // close all open files 
    for (fd = 0; fd < NOFILE; ++fd) {
        if(p->ofile[fd]){
            f = p->ofile[fd];
            fileclose(f);
            p->ofile[fd] = 0;
        }
    }

    begin_op();
    iput(p->cwd);
    end_op();
    p->cwd = 0;

    // we might re-parent a child to init. we can't be precise about
    // waking up init, since we can't acquire its lock once we've
    // acquired any other proc lock. so wake up init whether that's
    // necessary or not. init may miss this wakeup, but that seems
    // harmless.
    acquire(&initproc->lock);
    wakeup1(initproc);
    release(&initproc->lock);

    // grab a copy of p->parent, to ensure that we unlock the same
    // parent we locked. in case our parent gives us away to init while
    // we're waiting for the parent lock. we may then race with an
    // exiting parent, but the result will be a harmless spurious wakeup
    // to a dead or wrong process; proc structs are never re-allocated
    // as anything else.
    acquire(&p->lock);
    struct proc *original_parent = p->parent;
    release(&p->lock);

    // we need the parent's lock in order to wake it up from wait().
    // the parent-then-child rule says we have to lock it first.
    acquire(&original_parent->lock);
    acquire(&p->lock);
    reparent(p);

    wakeup1(original_parent);
    p->xstate = status;
    p->state = ZOMBIE;
    release(&original_parent->lock);

    sched();
    panic("zombie exit");
}

// Wait for a child process to exit and return its pid.
// Return -1 if this process has no children.
int wait(uint64 addr)
{
    struct proc *np;
    int havekids, pid;
    struct proc *p = myproc();

    // hold p->lock for the whole time to avoid lost
    // wakeups from a child's exit().
    acquire(&p->lock);

    for (;;)
    {
        havekids = 0;
        for (np = procs; np < &procs[NPROC]; ++np)
        {
            // this code uses np->parent without holding np->lock.
            // acquiring the lock first would cause a deadlock,
            // since np might be an ancestor, and we already hold p->lock.
            if (np->parent == p)
            {
                // np->parent can't change between the check and the acquire()
                // because only the parent changes it, and we're the parent.
                acquire(&np->lock);
                havekids = 1;
                if (np->state == ZOMBIE)
                {
                    pid = np->pid;
                    if (addr != 0 && copyout(p->pagetable, addr, (char *)&np->xstate, sizeof(np->xstate)) < 0)
                    {
                        release(&np->lock);
                        release(&p->lock);
                        return -1;
                    }
                    freeproc(np);
                    release(&np->lock);
                    release(&p->lock);
                    return pid;
                }
                release(&np->lock);
            }
        }
        if (!havekids || p->killed)
        {
            release(&p->lock);
            return -1;
        }

        sleep(p, &p->lock);
    }
}

void sleep(void *chan, struct spinlock *lk)
{
    struct proc *p = myproc();

    // Must acquire p->lock in order to
    // change p->state and then call sched.
    // Once we hold p->lock, we can be
    // guaranteed that we won't miss any wakeup
    // (wakeup locks p->lock),
    // so it's okay to release lk.
    if (lk != &p->lock)
    {
        acquire(&p->lock);
        release(lk);
    }

    p->chan = chan;
    p->state = SLEEPING;

    sched();

    p->chan = 0;

    if (lk != &p->lock)
    {
        release(&p->lock);
        acquire(lk);
    }
}

void wakeup(void *chan)
{
    struct proc *p;
    for (p = procs; p < &procs[NPROC]; ++p)
    {
        acquire(&p->lock);
        if (p->state == SLEEPING && p->chan == chan)
        {
            p->state = RUNNABLE;
        }
        release(&p->lock);
    }
}

// Wake up p if it is sleeping in wait(); used by exit().
// Caller must hold p->lock.
static void
wakeup1(struct proc *p)
{
    if (!holding(&p->lock))
        panic("wakeup1");
    if (p->chan == p && p->state == SLEEPING)
    {
        p->state = RUNNABLE;
    }
}

// A fork child's very first scheduling by scheduler()
// will swtch to forkret.
void forkret(void)
{
    static int first = 1;

    release(&myproc()->lock);
    if (first)
    {
        // File system initialization must be run in the context of a
        // regular process (e.g., because it calls sleep), and thus cannot
        // be run from main().
        first = 0;
        fsinit(ROOTDEV);
    }
}

void scheduler(void)
{
    struct proc *p;
    struct cpu *c = mycpu();

    c->proc = 0;
    for (;;)
    {
        sti();

        for (p = procs; p < &procs[NPROC]; ++p)
        {
            acquire(&p->lock);
            if (p->state == RUNNABLE)
            {
                p->state = RUNNING;
                c->proc = p;
                uvmswitch(p);
                swtch(&c->context, p->context);
                c->proc = 0;
            }
            release(&p->lock);
        }
    }
}

void sched(void)
{
    int intena;
    struct proc *p = myproc();
    if (!holding(&p->lock))
        panic("sched p->lock");
    if (mycpu()->noff != 1)
        panic("sched locks");
    if (p->state == RUNNING)
        panic("sched running");
    if (intr_get())
        panic("sched interruptible");
    intena = mycpu()->intena;
    swtch(&p->context, mycpu()->context);
    mycpu()->intena = intena;
}

void yield(void)
{
    struct proc *p = myproc();
    acquire(&p->lock);
    p->state = RUNNABLE;
    sched();
    release(&p->lock);
}

int
kill(int pid)
{
    struct proc *p;
    for(p = procs; p < &procs[NPROC]; ++p) {
        acquire(&p->lock);
        if(p->pid == pid) {
            p->killed = 1;
            if (p->state == SLEEPING) {
                p->state = RUNNABLE;
            }
            release(&p->lock);
            return 0;
        }
        release(&p->lock);
    }
    return -1;
}

// Copy to either a user address, or kernel address
// depending on usr_dst
// Return 0 on success, -1 on error
int 
either_copyout(int user_dst, uint64 dst, void *src, uint64 len)
{
    struct proc *p = myproc();
    if(user_dst) {
        return copyout(p->pagetable, dst, src, len);
    } else {
        memmove((char*)dst, src, len);
        return 0;
    }
}

// Copy from either a user address, or kernel address
// depending on usr_src.
// Return 0 on success, -1 on error.
int
either_copyin(void *dst, int user_src, uint64 src, uint64 len)
{
    struct proc *p = myproc();
    if(user_src) {
        return copyin(p->pagetable, dst, src, len);
    } else{ 
        memmove(dst, (char*)src, len);
        return 0;
    }
}

void
procdump(void)
{
  static char *states[] = {
  [UNUSED]    "unused",
  [SLEEPING]  "sleep ",
  [RUNNABLE]  "runble",
  [RUNNING]   "run   ",
  [ZOMBIE]    "zombie"
  };
  struct proc *p;
  char *state;

  printf("\n");
  for(p = procs; p < &procs[NPROC]; p++){
    if(p->state == UNUSED)
      continue;
    if(p->state >= 0 && p->state < NELEM(states) && states[p->state])
      state = states[p->state];
    else
      state = "???";
    printf("%d %s %s", p->pid, state, p->name);
    printf("\n");
  }
}