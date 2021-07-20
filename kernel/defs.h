struct spinlock;
struct context;
//kalloc.c
void            kinit(void);
void            freerange(void*, void*);
void            kfree(void*);
void*           kalloc(void);

//main.c
void            kmain();

//printf.c
void            panic(char* );
void            printfinit(void);

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

//vm.c
// void        kvmmap(pagetable_t kpgtbl, uint64 va, uint64 pa, uint64 sz, int perm, int dev_mem);
int             mappages(pagetable_t, uint64, uint64, uint64 , int);
pte_t*          walk(pagetable_t, uint64, int);
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
int             copyout(pagetable_t, uint64, char*, uint64);
int             copyin(pagetable_t, char *, uint64, uint64);
int             copyinstr(pagetable_t, char*, uint64, uint64);

