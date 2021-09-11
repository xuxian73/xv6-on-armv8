struct sleeplock;
struct spinlock;
struct context;
struct trapframe;
struct proc;
struct file;
struct buf;
struct stat;
struct superblock;
struct pipe;

typedef void (*ISR) ();
#define NELEM(x) (sizeof(x) / sizeof((x)[0]))

//bio.c
void            binit(void);
struct buf*     bread(uint dev, uint blockno);
void            bwrite(struct buf *b);
void            brelse(struct buf *b);
void            bpin(struct buf*);
void            bunpin(struct buf*);

// console.c
void            consputc(int c);
int             consolewrite(int user_src, uint64 src, int n);
int             consoleread(int user_dst, uint64 dst, int n);
void            consoleintr(int c);
void            consoleinit(void);

// exec.c
int             exec(char *path, char **argv);

//file.c
void            fileinit(void);
struct file*    filealloc(void);
struct file*    filedup(struct file*);
void            fileclose(struct file*);
int             filestat(struct file *, uint64);
int             fileread(struct file* f, uint64, int);
int             filewrite(struct file *f, uint64 addr, int n);

//fs.c
void            fsinit(int dev);
void            iinit();
struct inode*   ialloc(uint dev, short type);
void            iupdate(struct inode*);
struct inode*   idup(struct inode *);
void            ilock(struct inode *);
void            iunlock(struct inode *);
void            iput(struct inode *);
void            iunlockput(struct inode*);
void            itrunc(struct inode *);
void            stati(struct inode*, struct stat*);
int             readi(struct inode *, int, uint64, uint, uint);
int             writei(struct inode*, int, uint64, uint, uint);
int             namecmp(const char*, const char*);
struct inode*   dirlookup(struct inode*, char*, uint *);
int             dirlink(struct inode*, char*, uint);
struct inode *  namei(char*);
struct inode*   nameiparent(char*, char*);

// gic.c
void            gic_init();
void            gic_set(int, ISR);
int             gic_handler();

//kalloc.c
void            kinit(void);
void            freerange(void*, void*);
void            kfree(void*);
void*           kalloc(void);

//log.c
void            initlog(int dev, struct superblock *);
void            begin_op(void);
void            end_op(void);
void            log_write(struct buf*);

//main.c
void            kmain();

//memide.c
void            ideinit(void);
void            ideintr(void);
void            iderw(struct buf *b, int write);

// picirq.c
// void            pic_enable(int, ISR);
// void            pic_init(void*);
// void            pic_dispatch (struct trapframe *tp);

// pipe.c
int             pipealloc(struct file **f0, struct file **f1);
void            pipeclose(struct pipe *pi, int writable);
int             pipewrite(struct pipe *pi, uint64 addr, int n);
int             piperead(struct pipe *pi, uint64 addr, int n);

//printf.c
void            panic(char* ) __attribute__((noreturn));
void            printfinit(void);
void            printf(char*, ...);

//proc.c
struct cpu*     mycpu(void);
struct proc*    myproc(void);
void            procinit(void);
pagetable_t     proc_pagetable(struct proc *p);
void            proc_freepagetable(pagetable_t pagetable, uint64 sz);
void            forkret(void);
void            userinit(void);
void            sched(void);
void            scheduler(void);
void            exit(int);
int             fork(void);
int             growproc(int);
int             kill(int);
int             wait(uint64);
void            sleep(void *chan, struct spinlock *lk);
void            wakeup(void*);
int             either_copyout(int user_dst, uint64 dst, void *src, uint64 len);
int             either_copyin(void *dst, int user_src, uint64 src, uint64 len);
void            procdump(void);
void            yield();

//sleeplock.c
void            initsleeplock(struct sleeplock *, char *);
void            acquiresleep(struct sleeplock*);
void            releasesleep(struct sleeplock*);
int             holdingsleep(struct sleeplock*);

//spinlock.c
void            initlock(struct spinlock*, char* );
void            acquire(struct spinlock*);
void            release(struct spinlock*);
void            push_off(void);
void            pop_off(void);
int             holding(struct spinlock*);
int             intr_get(void);

//start.c
void            _puts(char*);
void            _puts_int(uint num);

//string.c
int             memcmp(const void*, const void*, uint);
void*           memmove(void*, const void*, uint);
void*           memcpy(void*, const void*,uint);
void*           memset(void*, int, uint);
char*           safestrcpy(char*, const char*, int);
int             strlen(const char*);
int             strncmp(const char*, const char*, uint);
char*           strncpy(char*, const char*, int);

// swtch.S
void            swtch(struct context**, struct context*);

//syscall.c
int             argint(int, int*);
int             argstr(int, char*, int);
int             argaddr(int, uint64*);
int             fetchstr(uint64, char*, int);
int             fetchaddr(uint64, uint64*);
void            syscall(void);

//sysfile.c

//sysproc.c

//timer.c
void            timer_init();

//trap.c
extern uint     ticks;
extern struct spinlock tickslock;

// uart.c
void            uart_init(void*);
void            uartputc(int);
void            uartputc_sync(int);
int             uartgetc(void);
void            uart_enable_rx();

//virtio_disk.c
void            virtio_disk_init();
void            virtio_disk_intr();
void            virtio_disk_rw(struct buf*, int);

//vm.c
// void        kvmmap(pagetable_t kpgtbl, uint64 va, uint64 pa, uint64 sz, int perm, int dev_mem);
int             mappages(pagetable_t, uint64, uint64, uint64 , int);
pte_t*          walk(pagetable_t, uint64, int);
uint64          walkaddr(pagetable_t, uint64);
void            pre_free4kpt(void* , void*);
void            page_init(void);
void            uvmunmap(pagetable_t, uint64, uint64, int);
pagetable_t     uvmcreate();
void            uvminit(pagetable_t, uchar*, uint);
uint64          uvmalloc(pagetable_t, uint64, uint64);
uint64          uvmdealloc(pagetable_t, uint64, uint64);
void            freewalk(pagetable_t);
void            uvmfree(pagetable_t, uint64);
int             uvmcopy(pagetable_t, pagetable_t, uint64);
void            uvmclear(pagetable_t, uint64);
void            uvmswitch(struct proc*);
int             copyout(pagetable_t, uint64, char*, uint64);
int             copyin(pagetable_t, char *, uint64, uint64);
int             copyinstr(pagetable_t, char*, uint64, uint64);


