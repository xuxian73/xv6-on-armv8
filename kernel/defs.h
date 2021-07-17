struct spinlock;

//kalloc.c
void            freerange(void*, void*);
void            kfree(void*);
//main.c
void            kmain();

//spinlock.c
void            initlock(struct spinlock*, char* );
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
// int         mappages(pagetable_t pgtbl, uint64 va, uint64 pa, uint64 sz, int perm, int dev_mem);

