struct context{
    uint64 x4;
    uint64 x5;
    uint64 x6;
    uint64 x7;
    uint64 x8;
    uint64 x9;
    uint64 x10;
    uint64 x11;
    uint64 x12;
    uint64 x13;
    uint64 x14;
    uint64 x15;
    uint64 x16;
    uint64 x17;
    uint64 x18;
    uint64 x19;
    uint64 x20;
    uint64 x21;
    uint64 x22;
    uint64 x23;
    uint64 x24;
    uint64 x25;
    uint64 x26;
    uint64 x27;
    uint64 x28;
    uint64 x29;
    uint64 elr;
};

struct cpu {
    struct proc* proc;          // The process running on this cpu, or null
    struct context context;     // swtch() here to enter scheduler()
    int noff;                   // Depth of push_off()
    int intena;                 // Were interrupts enabled before push_off()?
};

// In arm, different EL has its own TTBR,
// and share a exception vector, therefore,
// no as xv6-riscv, trapframe do not include 
// kernel_satp, kernel_trap 
struct trapframe {
    uint64  x0;
    uint64  x1;
    uint64  x2;
    uint64  x3;
    uint64  x4;
    uint64  x5;
    uint64  x6;
    uint64  x7;
    uint64  x8;
    uint64  x9;
    uint64  x10;
    uint64  x11;
    uint64  x12;
    uint64  x13;
    uint64  x14;
    uint64  x15;
    uint64  x16;
    uint64  x17;
    uint64  x18;
    uint64  x19;
    uint64  x20;
    uint64  x21;
    uint64  x22;
    uint64  x23;
    uint64  x24;
    uint64  x25;
    uint64  x26;
    uint64  x27;
    uint64  x28;
    uint64  x29;
    uint64  x30;    // link register
    uint64  sp;
    uint64  elr;    // instruction cause exception
    uint64  spsr;   // pstate
};

enum procstate { UNUSED, SLEEPING, RUNNABLE, RUNNING, ZOMBIE };

struct proc {
    struct spinlock lock;

    // p->lock must be held when using these
    enum procstate state;       // Process state
    struct proc * parent;       // Parent process
    void *chan;                 // If non-zero, sleeping on chan
    int killed;                 // If non-zero, have been killed
    int xstate;                 // Exit status to be return to parent's wait
    int pid;                    // Process ID

    // these are private to the process, so p->lock need not be held.
    uint64 kstack;              // Vitual address of kernel address
    uint64 sz;                  // Size of process memeory
    pagetable_t pagetable;      // User page table
    struct trapframe* trapframe;// data page for trampoline
    struct context context;     // swtch() here to run process
    char name[16];              // Process name (debugging)
};
